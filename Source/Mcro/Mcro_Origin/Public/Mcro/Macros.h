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

#pragma once

#include "CoreMinimal.h"
#include "HAL/PreprocessorHelpers.h"

#define PREPROCESSOR_TO_TEXT(x) TEXT(PREPROCESSOR_TO_STRING(x))

#if UE_BUILD_SHIPPING

/**
 *	UE_DEBUG_BREAK is disabled in all non-editor builds, not only in shipping. MCRO_DEBUG_BREAK however is only
 *	disabled in shipping,
 */
#define MCRO_DEBUG_BREAK() ((void)0)

#else

/**
 *	UE_DEBUG_BREAK is disabled in all non-editor builds, not only in shipping. MCRO_DEBUG_BREAK however is only
 *	disabled in shipping,
 */
#define MCRO_DEBUG_BREAK() ((void)(FPlatformMisc::IsDebuggerPresent() && ([] () { UE_DEBUG_BREAK_IMPL(); } (), 1)))

#endif
