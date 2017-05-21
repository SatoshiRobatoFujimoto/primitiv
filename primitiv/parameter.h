#ifndef PRIMITIV_PARAMETER_H_
#define PRIMITIV_PARAMETER_H_

#include <primitiv/shape.h>
#include <primitiv/tensor.h>

namespace primitiv {

class Initializer;

/**
 * Class to manage a trainable tensor parameter.
 */
class Parameter {
  Parameter() = delete;
  Parameter(const Parameter &) = delete;
  Parameter(Parameter &&) = delete;
  Parameter &operator=(const Parameter &) = delete;
  Parameter &operator=(Parameter &&) = delete;

public:
  /**
   * Creates a new Parameter object.
   * @param shape The shape of the parameter. The batch size should be 1.
   * @device The device object to manage internal memory.
   */
  Parameter(const Shape &shape, Device *device);

  /**
   * Set all values using a specific criteria.
   * @param init An Initializer object.
   */
  void reset_value(const Initializer &init);

  /**
   * Set all gradients to 0.
   */
  void reset_gradient();

  /**
   * Updates the value of the parameter.
   * @param diff A tensor representing the difference of each element.
   * @remarks This method performs: `value \gets value + diff`.
   */
  void add_value(const Tensor &diff);

  /**
   * Updates the gradient of the parameter.
   * @param diff A tensor representing the difference of each element.
   * @remarks This method performs: `grad \gets grad + diff`.
   */
  void add_gradient(const Tensor &diff);

  /**
   * Returns the shape of the parameter.
   * @return Shape object.
   */
  inline const Shape &shape() const { return shape_; }

  /**
   * Returns the Device object to manage the internal memory.
   * @return Pointer of the Device object.
   */
  inline Device *device() const { return device_; }

  /**
   * Returns the values of the parameter.
   * @return A tensor representing the parameter tensor.
   */
  inline const Tensor &value() const { return value_; }

  /**
   * Returns the current gradient of the parameter.
   * @return A tensor representing the gradient of the value.
   */
  inline const Tensor &gradient() const { return grad_; }

private:
  Shape shape_;
  Device *device_;
  Tensor value_;
  Tensor grad_;
};

}  // namespace primitiv

#endif  // PRIMITIV_PARAMETER_H_
