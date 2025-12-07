#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <iomanip>

#define LOG(...) \
do { \
        std::cout << __FILE__ << "(" << __LINE__ << "): " << __VA_ARGS__ << std::endl; \
} while(0)

#define LOGERR(...) \
    do { \
        std::cerr << __FILE__ << "(" << __LINE__ << "): " << __VA_ARGS__ << std::endl; \
} while(0)

inline std::ostream& operator<<(std::ostream& out, const glm::vec3& v)
{
    out << std::fixed << std::setprecision(6)
    << "("
    << std::setw(8) << v.x << ", "
    << std::setw(8) << v.y << ", "
    << std::setw(8) << v.z
    << ")";
    return out;
}
