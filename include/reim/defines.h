#ifndef __REIM_DEFINES_H__
#define __REIM_DEFINES_H__

// To avoid the name mangling in C++
#ifdef __cplusplus
#define REIM_BEGIN_EXTERN_C extern "C" {
#define REIM_END_EXTERN_C }
#else
#define REIM_BEGIN_EXTERN_C
#define REIM_END_EXTERN_C
#endif

#endif
