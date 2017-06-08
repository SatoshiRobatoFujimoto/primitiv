#ifndef PRIMITIV_SHAPE_H_
#define PRIMITIV_SHAPE_H_

#include <initializer_list>
#include <string>
#include <vector>

namespace primitiv {

/**
 * Data structure to represent the shape of the node.
 *
 * Examples:
 *   Shape()         == Shape({1, 1, 1, ...}, 1): scalar
 *   Shape({})       == Shape({1, 1, 1, ...}, 1): scalar
 *   Shape({n})      == Shape({n, 1, 1, ...}, 1): row vector
 *   Shape({n, m})   == Shape({n, m, 1, ...}, 1): matrix
 *   Shape({...}, k): k-parallelized data (mini-batch)
 */
class Shape {
public:
  /**
   * Creates a new scalar Shape object.
   */
  Shape() : dims_(), k_(1), num_elms_per_sample_(1) {}

  /**
   * Creates a new Shape object.
   * @param dims List of the dimension sizes.
   * @param k Batch size.
   */
  Shape(const std::initializer_list<unsigned> &dims, const unsigned k = 1);

  /**
   * Creates a new Shape object.
   * @param dims List of the dimension sizes.
   * @param k Batch size.
   */
  Shape(const std::vector<unsigned> &dims, const unsigned k = 1);

  /**
   * Returns the size of the i-th dimension.
   * @param i Dimension number to check.
   * @return Size of the i-th dimension.
   */
  unsigned operator[](const unsigned i) const {
    return i < depth() ? dims_[i] : 1;
  }

  /**
   * Returns the depth (length of non-1 dimensions) of the shape.
   * @return The depth of the shape.
   */
  unsigned depth() const { return dims_.size(); }

  /**
   * Returns the list of dimension sizes.
   * @return List of the dimension sizes.
   */
  const std::vector<unsigned> dims() const { return dims_; }

  /**
   * Returns the batch size.
   * @return Batch size.
   */
  unsigned batch_size() const { return k_; }

  /**
   * Returns the number of elements in each sample.
   * This value is equal to the product of all dimensions.
   * @return Number of elements.
   */
  unsigned num_elements_per_sample() const { return num_elms_per_sample_; }

  /**
   * Returns the number of elements in 1 to specified rank.
   * @param rank Upper bound of the dimension.
   * @return `dim[0] * dim[1] * ... * dim[rank-1]`
   */
  unsigned num_elements_under_rank(unsigned rank) const;

  /**
   * Returns the number of elements in all samples of the mini-batch.
   * This value is equal to `batch_size() * num_elements_per_sample()`.
   * @return Number of elements.
   */
  unsigned num_total_elements() const { return k_ * num_elms_per_sample_; }

  /**
   * Returns a string representation of the shape.
   * The format is: "[n,m,...]xk"
   * @return Encoded string.
   */
  std::string to_string() const;

  /**
   * Compares this and other shape.
   * @param rhs Shape object to compare.
   * @return true if this and rhs are same, false otherwise.
   */
  bool operator==(const Shape &rhs) const {
    return has_same_dims(rhs) && k_ == rhs.k_;
  }

  /**
   * Compares this and other shape.
   * @param rhs Shape object to compare.
   * @return true if this and rhs are not same, false otherwise.
   */
  bool operator!=(const Shape &rhs) const { return !operator==(rhs); }

  /**
   * Checks whether two batch size is compatible (broadcastable) or not.
   * @param rhs Shape object to compare.
   * @return true if both batch size is compatible, false otherwise.
   */
  bool has_compatible_batch(const Shape &rhs) const {
    return k_ == rhs.k_ || k_ == 1 || rhs.k_ == 1;
  }

  /**
   * Checks whether two shapes have completely same dimensions.
   * @param rhs Shape object to compare.
   * @return true if both shape have same dimensions, false otherwise.
   */
  bool has_same_dims(const Shape &rhs) const {
    return dims_ == rhs.dims_;
  }

  /**
   * Checks whether two shapes have same dimensions without an axis.
   * (LOO: leave one out)
   * @param rhs Shape object to compare.
   * @param dim Dimension to be ignored.
   * @return true if both shape have same dimensions regardless the dimension
   *         `dim`, false otherwise.
   */
  bool has_same_loo_dims(const Shape &rhs, unsigned dim) const;

  /**
   * Creates a new shape which have one different dimension.
   * @param dim Dimension to be changed.
   * @param m New size of the dimension `dim`.
   * @return New shape.
   */
  Shape resize_dim(unsigned dim, unsigned m) const;

  /**
   * Creates a new shape which have specified batch size.
   * @param k New batch size.
   * @return New shape.
   */
  Shape resize_batch(unsigned k) const;

  /**
   * Directly updates a specified dimension.
   * @param dim Dimension to be updated.
   * @param m New size of the dimension `dim`.
   */
  void update_dim(unsigned dim, unsigned m);

  /**
   * Directly updates the batch size.
   * @param k New batch size.
   */
  void update_batch(unsigned k);

private:
  std::vector<unsigned> dims_;
  unsigned k_;
  unsigned num_elms_per_sample_;

  /**
   * Check internal values and adjust them.
   */
  void adjust();
};

}  // namespace primitiv

#endif  // PRIMITIV_SHAPE_H_
