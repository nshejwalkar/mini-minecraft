#include "player.h"
#include "constants.h"
#include <QString>
#include <iostream>
#include "debug.h"
#include "qevent.h"

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

bool Player::processClick(QMouseEvent* e, glm::ivec3* out_hit, glm::ivec3* out_prevBlock) {
    float out_dist;

    if (this->gridMarch(m_camera.mcr_position,
                        m_forward*(3/glm::length(m_forward)),
                        mcr_terrain,
                        &out_dist,
                        out_hit,
                        out_prevBlock)) {
        if (e->button() == Qt::LeftButton) {
            LOG("clicked left, found hit at " << out_hit->x << ", " << out_hit->y << ", " << out_hit->z);
            return true;
        }
        else if (e->button() == Qt::RightButton) {
            LOG("clicked right, found hit at " << out_hit->x << ", " << out_hit->y << ", " << out_hit->z);
            return true;
        }
    }
    return false;
}

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
        if (inputs.qPressed || inputs.spacePressed) {
            m_acceleration += WALK_MULT * m_up;
        }
        if (inputs.ePressed || inputs.shiftPressed) {
            m_acceleration -= WALK_MULT * m_up;
        }
    }
    else {
        glm::vec3 fwd_2d = glm::normalize(glm::vec3(m_forward.x, 0.f, m_forward.z));
        glm::vec3 right_2d = glm::normalize(glm::vec3(m_right.x, 0.f, m_right.z));
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
        if (inputs.spacePressed && !m_jumping) {
            m_acceleration += WALK_MULT * m_up;
            m_jumping = true;
        }
    }

    // move to computePhysics?
    this->rotateOnUpGlobal(-MOUSE_SENS*inputs.mouseX);
    this->rotateOnRightLocal(-MOUSE_SENS*inputs.mouseY);
}

bool Player::gridMarch(glm::vec3 rayOrigin,
                       glm::vec3 rayDirection,
                       const Terrain &terrain,
                       float* out_dist,
                       glm::ivec3* out_blockHit,
                       glm::ivec3* out_prevBlock) {
    float maxLen = glm::length(rayDirection);
    glm::ivec3 currCell = glm::ivec3(glm::floor(rayOrigin));
    glm::ivec3 prevCell = currCell;
    rayDirection = glm::normalize(rayDirection);

    float curr_t = 0.f;
    while (curr_t < maxLen) {
        float min_t = glm::sqrt(3.f);
        float interfaceAxis = -1;
        for (int i = 0; i < 3; i++) {
            if (rayDirection[i] != 0) {
                float offset = glm::max(0.f, glm::sign(rayDirection[i]));

                if (currCell[i] == rayOrigin[i] && offset == 0.f) {
                    offset = -1.f;
                }
                int nextIntercept = currCell[i] + offset;
                float axis_t = (nextIntercept - rayOrigin[i]) / rayDirection[i];
                axis_t = glm::min(axis_t, maxLen);
                if (axis_t < min_t) {
                    min_t = axis_t;
                    interfaceAxis = i;
                }
            }
        }
        if (interfaceAxis == -1) {
            throw std::out_of_range("interfaceAxis was -1 after the for loop in gridMarch!");
        }
        curr_t += min_t;
        rayOrigin += rayDirection * min_t;
        glm::ivec3 offset = glm::ivec3(0.f);

        offset[interfaceAxis] = glm::min(0.f, glm::sign(rayDirection[interfaceAxis]));
        prevCell = currCell;
        currCell = glm::ivec3(glm::floor(rayOrigin)) + offset;

        BlockType cellType;
        try {cellType = terrain.getGlobalBlockAt(currCell.x, currCell.y, currCell.z);}
        catch (const std::out_of_range& e) {
            LOGERR("Out of range: " << e.what());
            break;
        }

        if (cellType != EMPTY) {
            *out_blockHit = currCell;
            if (out_prevBlock) *out_prevBlock = prevCell;
            *out_dist = glm::min(maxLen, curr_t);
            return true;
        }
    }
    *out_dist = glm::min(maxLen, curr_t);
    return false;
}

// calculate collision: for all 8 vertices' rayorigins, perform grid marching
// grid marching should return what axis side we're hitting (x,y,z) if anything
// m_velocity should *= with that bitmask to perform sliding
float Player::calculateCollision() {
    // for pos+(0.5,0,0.5) .... pos+(0.5,2,0.5)....
    std::array<glm::vec3, 8> vertices = {
        m_position + glm::vec3(0.5,0,0.5),
        m_position + glm::vec3(0.5,0,-0.5),
        m_position + glm::vec3(-0.5,0,0.5),
        m_position + glm::vec3(-0.5,0,-0.5),
        m_position + glm::vec3(0.5,2,0.5),
        m_position + glm::vec3(0.5,2,-0.5),
        m_position + glm::vec3(-0.5,2,0.5),
        m_position + glm::vec3(-0.5,2,-0.5)
    };
    float smallestCollision = 100;
    for (auto vertex : vertices) {
        float out_dist;
        glm::ivec3 out_hit;
        if (this->gridMarch(vertex, m_velocity, mcr_terrain, &out_dist, &out_hit)) {
            // LOG("COLLISION! out_dist: " << out_dist << ", out_hit: " << out_hit.x << ", " << out_hit.y << ", " << out_hit.z);
            smallestCollision = glm::min(out_dist, smallestCollision);
        }
    }
    return smallestCollision;
    // gridmarch(m_velocity)
    // as soon as any return true, dont finish. if we intersect two+ axes we want to differentiate that
}

void Player::computePhysics(float dT, const Terrain &terrain) {
    // Update the Player's position based on its acceleration
    // and velocity, and also perform collision detection.

    if (!flight_mode) m_acceleration += GRAVITY;
    LOG(m_acceleration.x << " " << m_acceleration.y << " " << m_acceleration.z);
    m_velocity = (DRAG*m_velocity) + m_acceleration * dT;
    // LOG("velocity before: " << m_velocity.x << ", " << m_velocity.y << ", " << m_velocity.z);
    float minColDist = this->calculateCollision();
    if (minColDist < 1) {
        m_velocity *= minColDist;
    }
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
