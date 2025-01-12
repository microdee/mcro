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
#include "McroWindows/Error/WindowsError.h"

/**
 *	@file
 *	This header provides convenience macros for dealing with API returning `HRESULT` elegantly
 */

#if UE_BUILD_SHIPPING
#define HR_WITH_STACKTRACE
#else
#define HR_WITH_STACKTRACE ->WithCppStackTrace()
#endif

#define MCRO_TRY_WITH_IMPL(tempVar, expression, noErrorInfo)                                            \
	HRESULT tempVar = (expression);                                                                     \
	if (UNLIKELY(tempVar != S_OK))                                                                      \
		return Mcro::Error::IError::Make(new Mcro::Windows::Error::FHresultError(tempVar, noErrorInfo)) \
			->WithLocation()                                                                            \
			->AsRecoverable                                                                             \
			->WithCodeContext(PREPROCESSOR_TO_TEXT(expression))                                        //

/** Use this macro in a function which returns an `Mcro::Error::TMaybe`. */
#define HR_TRY_WITH(expression, noErrorInfo) \
	MCRO_TRY_WITH_IMPL(PREPROCESSOR_JOIN(tempHr, __LINE__), expression, noErrorInfo)

/**
 *	Use this macro in a function which returns an `Mcro::Error::TMaybe`. On non-shipping builds stacktrace is captured
 *	by default.
 */
#define HR_TRY(expression)         \
	HR_TRY_WITH(expression, false) \
		HR_WITH_STACKTRACE         \
		->AsFatal()                \
		->BreakDebugger()         //

/** Use this macro in a function which returns an `Mcro::Error::TMaybe`. This version doesn't capture a stacktrace. */
#define HR_TRY_FAST(expression) HR_TRY_WITH(expression, false)

/**
 *	Use this macro in a function which returns an `Mcro::Error::TMaybe`. This version doesn't capture a stacktrace and
 *	it won't calculate human readable messages from the HRESULT the error code.
 */
#define HR_TRY_RAW(expression) HR_TRY_WITH(expression, true)
