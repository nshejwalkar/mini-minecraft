#pragma once
#include "drawable.h"

class Crosshair : public Drawable
{
public:
    Crosshair(OpenGLContext* context) : Drawable(context){};
    void createVBOdata() override;
    GLenum drawMode() override;
};
