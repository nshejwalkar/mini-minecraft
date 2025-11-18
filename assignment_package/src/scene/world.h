#pragma once

class World
{
public:
    // Constructor
    World();

    // Gets the height at (x, z)
    int getHeight(float x, float z) const;
};
