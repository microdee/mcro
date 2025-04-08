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
 * Use this header and `Start.h` in tandem to include third-party library headers which may not tolerate Unreal's common
 * vocabulary macros or its stricter warning policies.
 */

THIRD_PARTY_INCLUDES_END
// PRAGMA_POP_PLATFORM_DEFAULT_PACKING

#if PLATFORM_WINDOWS && MCRO_ALLOW_WINDOWS_TYPES
#include "Windows/HideWindowsPlatformAtomics.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#undef MCRO_ALLOW_WINDOWS_TYPES

// restore temporary macro undefs


#if !MCRO_ALLOW_TEXT
#pragma pop_macro("TEXT")
#endif
#if !MCRO_ALLOW_TRUE
#pragma pop_macro("TRUE")
#endif
#if !MCRO_ALLOW_FALSE
#pragma pop_macro("FALSE")
#endif
#if !MCRO_ALLOW_MAX_uint8
#pragma pop_macro("MAX_uint8")
#endif
#if !MCRO_ALLOW_MAX_uint16
#pragma pop_macro("MAX_uint16")
#endif
#if !MCRO_ALLOW_MAX_uint32
#pragma pop_macro("MAX_uint32")
#endif
#if !MCRO_ALLOW_MAX_int32
#pragma pop_macro("MAX_int32")
#endif
#if !MCRO_ALLOW_CONSTEXPR
#pragma pop_macro("CONSTEXPR")
#endif
#if !MCRO_ALLOW_PI
#pragma pop_macro("PI")
#endif
#if !MCRO_ALLOW_dynamic_cast
#pragma pop_macro("dynamic_cast")
#endif
#if !MCRO_ALLOW_VARARGS
#pragma pop_macro("VARARGS")
#endif
#if !MCRO_ALLOW_CDECL
#pragma pop_macro("CDECL")
#endif
#if !MCRO_ALLOW_STDCALL
#pragma pop_macro("STDCALL")
#endif
#if !MCRO_ALLOW_FORCEINLINE
#pragma pop_macro("FORCEINLINE")
#endif
#if !MCRO_ALLOW_FORCENOINLINE
#pragma pop_macro("FORCENOINLINE")
#endif
#if !MCRO_ALLOW_ABSTRACT
#pragma pop_macro("ABSTRACT")
#endif
#if !MCRO_ALLOW_LINE_TERMINATOR
#pragma pop_macro("LINE_TERMINATOR")
#endif
#if !MCRO_ALLOW_LINE_TERMINATOR_ANSI
#pragma pop_macro("LINE_TERMINATOR_ANSI")
#endif
#if !MCRO_ALLOW_DLLEXPORT
#pragma pop_macro("DLLEXPORT")
#endif
#if !MCRO_ALLOW_DLLIMPORT
#pragma pop_macro("DLLIMPORT")
#endif
#if !MCRO_ALLOW_LIKELY
#pragma pop_macro("LIKELY")
#endif
#if !MCRO_ALLOW_UNLIKELY
#pragma pop_macro("UNLIKELY")
#endif
#if !MCRO_ALLOW_RESTRICT
#pragma pop_macro("RESTRICT")
#endif
#if !MCRO_ALLOW_MOBILE
#pragma pop_macro("MOBILE")
#endif
#if !MCRO_ALLOW_CONSOLE
#pragma pop_macro("CONSOLE")
#endif
#if !MCRO_ALLOW_DEFAULTS
#pragma pop_macro("DEFAULTS")
#endif

#undef MCRO_ALLOW_TEXT
#undef MCRO_ALLOW_TRUE
#undef MCRO_ALLOW_FALSE
#undef MCRO_ALLOW_MAX_uint8
#undef MCRO_ALLOW_MAX_uint16
#undef MCRO_ALLOW_MAX_uint32
#undef MCRO_ALLOW_MAX_int32
#undef MCRO_ALLOW_CONSTEXPR
#undef MCRO_ALLOW_PI
#undef MCRO_ALLOW_dynamic_cast
#undef MCRO_ALLOW_VARARGS
#undef MCRO_ALLOW_CDECL
#undef MCRO_ALLOW_STDCALL
#undef MCRO_ALLOW_FORCEINLINE
#undef MCRO_ALLOW_FORCENOINLINE
#undef MCRO_ALLOW_ABSTRACT
#undef MCRO_ALLOW_LINE_TERMINATOR
#undef MCRO_ALLOW_LINE_TERMINATOR_ANSI
#undef MCRO_ALLOW_DLLEXPORT
#undef MCRO_ALLOW_DLLIMPORT
#undef MCRO_ALLOW_LIKELY
#undef MCRO_ALLOW_UNLIKELY
#undef MCRO_ALLOW_RESTRICT
#undef MCRO_ALLOW_MOBILE
#undef MCRO_ALLOW_CONSOLE
#undef MCRO_ALLOW_DEFAULTS

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

#undef MCRO_HIDE_TEXT
#undef MCRO_ALLOW_WINDOWS_TYPES

#undef NON_UNREAL_INCLUDE_REGION
