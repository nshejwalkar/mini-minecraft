#include "playerbounds.h"

PlayerBounds::PlayerBounds(OpenGLContext *ctx)
    : Drawable(ctx)
{}

// called every tick by mygl to update the player's onscreen verts.
void PlayerBounds::updateFromVerts(const std::array<glm::vec3, 8> &verts) {
    m_verts = verts;
    createVBOdata();
}

void PlayerBounds::createVBOdata() {
    std::vector<GLuint> idx;
    std::vector<glm::vec4> pos;
    std::vector<glm::vec4> col;

    for (int i = 0; i < 8; ++i) {
        idx.push_back(i);
        pos.push_back(glm::vec4(m_verts[i], 1.f));
        col.push_back(glm::vec4(1.f, 0.f, 0.f, 1.f));   // red
    }

    indexCounts[INDEX] = idx.size();  // always 8

    generateBuffer(POSITION);
    bindBuffer(POSITION);
    mp_context->glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec4), pos.data(), GL_DYNAMIC_DRAW);
    generateBuffer(COLOR);
    bindBuffer(COLOR);
    mp_context->glBufferData(GL_ARRAY_BUFFER, col.size() * sizeof(glm::vec4), col.data(), GL_DYNAMIC_DRAW);
    generateBuffer(INDEX);
    bindBuffer(INDEX);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(GLuint), idx.data(), GL_DYNAMIC_DRAW);
}
