/** @noop License Comment
 *  @file
 *  @copyright
 *  This Source Code is subject to the terms of the Mozilla Public License, v2.0.
 *  If a copy of the MPL was not distributed with this file You can obtain one at
 *  https://mozilla.org/MPL/2.0/
 *  
 *  @author David Mórász
 *  @date 2025
 */

/**
 * @file
 * @brief
 * Use this header and `End.h` in tandem to include third-party library headers which may not tolerate Unreal's common
 * vocabulary macros or its stricter warning policies.
 * 
 * With `MCRO_ALLOW_*` macros one can control if a the associated macro should be hidden from the library or not. By
 * default they're all disallowed (0). For example some COM headers expect `TEXT` to be defined, so one can do
 * @code
 * #define MCRO_ALLOW_TEXT 1
 * #include "Mcro/LibraryIncludes/Start.h"
 * #include "comdef.h"
 * #include "Mcro/LibraryIncludes/End.h"
 * @endcode
 *
 * Similarly platform specific include guards like `AllowWindowsPlatformTypes` and `AllowWindowsPlatformAtomics` can
 * be controlled with `MCRO_ALLOW_<platform>_TYPES` macro. These are allowed by default (1).
 * 
 * In most cases these 'allow' macros are not needed. Only use them when the wrapped library headers cause compile error
 * for a missing macro they expected or if 'platform-types' interfere with them. The meaning of these macros are reset
 * to their default after each inclusion of `End.h`
 */

#include "Mcro/Macros.h"

#ifndef MCRO_ALLOW_WINDOWS_TYPES
#define MCRO_ALLOW_WINDOWS_TYPES 1
#endif

#ifndef MCRO_ALLOW_TEXT
#define MCRO_ALLOW_TEXT 0
#endif
#ifndef MCRO_ALLOW_TRUE
#define MCRO_ALLOW_TRUE 0
#endif
#ifndef MCRO_ALLOW_FALSE
#define MCRO_ALLOW_FALSE 0
#endif
#ifndef MCRO_ALLOW_MAX_uint8
#define MCRO_ALLOW_MAX_uint8 0
#endif
#ifndef MCRO_ALLOW_MAX_uint16
#define MCRO_ALLOW_MAX_uint16 0
#endif
#ifndef MCRO_ALLOW_MAX_uint32
#define MCRO_ALLOW_MAX_uint32 0
#endif
#ifndef MCRO_ALLOW_MAX_int32
#define MCRO_ALLOW_MAX_int32 0
#endif
#ifndef MCRO_ALLOW_CONSTEXPR
#define MCRO_ALLOW_CONSTEXPR 0
#endif
#ifndef MCRO_ALLOW_PI
#define MCRO_ALLOW_PI 0
#endif
#ifndef MCRO_ALLOW_dynamic_cast
#define MCRO_ALLOW_dynamic_cast 0
#endif
#ifndef MCRO_ALLOW_VARARGS
#define MCRO_ALLOW_VARARGS 0
#endif
#ifndef MCRO_ALLOW_CDECL
#define MCRO_ALLOW_CDECL 0
#endif
#ifndef MCRO_ALLOW_STDCALL
#define MCRO_ALLOW_STDCALL 0
#endif
#ifndef MCRO_ALLOW_FORCEINLINE
#define MCRO_ALLOW_FORCEINLINE 0
#endif
#ifndef MCRO_ALLOW_FORCENOINLINE
#define MCRO_ALLOW_FORCENOINLINE 0
#endif
#ifndef MCRO_ALLOW_ABSTRACT
#define MCRO_ALLOW_ABSTRACT 0
#endif
#ifndef MCRO_ALLOW_LINE_TERMINATOR
#define MCRO_ALLOW_LINE_TERMINATOR 0
#endif
#ifndef MCRO_ALLOW_LINE_TERMINATOR_ANSI
#define MCRO_ALLOW_LINE_TERMINATOR_ANSI 0
#endif
#ifndef MCRO_ALLOW_DLLEXPORT
#define MCRO_ALLOW_DLLEXPORT 0
#endif
#ifndef MCRO_ALLOW_DLLIMPORT
#define MCRO_ALLOW_DLLIMPORT 0
#endif
#ifndef MCRO_ALLOW_LIKELY
#define MCRO_ALLOW_LIKELY 0
#endif
#ifndef MCRO_ALLOW_UNLIKELY
#define MCRO_ALLOW_UNLIKELY 0
#endif
#ifndef MCRO_ALLOW_RESTRICT
#define MCRO_ALLOW_RESTRICT 0
#endif
#ifndef MCRO_ALLOW_MOBILE
#define MCRO_ALLOW_MOBILE 0
#endif
#ifndef MCRO_ALLOW_CONSOLE
#define MCRO_ALLOW_CONSOLE 0
#endif
#ifndef MCRO_ALLOW_DEFAULTS
#define MCRO_ALLOW_DEFAULTS 0
#endif

