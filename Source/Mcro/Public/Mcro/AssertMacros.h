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

/**
 *	@file
 *	By default `ASSERT_QUIT|CRASH` macros are also active in Shipping builds. This was a personal preference and
 *	that's why you can turn it off if you define `MCRO_ASSERT_IGNORE_SHIPPING`.
 *	
 *	@remarks
 *	Yes I know that "shipping builds must have all debug info and checks removed" but I have yet to see one project in
 *	my career where Shipping builds didn't have weird issues or crashes not present in other debug builds, and debugging
 *	them removed many years from my expected life-span.
 */

#include "CoreMinimal.h"
#include "Mcro/Error.h"
#include "Mcro/Macros.h"
#include "Logging/LogMacros.h"

/** @brief Do not use this namespace directly use `ASSERT_QUIT|CRASH` macros instead */
namespace Mcro::AssertMacros
{
	using namespace Mcro::Error;

	namespace Detail
	{
		MCRO_API void SubmitError(
			EErrorSeverity severity,
			FString const& codeContext,
			bool async, bool important,
			TUniqueFunction<void(IErrorRef const&)>&& extraSetup,
			std::source_location const& location = std::source_location::current()
		);
		MCRO_API bool IsRunningPIE();
		MCRO_API void StopPie();
	}
}

#define MCRO_ASSERT_SUBMIT_ERROR(condition, severity, async, important, ...) \
	Mcro::AssertMacros::Detail::SubmitError(                                 \
		Mcro::Error::EErrorSeverity::severity,                               \
		PREPROCESSOR_TO_TEXT(condition),                                     \
		async, important,                                                    \
		[&](Mcro::Error::IErrorRef const& error) { (error __VA_ARGS__); }    \
	);                                                                      //

#define MCRO_ASSERT_CRASH_METHOD                                   \
	UE_LOG(LogTemp, Fatal,                                         \
		TEXT_"Program cannot continue for the reasons above. (at " \
		PREPROCESSOR_TO_TEXT(__FILE__:__LINE__) TEXT_")"           \
	)                                                             //

#define MCRO_CRASH_BODY(condition, ...)    \
	MCRO_ASSERT_SUBMIT_ERROR(              \
		condition, Crashing, false, false, \
		__VA_ARGS__                        \
	)                                      \
	MCRO_ASSERT_CRASH_METHOD;             //

#define MCRO_QUIT_BODY(condition, returnOnFailure, ...) \
	MCRO_ASSERT_SUBMIT_ERROR(                           \
		condition, Fatal,                               \
		Mcro::AssertMacros::Detail::IsRunningPIE(),     \
		Mcro::AssertMacros::Detail::IsRunningPIE(),     \
		__VA_ARGS__                                     \
	)                                                   \
	if (Mcro::AssertMacros::Detail::IsRunningPIE())     \
	{                                                   \
		Mcro::AssertMacros::Detail::StopPie();          \
		return returnOnFailure;                         \
	}                                                   \
	else { MCRO_ASSERT_CRASH_METHOD }                  //

#define MCRO_ASSERT_CRASH_COMMON(condition, ...) \
	if (UNLIKELY(!(condition)))                  \
	{                                            \
		MCRO_CRASH_BODY(condition, __VA_ARGS__)  \
	}                                           //

#define MCRO_ASSERT_QUIT_COMMON(condition, returnOnFailure, ...) \
	if (UNLIKELY(!(condition)))                                  \
	{                                                            \
		MCRO_QUIT_BODY(condition, returnOnFailure, __VA_ARGS__)  \
	}                                                           //

#if WITH_EDITOR

