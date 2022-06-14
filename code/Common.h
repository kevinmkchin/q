#pragma once


#include <fstream>
#include <string>
#include <vector>

#include "Timer.h"
#include "Utils.h"
#include "MessageLogger.h"


// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
#include "../lib/project_path.h" // Get defintion of PROJECT_SOURCE_DIR from this CMake created header
inline std::string wd_path() { return std::string(PROJECT_SOURCE_DIR); }
inline std::string wd_path(const std::string& name) { return wd_path() + std::string(name); }
inline std::string data_path() { return wd_path() + "data/"; }
inline std::string data_path(const std::string& name) { return wd_path() + "data/" + name; }
inline std::string shader_path(const std::string& name) { return wd_path() + "shaders/" + name; }
inline std::string textures_path(const std::string& name) { return data_path() + "textures/" + std::string(name); }
inline std::string scripts_path(const std::string& name) { return data_path() + "scripts/" + std::string(name); }
inline std::string audio_path(const std::string& name) { return data_path() + "audio/" + std::string(name); }
inline std::string mesh_path(const std::string& name) { return data_path() + "meshes/" + std::string(name); }
inline std::string level_path(const std::string& name) { return data_path() + "level/" + std::string(name); }
inline std::string font_path(const std::string& name) { return data_path() + "fonts/" + std::string(name); }
inline std::string primitives_path(const std::string& name) { return data_path() + "primitives/" + std::string(name); }

#define ARRAY_COUNT(a) (sizeof(a) / (sizeof(a[0])))

/** HELPER FUNCTIONS **/
#if SLOW_BUILD
#if (defined _WIN64)
#define ASSERT(predicate) if(!(predicate)) {*(int*)0 = 0;}
#else
#define ASSERT(predicate) if(!(predicate)) { __builtin_trap(); }
#endif
#else
#define ASSERT(predicate)
#endif

#define INTERNAL static
#define LOCAL_PERSIST static

#define _RGB(r,g,b) ((float)r)/255.f, ((float)g)/255.f, ((float)b)/255.f

typedef uint8_t       u8;
typedef uint16_t      u16;
typedef uint32_t      u32;
typedef uint64_t      u64;
typedef int8_t        i8;
typedef int16_t       i16;
typedef int32_t       i32;
typedef int64_t       i64;
typedef uint_fast8_t  u8f;
typedef uint_fast16_t u16f;
typedef uint_fast32_t u32f;
typedef int_fast8_t   i8f;
typedef int_fast16_t  i16f;
typedef int_fast32_t  i32f;
typedef i16           bool16;
typedef i32           bool32;
