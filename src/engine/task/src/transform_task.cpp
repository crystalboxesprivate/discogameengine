#include <task/transform_task.h>
#include <game/transform_component.h>

using game::TransformComponent;
using namespace task;
using namespace glm;

void TransformTask::init() {
}
void TransformTask::update() {
  using namespace glm;
  // get components of type transform
  auto &transforms = component::get_array_of_components<TransformComponent>();
  for (auto &transform : transforms) {
     transform.transform_matrix_previous = transform.transform_matrix;
     transform.transform_matrix = translate(mat4(1.0f), transform.position) * mat4(transform.rotation) * scale(mat4(1.0f), transform.scale);
  }
}
