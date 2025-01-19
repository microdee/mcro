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
 * Use this header and `Start.h` in tandem to include third-party library headers which may not tolerate Unreal's common
 * vocabulary macros or its stricter warning policies.
 */

THIRD_PARTY_INCLUDES_END
PRAGMA_POP_PLATFORM_DEFAULT_PACKING

// restore temporary macro undefs

#pragma pop_macro("TEXT")
#pragma pop_macro("TRUE")
#pragma pop_macro("FALSE")
#pragma pop_macro("MAX_uint8")
#pragma pop_macro("MAX_uint16")
#pragma pop_macro("MAX_uint32")
#pragma pop_macro("MAX_int32")
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
#pragma pop_macro("DEFAULTS")

#pragma warning( pop )

#ifdef NUIR_DO_CHECK
#undef NUIR_DO_CHECK

#pragma pop_macro("DO_CHECK")
#pragma pop_macro("checkCode")
#pragma pop_macro("check")
#pragma pop_macro("checkf")
#pragma pop_macro("verify")
#pragma pop_macro("verifyf")
#pragma pop_macro("unimplemented")
#pragma pop_macro("ensure")

#endif

#ifdef NUIR_MSVC_GNUC_AVOIDANCE
#pragma pop_macro("__GNUC__")
#endif

#if PLATFORM_WINDOWS
#include "Windows/HideWindowsPlatformAtomics.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#undef NON_UNREAL_INCLUDE_REGION
