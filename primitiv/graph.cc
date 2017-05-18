#include <config.h>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <utility>
#include <primitiv/device.h>
#include <primitiv/graph.h>

using std::move;
using std::cout;
using std::endl;
using std::vector;

namespace primitiv {

Graph::~Graph() {
  for (NodeInfo *n : nodes_) {
    delete n->func;
    delete n;
  }
}

#define CHECK_NODE(node) { \
  if ((node).g_ != this) { \
    std::stringstream ss; \
    ss << "Graph mismatched. node.g_: " << (node).g_ << " != this: " << this; \
    throw std::runtime_error(ss.str()); \
  } \
  if ((node).id_ >= nodes_.size()) { \
    std::stringstream ss; \
    ss << "Invalid node ID. " \
       << "This may be a bug and the program will abort. " \
       << "node.id_: " << (node).id_ \
       << " >= nodes_.size(): " << nodes_.size(); \
    std::abort(); \
  } \
}

Node Graph::add_function(
    Function *func,
    const std::initializer_list<const Node> &args) {
  // Gathers information of args.
  vector<unsigned> arg_ids;
  vector<const Shape *> arg_shapes;
  for (const Node &arg : args) {
    CHECK_NODE(arg);
    arg_ids.emplace_back(arg.id_);
    arg_shapes.emplace_back(&nodes_[arg.id_]->shape);
  }

  // Calculates the shape of the resulting value.
  // This may throw an exception when trying an invalid operation.
  Shape ret_shape = func->forward_shape(arg_shapes);

  // Updates the graph.
  const unsigned ret_id = nodes_.size();
  for (const unsigned arg_id : arg_ids) {
    nodes_[arg_id]->sinks.emplace_back(ret_id);
  }
  nodes_.emplace_back(new NodeInfo {
      move(ret_shape), func, Tensor(), Tensor(), move(arg_ids), {}});

  return Node(this, ret_id);
}

const Tensor &Graph::forward(const Node &node) {
  CHECK_NODE(node);

  std::function<void(const unsigned)> forward_recursive = [&](
      const unsigned id) {
    NodeInfo &n = *nodes_[id];
    if (n.value.valid()) {
      // Values of the node are already calculated.
      return;
    }

    // First time of accessing this node. Calculates actual values.
    vector<const Tensor *> args;
    for (const unsigned arg_id : n.args) {
      forward_recursive(arg_id);
      args.emplace_back(&nodes_[arg_id]->value);
    }
    n.value = n.func->forward(args);
  };

  forward_recursive(node.id_);
  return nodes_[node.id_]->value;
}

void Graph::backward(const Node &node) {
  CHECK_NODE(node);

  NodeInfo &last_node = *nodes_[node.id_];
  if (!last_node.value.valid()) {
    std::stringstream ss;
    ss << "Node " << node.id_ << " is not calculated in the forward path.";
    throw std::runtime_error(ss.str());
  }
  if (last_node.grad.valid()) {
    std::stringstream ss;
    ss << "Node " << node.id_ << " already has the gradient vector.";
    throw std::runtime_error(ss.str());
  }

  // Make identity gradient at the last node.
  last_node.grad = last_node.value.device()->constant(last_node.shape, 1);

  // The node ID represents the topological order.
  for (int id = node.id_; id >= 0; --id) {
    const NodeInfo &cur_node = *nodes_[id];
    if (!cur_node.value.valid()) {
      // Not calculated in the forward path.
      continue;
    }
    vector<const Tensor *> arg_values;
    vector<Tensor *> arg_grads;
    for (unsigned arg : cur_node.args) {
      // Gather argument value/gradient tensors.
      NodeInfo &arg_node = *nodes_[arg];
      if (!arg_node.grad.valid()) {
        arg_node.grad = arg_node.value.device()->constant(arg_node.shape, 0);
      }
      arg_values.emplace_back(&arg_node.value);
      arg_grads.emplace_back(&arg_node.grad);

      // Calculate the gradient from this node.
      cur_node.func->backward(
          cur_node.value, cur_node.grad, arg_values, arg_grads);
    }
  }
}

const Tensor &Graph::get_value(const Node &node) const {
  CHECK_NODE(node);
  return nodes_[node.id_]->value;
}

const Tensor &Graph::get_gradient(const Node &node) const {
  CHECK_NODE(node);
  return nodes_[node.id_]->grad;
}

void Graph::dump() const {
  cout << "Computation graph:" << endl;
  for (unsigned i = 0; i < nodes_.size(); ++i) {
    const NodeInfo &n = *nodes_[i];
    cout << "  [" << i << "]"
         << ": shape=" << n.shape.to_string()
         << ", func=" << n.func->name()
         << ", args=[";
    for (unsigned j = 0; j < n.args.size(); ++j) {
      if (j > 0) cout << ',';
      cout << n.args[j];
    }
    cout << "], sinks=[";
    for (unsigned j = 0; j < n.sinks.size(); ++j) {
      if (j > 0) cout << ',';
      cout << n.sinks[j];
    }
    cout << ']' << endl;
  }
}

}  // namespace primitiv