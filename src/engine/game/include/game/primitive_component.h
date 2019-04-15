#pragma once
#include <component/component.h>
#include <string>

namespace game {
using namespace component;
struct PrimitiveComponent /*: public Component*/ {
  //declare_component(PrimitiveComponent);
  PrimitiveComponent() : is_wireframe(false), is_visible(true), use_vertex_color(false), is_unlit(false) {
  }

  bool is_wireframe = false;
  bool is_visible = true;

  bool use_vertex_color = false;
  bool is_unlit = false;
};
} // namespace game
declare_component(game::PrimitiveComponent)