#ifdef NON_UNREAL_INCLUDE_REGION
#error Third-party library or non-unreal include regions cannot be nested, one region is already opened before
#endif
#define NON_UNREAL_INCLUDE_REGION 1

#pragma warning( push )
#pragma warning( disable : 4005 5205 4265 4268 4946 4103 )

// temporarily undefine simplistic Unreal macros third-party libraries might also use

#if !MCRO_ALLOW_TEXT
#pragma push_macro("TEXT")
#undef TEXT
#endif
#if !MCRO_ALLOW_TRUE
#pragma push_macro("TRUE")
#undef TRUE
#endif
#if !MCRO_ALLOW_FALSE
#pragma push_macro("FALSE")
#undef FALSE
#endif
#if !MCRO_ALLOW_MAX_uint8
#pragma push_macro("MAX_uint8")
#undef MAX_uint8
#endif
#if !MCRO_ALLOW_MAX_uint16
#pragma push_macro("MAX_uint16")
#undef MAX_uint16
#endif
#if !MCRO_ALLOW_MAX_uint32
#pragma push_macro("MAX_uint32")
#undef MAX_uint32
#endif
#if !MCRO_ALLOW_MAX_int32
#pragma push_macro("MAX_int32")
#undef MAX_int32
#endif
#if !MCRO_ALLOW_CONSTEXPR
#pragma push_macro("CONSTEXPR")
#undef CONSTEXPR
#endif
#if !MCRO_ALLOW_PI
#pragma push_macro("PI")
#undef PI
#endif
#if !MCRO_ALLOW_dynamic_cast
#pragma push_macro("dynamic_cast")
#undef dynamic_cast
#endif
#if !MCRO_ALLOW_VARARGS
#pragma push_macro("VARARGS")
#undef VARARGS
#endif
#if !MCRO_ALLOW_CDECL
#pragma push_macro("CDECL")
#undef CDECL
#endif
#if !MCRO_ALLOW_STDCALL
#pragma push_macro("STDCALL")
#undef STDCALL
#endif
#if !MCRO_ALLOW_FORCEINLINE
#pragma push_macro("FORCEINLINE")
#undef FORCEINLINE
#endif
#if !MCRO_ALLOW_FORCENOINLINE
#pragma push_macro("FORCENOINLINE")
#undef FORCENOINLINE
#endif
#if !MCRO_ALLOW_ABSTRACT
#pragma push_macro("ABSTRACT")
#undef ABSTRACT
#endif
#if !MCRO_ALLOW_LINE_TERMINATOR
#pragma push_macro("LINE_TERMINATOR")
#undef LINE_TERMINATOR
#endif
#if !MCRO_ALLOW_LINE_TERMINATOR_ANSI
#pragma push_macro("LINE_TERMINATOR_ANSI")
#undef LINE_TERMINATOR_ANSI
#endif
#if !MCRO_ALLOW_DLLEXPORT
#pragma push_macro("DLLEXPORT")
#undef DLLEXPORT
#endif
#if !MCRO_ALLOW_DLLIMPORT
#pragma push_macro("DLLIMPORT")
#undef DLLIMPORT
#endif
#if !MCRO_ALLOW_LIKELY
#pragma push_macro("LIKELY")
#undef LIKELY
#endif
#if !MCRO_ALLOW_UNLIKELY
#pragma push_macro("UNLIKELY")
#undef UNLIKELY
#endif
#if !MCRO_ALLOW_RESTRICT
#pragma push_macro("RESTRICT")
#undef RESTRICT
#endif
#if !MCRO_ALLOW_MOBILE
#pragma push_macro("MOBILE")
#undef MOBILE
#endif
#if !MCRO_ALLOW_CONSOLE
#pragma push_macro("CONSOLE")
#undef CONSOLE
#endif
#if !MCRO_ALLOW_DEFAULTS
#pragma push_macro("DEFAULTS")
#undef DEFAULTS
#endif

#if DO_CHECK
#define NUIR_DO_CHECK 1

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

#endif

#if PLATFORM_WINDOWS && MCRO_ALLOW_WINDOWS_TYPES
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
#endif

// A combination of compiler specific macros may be present, which is super rare probably. Third-party libraries might
// not tolerate that well. Sanitizing such macros here:

// prefer MSVC compiler macros over GCC if they're both present for some godforsaken reason
#if defined(_MSC_VER) && defined(__GNUC__)

#if _MSC_VER > 0
#pragma message ("Non-Unreal include region: _MSC_VER - __GNUC__ macro collision detected, temporarily undefining __GNUC__ (GCC: " PREPROCESSOR_TO_STRING(__GNUC__) ", MSVC: " PREPROCESSOR_TO_STRING(_MSC_VER) ")")
#define NUIR_MSVC_GNUC_AVOIDANCE __GNUC__
#pragma push_macro("__GNUC__")
#undef __GNUC__
#endif

#endif

THIRD_PARTY_INCLUDES_START
// PRAGMA_PUSH_PLATFORM_DEFAULT_PACKING
