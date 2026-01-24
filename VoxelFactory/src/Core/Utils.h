#pragma once
#include <iostream>

#ifdef DEBUG

#define ASSERT(cond)          if(!(cond)) { exit(-1); }
#define ASSERT_MSG(cond, msg) if(!(cond)) { std::cerr << msg << std::endl; exit(-1); }

#else
#define ASSERT(cond)
#define ASSERT_MSG(cond, msg)
#endif
