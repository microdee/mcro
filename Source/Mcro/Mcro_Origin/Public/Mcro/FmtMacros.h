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
	FString result = FString::Format(format, args);
	result.TrimToNullTerminator();
	return result;
}

template <size_t N>
FString operator % (const TCHAR(& format)[N], FStringFormatOrderedArguments&& args)
{
	FString result = FString::Format(format, args);
	result.TrimToNullTerminator();
	return result;
}

template <size_t N>
FString operator % (FStringFormatNamedArguments&& args, const TCHAR(& format)[N])
{
	FString result = FString::Format(format, args);
	result.TrimToNullTerminator();
	return result;
}

template <size_t N>
FString operator % (const TCHAR(& format)[N], FStringFormatNamedArguments&& args)
{
	FString result = FString::Format(format, args);
	result.TrimToNullTerminator();
	return result;
}

#define MCRO_FMT_NAMED_ARG_TRANSFORM(s, data, elem) BOOST_PP_EXPAND(MCRO_FMT_NAMED_ARG elem)
#define MCRO_FMT_NAMED_ARG(key, value) MakeTuple(FString(TEXT(#key)), value)

#define MCRO_FMT_NAMED(seq)                     \
	BOOST_PP_IIF(BOOST_PP_IS_BEGIN_PARENS(seq), \
		MCRO_FMT_NAMED_0,                       \
		BOOST_PP_IDENTITY_N(, 1)                \
	)(seq)                                     //

#define MCRO_FMT_NAMED_0(seq)                     \
	Mcro::Text::NamedArguments(                   \
		BOOST_PP_SEQ_ENUM(                        \
			BOOST_PP_SEQ_TRANSFORM(               \
				MCRO_FMT_NAMED_ARG_TRANSFORM, ,   \
				BOOST_PP_VARIADIC_SEQ_TO_SEQ(seq) \
			)                                     \
		)                                         \
	)                                            //

#define MCRO_FMT_ORDERED(...) Mcro::Text::OrderedArguments(__VA_ARGS__)

#define MCRO_FMT_ARGS(...)                                      \
	BOOST_PP_IIF(BOOST_PP_IS_BEGIN_PARENS(__VA_ARGS__),         \
		MCRO_FMT_NAMED(BOOST_PP_VARIADIC_ELEM(0, __VA_ARGS__)), \
		MCRO_FMT_ORDERED(__VA_ARGS__)                           \
	)                                                          //

/**
 *	@brief
 *	Leading fake text literal which makes using `FString::Format(...);` much more comfortable.
 *
 *	`FMT` macros allow more types to be used directly in the format arguments expression because `Mcro::Text` has
 *	a couple of conversion utilities. If the first argument of `FMT_` is a sequence of `("key", value)` pairs enclosed
 *	in parentheses, then named format arguments are assumed. Ordered format arguments are assumed otherwise. The two
 *	modes cannot be mixed.
 *
 *	Usage:
 *	@code
 *	EPixelFormat format = PF_Unknown; int32 num = 42;
 *	
 *	FString ordered = FMT_(format, num) "Hi {0}, your number is {1}";
 *	// -> "Hi PF_Unknown, your number is 42"
 *
 *	//                                 | Notice the lack of comma here
 *	//                                 V  
 *	FString named = FMT_((Type, format) (Count, num)) "Hi {Type}, your number is {Count}";
 *	// -> "Hi PF_Unknown, your number is 42"
 *	@endcode
 *
 *	The following argument types are supported out-of-box:
 *	- Originally supported by `FStringFormatArg` and what's implicitly convertable to
 *	  - `FString, FStringView, int32, uint32, int64, uint64, float, double`
 *	  - `ANSICHAR, WIDECHAR, UCS2CHAR, UTF8CHAR` pointer strings
 *	- Anything which has a `ToString()` member method which produces one of the above type
 *	  - Including but not exclusively `FText` and `FName` for example
 *	- STL strings and views of any encoding
 *	- Enums where their entries are serialized as human-readable names
 *
 *	@remarks
 *	To add more supported types overload the `%` operator between your preferred type and `FStringFormatTag`
 *	and return a value which is implicitly convertible to `FStringFormatArg` in the `Mcro::Text` namespace. For example
 *	check `Enums.h` to see how that's done with enums. For your own types you can also implement a `ToString()` member
 *	method to get automatic support.
 *
 *	@warning
 *	The **named arguments** overload (`FMT_((A, a) (B, b) (C, c))`) must not have comma `,` between the individual
 *	pairs. This is because named argument pairs are passed into this macro as a "sequence" instead of variadic arguments.
 *	Ordered format arguments however should be passed in as regular variadic arguments separated by comma `,` as
 *	nature intended.
 *
 *	@note
 *	`FMT` macros are the only things in MCRO where excessive preprocessing is used
 */
#define FMT_(...) MCRO_FMT_ARGS(__VA_ARGS__) % TEXT_

/**
 *	@brief
 *	Trailing fake text literal which makes using `FString::Format(...);` much more comfortable.
 *
 *	`FMT` macros allow more types to be used directly in the format arguments expression because `Mcro::Text` has
 *	a couple of conversion utilities. If the first argument of `_FMT` is a sequence of `("key", value)` pairs enclosed
 *	in parentheses, then named format arguments are assumed. Ordered format arguments are assumed otherwise. The two
 *	modes cannot be mixed.
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
 *		(Type, format) // <- Notice the lack of comma here  ^ this space is important
 *		(Count, num)
 *	);
 *	// -> "Hi PF_Unknown, your number is 42"
 *	@endcode
 *
 *	The following argument types are supported out-of-box:
 *	- Originally supported by `FStringFormatArg` and what's implicitly convertable to
 *	  - `FString, FStringView, int32, uint32, int64, uint64, float, double`
 *	  - `ANSICHAR, WIDECHAR, UCS2CHAR, UTF8CHAR` pointer strings
 *	- Anything which has a `ToString()` member method which produces one of the above type
 *	  - Including but not exclusively `FText` and `FName` for example
 *	- STL strings and views of any encoding
 *	- Enums where their entries are serialized as human-readable names
 *	
 *	@remarks
 *	To add more supported types overload the `%` operator between your preferred type and `FStringFormatTag`
 *	and return a value which is implicitly convertible to `FStringFormatArg` in the `Mcro::Text` namespace. For example
 *	check `Enums.h` to see how that's done with enums. For your own types you can also implement a `ToString()` member
 *	method to get automatic support.
 *
 *	@warning
 *	The **named arguments** overload (`_FMT((A, a) (B, b) (C, c))`) must not have comma `,` between the individual
 *	pairs. This is because named argument pairs are passed into this macro as a "sequence" instead of variadic arguments.
 *	Ordered format arguments however should be passed in as regular variadic arguments separated by comma `,` as
 *	nature intended.
 *
 *	@note
 *	`FMT` macros are the only things in MCRO where excessive preprocessing is used
 */
#define _FMT(...) % MCRO_FMT_ARGS(__VA_ARGS__)