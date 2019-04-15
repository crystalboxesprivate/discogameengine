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
  auto &transforms = component::get_components_of_type<TransformComponent>();
  for (auto tr : transforms) {
    auto &transform = component::get_ref<TransformComponent>(tr);
    mat4 model_matrix(1.f);
    mat4 translation_matrix = translate(mat4(1.0f), transform.position);
    model_matrix = model_matrix * translation_matrix; // matMove
    quat q_rotation = transform.orient;
    mat4 rotation_matrix = mat4(q_rotation);
    model_matrix = model_matrix * rotation_matrix;
    mat4 scale_matrix = scale(mat4(1.0f), transform.scale3d);
    model_matrix = model_matrix * scale_matrix;
    transform.transform_matrix = model_matrix;
  }
}
