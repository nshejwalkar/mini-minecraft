#pragma once

#include "drawable.h"

class PlayerBounds : public Drawable
{
public:
    PlayerBounds(OpenGLContext *ctx);
    void updateFromVerts(const std::array<glm::vec3, 8> &verts);
    GLenum drawMode() override {return GL_POINTS;}
    void createVBOdata() override;

private:
    std::array<glm::vec3, 8> m_verts;
};
