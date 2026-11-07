#pragma once
#if defined(_MSC_VER)
#define CURRENT_FUNCTION_SIGNATURE __FUNCSIG__ 
#elif defined(__clang__) 
#define CURRENT_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#elif defined(__GNUC__)
#define CURRENT_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#else 
#define CURRENT_FUNCTION_SIGNATURE "Unkown Compiler: Unknown Function"
#endif