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

/** Use this macro in a function which returns an `Mcro::Error::TMaybe`. */
#define HR_TRY_WITH(expression, noErrorInfo, ...)                                                   \
	{                                                                                               \
		HRESULT hr = (expression);                                                                  \
		if (UNLIKELY(hr != S_OK))                                                                   \
			return MakeError(                                                                       \
				Mcro::Error::IError::Make(new Mcro::Windows::Error::FHresultError(hr, noErrorInfo)) \
					->WithLocation()                                                                \
					->AsRecoverable                                                                 \
					->WithCodeContext(PREPROCESSOR_TO_TEXT(expression))                             \
					__VA_ARGS__                                                                     \
			);                                                                                      \
	}

/**
 *	Use this macro in a function which returns an `Mcro::Error::TMaybe`. On non-shipping builds stacktrace is captured
 *	by default.
 */
#define HR_TRY(expression, ...)    \
	HR_TRY_WITH(expression, false, \
		HR_WITH_STACKTRACE         \
		->AsFatal()                \
		__VA_ARGS__                \
		->BreakDebugger()          \
	)

/** Use this macro in a function which returns an `Mcro::Error::TMaybe`. This version doesn't capture a stacktrace. */
#define HR_TRY_FAST(expression, ...) HR_TRY_WITH(expression, false, __VA_ARGS__)

/**
 *	Use this macro in a function which returns an `Mcro::Error::TMaybe`. This version doesn't capture a stacktrace and
 *	it won't calculate human readable messages from the HRESULT the error code.
 */
#define HR_TRY_RAW(expression, ...) HR_TRY_WITH(expression, true, __VA_ARGS__)