/**
 *	@brief 
 *	Use this instead of `check` macro if the checked expression shouldn't be ignored in shipping builds. This version
 *	will also crash the entire editor even during a PIE session. Use this only if there's absolutely no way to recover
 *	from an invalid state, like a situation when early return is not possible / doesn't make sense (e.g.: return a
 *	checked pointer by reference).
 *	
 *	This macro will attempt to display the error before crashing via the IError type. You can also append extra
 *	information to it, or even override preset information like so:
 *	@code
 *	ASSERT_CRASH(arrayNum > 0,
 *		->WithDetails(TEXT_"Did you forget to specify data?")
 *		->WithAppendix(TEXT_"Data", data.ToString())
 *	)
 *	@endcode
 */
#define ASSERT_CRASH(condition, ...) MCRO_ASSERT_CRASH_COMMON(condition, __VA_ARGS__)

/** @brief This is equivalent to ASSERT_CRASH but if a code path reaches this macro it will always crash */
#define FORCE_CRASH(...) MCRO_CRASH_BODY(Invalid code path, __VA_ARGS__)

/**
 *	@brief
 *	Use this instead of `check` macro if the checked expression shouldn't be ignored in shipping builds. This version
 *	will not crash the editor but will display an error, stops the PIE, and should return the calling function.
 *
 *	A return value should be provided on failure, or leave it empty to return void. so either
 *	@code
 *	ASSERT_QUIT(SomethingImportantButNullable, ) // important to leave an extra comma for void functions
 *	// or return default
 *	ASSERT_QUIT(SomethingImportantButNullable, {})
 *	// or return whatever
 *	ASSERT_QUIT(SomethingImportantButNullable, myReturnValue)
 *	@endcode
 *	
 *	This is done in the hopes that these checks failing in PIE sessions doesn't crash the entire editor giving the
 *	developer a chance for fixing the error before restarting. However, early returns might cause
 *	undefined/vaguely-defined side effects and may lead to uninitialized resources or in worst case scenario may lead to
 *	incorrect states in resource cleanups which then might crash the editor. Always check the validity of your resources in destructors.
 *	Failing this check outside of PIE or in a Standalone session will still crash with a `check` macro.
 *	
 *	This macro will attempt to display the error before quitting via the IError type. You can also append extra
 *	information to it like so, or even override preset information like so:
 *	@code
 *	ASSERT_QUIT(arrayNum > 0, myReturnValue,
 *		->WithDetails(TEXT_"Did you forget to specify data?")
 *		->WithAppendix(TEXT_"Data", data.ToString())
 *	)
 *	@endcode
 */
#define ASSERT_QUIT(condition, returnOnFailure, ...) MCRO_ASSERT_QUIT_COMMON(condition, returnOnFailure, __VA_ARGS__)

/** @brief  This is equivalent to ASSERT_QUIT but if a code path reaches this macro it will always quit */
#define FORCE_QUIT(returnOnFailure, ...) MCRO_QUIT_BODY(Invalid code path, returnOnFailure, __VA_ARGS__)

#elif UE_BUILD_SHIPPING && defined(MCRO_ASSERT_IGNORE_SHIPPING)

#define ASSERT_CRASH(condition, ...)
#define ASSERT_QUIT(condition, returnOnFailure, ...)

#else

#define ASSERT_CRASH(condition, ...) MCRO_ASSERT_CRASH_COMMON(condition, __VA_ARGS__)
#define ASSERT_QUIT(condition, returnOnFailure, ...) MCRO_ASSERT_CRASH_COMMON(condition, __VA_ARGS__)

#endif

/** @brief Shorthand of `ASSERT_QUIT` for TMaybe monads */
#define ASSERT_QUIT_NON_ERROR(maybe, returnOnFailure, ...) ASSERT_QUIT(maybe, returnOnFailure, ->WithError(maybe.GetErrorRef()) __VA_ARGS__)

/** @brief Shorthand of `ASSERT_CRASH` for TMaybe monads */
#define ASSERT_CRASH_NON_ERROR(maybe, ...) ASSERT_CRASH(maybe, ->WithError(maybe.GetErrorRef()) __VA_ARGS__)