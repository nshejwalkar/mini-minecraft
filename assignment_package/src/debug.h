#pragma once

#include <glm/glm.hpp>
#include <iostream>

#define LOG(...) \
do { \
        std::cout << __FILE__ << "(" << __LINE__ << "): " << __VA_ARGS__ << std::endl; \
} while(0)

#define LOGERR(...) \
    do { \
        std::cerr << __FILE__ << "(" << __LINE__ << "): " << __VA_ARGS__ << std::endl; \
} while(0)
