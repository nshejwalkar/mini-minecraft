#pragma once

#include "glm/glm.hpp"

constexpr float MOUSE_SENS = 0.05;
constexpr float FLY_MULT = 0.005;
constexpr float WALK_MULT = 0.001;
constexpr float JUMP_MULT = 0.08;
constexpr float DRAG = 0.85;
constexpr float MAX_TELEPORT = 200;

inline const glm::vec3 STARTING_POS = {48.f, 130.f, 48.f};
inline const glm::vec3 GRAVITY = {0.f, -0.005, 0.f};
