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
 *	@file
 *	@brief
 *	Use leading `FMT_` or trailing `_FMT` fake text literals to create modern formatted strings with a better API.
 *
 *	The major difference from `PRINTF_` or `FString::Printf(...)` is that `FMT` macros can take user defined string
 *	conversions into account, so more types can be used directly as arguments. 
 */

#pragma once

#include <string>

#include "CoreMinimal.h"
#include "Mcro/Text.h"
#include "Mcro/TextMacros.h"
#include "Mcro/Enums.h"
#include "Mcro/Macros.h"

template <size_t N>
FString operator % (FStringFormatOrderedArguments&& args, const TCHAR(& format)[N])
{
	return FString::Format(format, args);
}

template <size_t N>
FString operator % (const TCHAR(& format)[N], FStringFormatOrderedArguments&& args)
{
	return FString::Format(format, args);
}

template <size_t N>
FString operator % (FStringFormatNamedArguments&& args, const TCHAR(& format)[N])
{
	return FString::Format(format, args);
}

template <size_t N>
FString operator % (const TCHAR(& format)[N], FStringFormatNamedArguments&& args)
{
	return FString::Format(format, args);
}

#define MCRO_FMT_ORDERED(...) Mcro::Text::OrderedArguments(__VA_ARGS__)
#define MCRO_FMT_NAMED_ARG_INVOKE(pair) MCRO_FMT_NAMED_ARG ## pair
#define MCRO_FMT_NAMED_ARG(key, value) MakeTuple(FString(TEXT(#key)), value)
#define MCRO_FMT_NAMED(...) Mcro::Text::NamedArguments(FOR_EACH_VA_ARGS(MCRO_FMT_NAMED_ARG_INVOKE, __VA_ARGS__));

#define MCRO_FMT_ARGS(first, ...) \
	PREPROCESSOR_IF(HAS_PARENTHESIS(first), \
		MCRO_FMT_NAMED(first, __VA_ARGS__), \
		MCRO_FMT_ORDERED(first, __VA_ARGS__) \
	)

/**
 *	@brief
 *	Leading fake text literal which makes using `FString::Format(...);` much more comfortable.
 *
 *	`FMT` macros allow more types to be used directly in the format arguments expression because `Mcro::Text` has
 *	couple of conversion utilities. If the first argument of `FMT_` is a `("key", value)` pair enclosed in parenthesis,
 *	then named format arguments are assumed. Ordered format arguments are assumed otherwise. The two modes cannot be
 *	mixed.
 *
 *	Usage:
 *	@code
 *	EPixelFormat format = PF_Unknown; int32 num = 42;
 *	
 *	FString ordered = FMT_(format, num) "Hi {0}, your number is {1}";
 *	// -> "Hi PF_Unknown, your number is 42"
 *	
 *	FString named = FMT_((Type, format), (Count, num)) "Hi {Type}, your number is {Count}";
 *	// -> "Hi PF_Unknown, your number is 42"
 *	@endcode
 *
 *	@remarks
 *	To add more supported types overload the `%` operator between your preferred type and `FStringFormatArgumentTag`
 *	and return a value which is implicitly convertible to `FStringFormatArg` in the `Mcro::Text` namespace. For example
 *	check `Enums.h` to see how that's done with enums. For your own types you can also implement a `ToString()` member
 *	method to get automatic support.
 */
#define FMT_(...) MCRO_FMT_ARGS(__VA_ARGS__) % TEXT_

/**
 *	@brief
 *	Trailing fake text literal which makes using `FString::Format(...);` much more comfortable.
 *
 *	`FMT` macros allow more types to be used directly in the format arguments expression because `Mcro::Text` has
 *	couple of conversion utilities. If the first argument of `_FMT` is a `("key", value)` pair enclosed in parenthesis,
 *	then named format arguments are assumed. Ordered format arguments are assumed otherwise. The two modes cannot be
 *	mixed.
 *
 *	Usage:
 *	@code
 *	EPixelFormat format = PF_Unknown; int32 num = 42;
 *	
 *	FString ordered = TEXT_"Hi {0}, your number is {1}" _FMT(format, num);
 *	// -> "Hi PF_Unknown, your number is 42"           ^ this space is important
 *
 *	// Named arguments look better with multiple lines on this version
 *	FString named = TEXT_"Hi {Type}, your number is {Count}" _FMT(
 *		(Type, format),                                  // ^ this space is important
 *		(Count, num)
 *	);
 *	// -> "Hi PF_Unknown, your number is 42"
 *	@endcode
 *	
 *	@remarks
 *	To add more supported types overload the `%` operator between your preferred type and `FStringFormatArgumentTag`
 *	and return a value which is implicitly convertible to `FStringFormatArg` in the `Mcro::Text` namespace. For example
 *	check `Enums.h` to see how that's done with enums. For your own types you can also implement a `ToString()` member
 *	method to get automatic support.
 */
#define _FMT(...) % MCRO_FMT_ARGS(__VA_ARGS__)