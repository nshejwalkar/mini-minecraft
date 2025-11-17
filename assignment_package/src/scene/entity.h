#pragma once
#include "glm_includes.h"
#include <glm/gtx/vector_angle.hpp>
#include <ostream>

struct InputBundle {
    bool wPressed, aPressed, sPressed, dPressed, ePressed, qPressed;
    bool spacePressed, shiftPressed;
    float mouseX, mouseY;

    InputBundle()
        : wPressed(false), aPressed(false), sPressed(false),
        dPressed(false), ePressed(false), qPressed(false),
        spacePressed(false), shiftPressed(false), mouseX(0.f), mouseY(0.f)
    {}
};

inline std::ostream& operator<<(std::ostream& os, const InputBundle& ib) {
    os << "InputBundle { "
       << "w=" << ib.wPressed << ", "
       << "a=" << ib.aPressed << ", "
       << "s=" << ib.sPressed << ", "
       << "d=" << ib.dPressed << ", "
       << "e=" << ib.ePressed << ", "
       << "q=" << ib.qPressed << ", "
       << "space=" << ib.spacePressed << ", "
       << "mouseX=" << ib.mouseX << ", "
       << "mouseY=" << ib.mouseY
       << " }";
    return os;
}

class Entity {
protected:
    // Vectors that define the axes of our local coordinate system
    glm::vec3 m_forward, m_right, m_up;
    // The origin of our local coordinate system
    glm::vec3 m_position;

public:
    // A readonly reference to position for external use
    const glm::vec3& mcr_position;

    // Various constructors
    Entity();
    Entity(glm::vec3 pos);
    Entity(const Entity &e);
    virtual ~Entity();

    // reset position (player and camera)
    // virtual void resetEntity(glm::vec3 pos);

    // To be called by MyGL::tick()
    virtual void tick(float dT, InputBundle &input) = 0;

    // Translate along the given vector
    virtual void moveAlongVector(glm::vec3 dir);

    // Translate along one of our local axes
    virtual void moveForwardLocal(float amount);
    virtual void moveRightLocal(float amount);
    virtual void moveUpLocal(float amount);

    // Translate along one of the world axes
    virtual void moveForwardGlobal(float amount);
    virtual void moveRightGlobal(float amount);
    virtual void moveUpGlobal(float amount);

    // Rotate about one of our local axes
    virtual void rotateOnForwardLocal(float degrees);
    virtual void rotateOnRightLocal(float degrees);
    virtual void rotateOnUpLocal(float degrees);

    // Rotate about one of the world axes
    virtual void rotateOnForwardGlobal(float degrees);
    virtual void rotateOnRightGlobal(float degrees);
    virtual void rotateOnUpGlobal(float degrees);
};
