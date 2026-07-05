#pragma once
#include <iostream>
#include <format>

inline bool FloatEquals(float f1, float f2, float epsilon = 0.001f) {
	return (f1 >= f2 - epsilon && f1 <= f2 + epsilon);
}

#define BIT(x) 1 << x

#define VEC2_STR(v) std::format("( {:.2f}, {:.2f} )", v.x, v.y)
#define IVEC2_STR(v) std::format("( {}, {} )", v.x, v.y)
#define VEC3_STR(v) std::format("( {:.2f}, {:.2f}, {:.2f} )", v.x, v.y, v.z)

#ifdef DEBUG

#define ASSERT(cond)               if(!(cond)) { exit(-1); }
#define ASSERT_MSG(cond, msg, ...) if(!(cond)) { std::cerr << std::format(msg, __VA_ARGS__) << std::endl; exit(-1); }

#else
#define ASSERT(cond)
#define ASSERT_MSG(cond, msg, ...)

#endif