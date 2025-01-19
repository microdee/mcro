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
 * Use this header and `End.h` in tandem to include third-party library headers which may not tolerate Unreal's common
 * vocabulary macros or its stricter warning policies.
 */

#include "Mcro/Macros.h"

#ifdef NON_UNREAL_INCLUDE_REGION
#error Third-party library or non-unreal include regions cannot be nested, one region is already opened before
#endif
#define NON_UNREAL_INCLUDE_REGION 1

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
#endif

#pragma warning( push )
#pragma warning( disable : 4005 5205 4265 4268 4946 4103 )

// temporarily undefine simplistic Unreal macros third-party libraries might also use

#pragma push_macro("TEXT")
#pragma push_macro("TRUE")
#pragma push_macro("FALSE")
#pragma push_macro("MAX_uint8")
#pragma push_macro("MAX_uint16")
#pragma push_macro("MAX_uint32")
#pragma push_macro("MAX_int32")
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
#pragma push_macro("DEFAULTS")

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
#undef DEFAULTS

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
PRAGMA_PUSH_PLATFORM_DEFAULT_PACKING
