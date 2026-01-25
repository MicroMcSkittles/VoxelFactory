#pragma once
#include <iostream>
#include <format>

#ifdef DEBUG

#define ASSERT(cond)               if(!(cond)) { exit(-1); }
#define ASSERT_MSG(cond, msg, ...) if(!(cond)) { std::cerr << std::format(msg, __VA_ARGS__) << std::endl; exit(-1); }

#else
#define ASSERT(cond)
#define ASSERT_MSG(cond, msg, ...)
#endif
