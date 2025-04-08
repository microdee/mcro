
#pragma once

// This header is a wrapper around ctre.h, making it usable in Unreal Engine.

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
#endif

// temporarily undefine simplistic Unreal macros

#pragma push_macro("TEXT")
#pragma push_macro("CONSTEXPR")
#pragma push_macro("PI")
#pragma push_macro("dynamic_cast")
#pragma push_macro("VARARGS")
#pragma push_macro("CDECL")
#pragma push_macro("STDCALL")
#pragma push_macro("FORCEINLINE")
#pragma push_macro("FORCENOINLINE")
#pragma push_macro("ABSTRACT")
#pragma push_macro("LINE_TERMINATOR")
#pragma push_macro("LINE_TERMINATOR_ANSI")
#pragma push_macro("DLLEXPORT")
#pragma push_macro("DLLIMPORT")
#pragma push_macro("LIKELY")
#pragma push_macro("UNLIKELY")
#pragma push_macro("RESTRICT")
#pragma push_macro("MOBILE")
#pragma push_macro("CONSOLE")
#pragma push_macro("PLATFORM_WINDOWS")
#pragma push_macro("PLATFORM_COMPILER_CLANG")
#pragma push_macro("PLATFORM_APPLE")
#pragma push_macro("PLATFORM_MAC")
#pragma push_macro("PLATFORM_LINUX")
#pragma push_macro("PLATFORM_FREEBSD")
#pragma push_macro("PLATFORM_UNIX")

#undef TEXT
#undef CONSTEXPR
#undef PI
#undef dynamic_cast
#undef VARARGS
#undef CDECL
#undef STDCALL
#undef FORCEINLINE
#undef FORCENOINLINE
#undef ABSTRACT
#undef LINE_TERMINATOR
#undef LINE_TERMINATOR_ANSI
#undef DLLEXPORT
#undef DLLIMPORT
#undef LIKELY
#undef UNLIKELY
#undef RESTRICT
#undef MOBILE
#undef CONSOLE
#undef PLATFORM_WINDOWS
#undef PLATFORM_COMPILER_CLANG
#undef PLATFORM_APPLE
#undef PLATFORM_MAC
#undef PLATFORM_LINUX
#undef PLATFORM_FREEBSD
#undef PLATFORM_UNIX

#pragma push_macro("DO_CHECK")
#pragma push_macro("checkCode")
#pragma push_macro("check")
#pragma push_macro("checkf")
#pragma push_macro("verify")
#pragma push_macro("verifyf")
#pragma push_macro("unimplemented")
#pragma push_macro("ensure")

#undef DO_CHECK
#undef checkCode
#undef check
#undef checkf
#undef verify
#undef verifyf
#undef unimplemented
#undef ensure

#define CTRE_INCL_STR2(x) #x
#define CTRE_INCL_STR(x) CTRE_INCL_STR2(x)

// prefer MSVC compiler macros over GCC if they're both present for some god forsaken reason
#if defined(_MSC_VER) && defined(__GNUC__)

#if _MSC_VER > 0
#pragma message ("CTRE: _MSC_VER - __GNUC__ macro collision detected, temporarily undefining __GNUC__ (GCC: " CTRE_INCL_STR(__GNUC__) ", MSVC: " CTRE_INCL_STR(_MSC_VER) ")")
#define CTRE_MSVC_GNUC_AVOIDANCE __GNUC__
#pragma push_macro("__GNUC__")
#undef __GNUC__
#endif

#endif

#undef CTRE_INCL_STR
#undef CTRE_INCL_STR2

#pragma warning(push)
#pragma warning(disable: 4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'

#include "ctre.hpp"

#pragma warning(pop)

// restore temporary macro undefs

#pragma pop_macro("TEXT")
#pragma pop_macro("CONSTEXPR")
#pragma pop_macro("PI")
#pragma pop_macro("dynamic_cast")
#pragma pop_macro("VARARGS")
#pragma pop_macro("CDECL")
#pragma pop_macro("STDCALL")
#pragma pop_macro("FORCEINLINE")
#pragma pop_macro("FORCENOINLINE")
#pragma pop_macro("ABSTRACT")
#pragma pop_macro("LINE_TERMINATOR")
#pragma pop_macro("LINE_TERMINATOR_ANSI")
#pragma pop_macro("DLLEXPORT")
#pragma pop_macro("DLLIMPORT")
#pragma pop_macro("LIKELY")
#pragma pop_macro("UNLIKELY")
#pragma pop_macro("RESTRICT")
#pragma pop_macro("MOBILE")
#pragma pop_macro("CONSOLE")
#pragma pop_macro("PLATFORM_WINDOWS")
#pragma pop_macro("PLATFORM_COMPILER_CLANG")
#pragma pop_macro("PLATFORM_APPLE")
#pragma pop_macro("PLATFORM_MAC")
#pragma pop_macro("PLATFORM_LINUX")
#pragma pop_macro("PLATFORM_FREEBSD")
#pragma pop_macro("PLATFORM_UNIX")

#pragma pop_macro("DO_CHECK")
#pragma pop_macro("checkCode")
#pragma pop_macro("check")
#pragma pop_macro("checkf")
#pragma pop_macro("verify")
#pragma pop_macro("verifyf")
#pragma pop_macro("unimplemented")
#pragma pop_macro("ensure")

#ifdef CTRE_MSVC_GNUC_AVOIDANCE
#pragma pop_macro("__GNUC__")
#endif

#if PLATFORM_WINDOWS
#include "Windows/HideWindowsPlatformAtomics.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif