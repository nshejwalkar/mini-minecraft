#include "player.h"
#include "constants.h"
#include <QString>
#include <iostream>
#include "debug.h"

Player::Player(glm::vec3 pos, const Terrain &terrain)
    : Entity(pos),
      m_velocity(0,0,0),
      m_acceleration(0,0,0),
      m_camera(pos + glm::vec3(0, 1.5f, 0)),
      mcr_terrain(terrain),
      flight_mode(true),
      mcr_camera(m_camera)
{}

Player::~Player()
{}

// void Player::resetEntity(glm::vec3 pos, const Terrain &terrain) {
//     *this = Player(pos, terrain);
// }

void Player::tick(float dT, InputBundle &input) {
    processInputs(input);
    computePhysics(dT, mcr_terrain);
}

void Player::processInputs(InputBundle &inputs) {
    // Update the Player's velocity and acceleration based on the
    // state of the inputs.
    // LOG(inputs);

    m_acceleration = glm::vec3(0.0f);

    if (this->flight_mode) {
        if (inputs.wPressed) {
            m_acceleration += WALK_MULT * m_forward;
        }
        if (inputs.sPressed) {
            m_acceleration -= WALK_MULT * m_forward;
        }
        if (inputs.dPressed) {
            m_acceleration += WALK_MULT * m_right;
        }
        if (inputs.aPressed) {
            m_acceleration -= WALK_MULT * m_right;
        }
        if (inputs.qPressed) {
            m_acceleration += WALK_MULT * m_up;
        }
        if (inputs.ePressed) {
            m_acceleration -= WALK_MULT * m_up;
        }
    }
    else {
        glm::vec3 fwd_2d = glm::normalize(glm::vec3(m_forward.x, 0.f, m_forward.z));
        glm::vec3 right_2d = glm::normalize(glm::vec3(m_right.x,   0.f, m_right.z));
        if (inputs.wPressed) {
            m_acceleration += WALK_MULT * fwd_2d;
        }
        if (inputs.sPressed) {
            m_acceleration -= WALK_MULT * fwd_2d;
        }
        if (inputs.dPressed) {
            m_acceleration += WALK_MULT * right_2d;
        }
        if (inputs.aPressed) {
            m_acceleration -= WALK_MULT * right_2d;
        }
    }

    // move to computePhysics?
    this->rotateOnUpGlobal(-MOUSE_SENS*inputs.mouseX);
    this->rotateOnRightLocal(-MOUSE_SENS*inputs.mouseY);
}

void Player::computePhysics(float dT, const Terrain &terrain) {
    // Update the Player's position based on its acceleration
    // and velocity, and also perform collision detection.
    m_velocity = (DRAG*m_velocity) + m_acceleration * dT;
    moveAlongVector(m_velocity);
}

void Player::setCameraWidthHeight(unsigned int w, unsigned int h) {
    m_camera.setWidthHeight(w, h);
}

void Player::moveAlongVector(glm::vec3 dir) {
    Entity::moveAlongVector(dir);
    m_camera.moveAlongVector(dir);
}
void Player::moveForwardLocal(float amount) {
    Entity::moveForwardLocal(amount);
    m_camera.moveForwardLocal(amount);
}
void Player::moveRightLocal(float amount) {
    Entity::moveRightLocal(amount);
    m_camera.moveRightLocal(amount);
}
void Player::moveUpLocal(float amount) {
    Entity::moveUpLocal(amount);
    m_camera.moveUpLocal(amount);
}
void Player::moveForwardGlobal(float amount) {
    Entity::moveForwardGlobal(amount);
    m_camera.moveForwardGlobal(amount);
}
void Player::moveRightGlobal(float amount) {
    Entity::moveRightGlobal(amount);
    m_camera.moveRightGlobal(amount);
}
void Player::moveUpGlobal(float amount) {
    Entity::moveUpGlobal(amount);
    m_camera.moveUpGlobal(amount);
}
void Player::rotateOnForwardLocal(float degrees) {
    Entity::rotateOnForwardLocal(degrees);
    m_camera.rotateOnForwardLocal(degrees);
}
void Player::rotateOnRightLocal(float degrees) {
    Entity::rotateOnRightLocal(degrees);
    m_camera.rotateOnRightLocal(degrees);
}
void Player::rotateOnUpLocal(float degrees) {
    Entity::rotateOnUpLocal(degrees);
    m_camera.rotateOnUpLocal(degrees);
}
void Player::rotateOnForwardGlobal(float degrees) {
    Entity::rotateOnForwardGlobal(degrees);
    m_camera.rotateOnForwardGlobal(degrees);
}
void Player::rotateOnRightGlobal(float degrees) {
    Entity::rotateOnRightGlobal(degrees);
    m_camera.rotateOnRightGlobal(degrees);
}
void Player::rotateOnUpGlobal(float degrees) {
    Entity::rotateOnUpGlobal(degrees);
    m_camera.rotateOnUpGlobal(degrees);
}

QString Player::posAsQString() const {
    std::string str("( " + std::to_string(m_position.x) + ", " + std::to_string(m_position.y) + ", " + std::to_string(m_position.z) + ")");
    return QString::fromStdString(str);
}
QString Player::velAsQString() const {
    std::string str("( " + std::to_string(m_velocity.x) + ", " + std::to_string(m_velocity.y) + ", " + std::to_string(m_velocity.z) + ")");
    return QString::fromStdString(str);
}
QString Player::accAsQString() const {
    std::string str("( " + std::to_string(m_acceleration.x) + ", " + std::to_string(m_acceleration.y) + ", " + std::to_string(m_acceleration.z) + ")");
    return QString::fromStdString(str);
}
QString Player::lookAsQString() const {
    std::string str("( " + std::to_string(m_forward.x) + ", " + std::to_string(m_forward.y) + ", " + std::to_string(m_forward.z) + ")");
    return QString::fromStdString(str);
}
