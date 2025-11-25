#pragma once
#include "entity.h"
#include "camera.h"
#include "terrain.h"

class Player : public Entity {
private:
    glm::vec3 m_velocity, m_acceleration;
    Camera m_camera;
    const Terrain &mcr_terrain;

    glm::vec3 moveWithCollisionSingleVector(const glm::vec3 &delta);
    void computePhysics(float dT, const Terrain &terrain);
    glm::vec3 calculateCollision();
    bool gridMarch(glm::vec3 rayOrigin,
                   glm::vec3 rayDirection,
                   const Terrain &terrain,
                   float* out_dist,
                   glm::ivec3* out_blockHit,
                   glm::ivec3* out_prevBlock = nullptr);
    bool m_touchingGround = true;
    bool m_teleporting = false;

public:
    bool flight_mode;  // changed from myGL
    bool noclip_mode;
    // Readonly public reference to our camera
    // for easy access from MyGL
    const Camera& mcr_camera;

    Player(glm::vec3 pos, const Terrain &terrain);
    virtual ~Player() override;

    // void resetEntity(glm::vec3 pos) override;

    void setCameraWidthHeight(unsigned int w, unsigned int h);

    void tick(float dT, InputBundle &input) override;

    void processInputs(InputBundle &inputs);
    bool processClick(QMouseEvent* e, glm::ivec3* out_hit, glm::ivec3* out_prevBlock);

    // Player overrides all of Entity's movement
    // functions so that it transforms its camera
    // by the same amount as it transforms itself.
    void moveAlongVector(glm::vec3 dir) override;
    void moveForwardLocal(float amount) override;
    void moveRightLocal(float amount) override;
    void moveUpLocal(float amount) override;
    void moveForwardGlobal(float amount) override;
    void moveRightGlobal(float amount) override;
    void moveUpGlobal(float amount) override;
    void rotateOnForwardLocal(float degrees) override;
    void rotateOnRightLocal(float degrees) override;
    void rotateOnUpLocal(float degrees) override;
    void rotateOnForwardGlobal(float degrees) override;
    void rotateOnRightGlobal(float degrees) override;
    void rotateOnUpGlobal(float degrees) override;

    // For sending the Player's data to the GUI
    // for display
    QString posAsQString() const;
    QString velAsQString() const;
    QString accAsQString() const;
    QString lookAsQString() const;
};
