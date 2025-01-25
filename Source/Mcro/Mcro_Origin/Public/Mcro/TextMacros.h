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
 *	Use leading `TEXT_` without parenthesis for Unreal compatible text literals.
 */

#pragma once

#include <string>

#include "CoreMinimal.h"
#include "Mcro/FunctionTraits.h"

#define UTF8TEXT_PASTE_ u8""
#define UTF16TEXT_PASTE_ u""

#if PLATFORM_WIDECHAR_IS_CHAR16
	#define WIDETEXT_PASTE_ UTF16TEXT_PASTE_
#else
	#define WIDETEXT_PASTE_ L""
#endif

#define UTF8TEXT_ UTF8TEXT_PASTE_ // TODO: UE::Core::Private::ToUTF8Literal with operator?
#define UTF16TEXT_ UTF16TEXT_PASTE_
#define WIDETEXT_ WIDETEXT_PASTE_

#if PLATFORM_TCHAR_IS_UTF8CHAR
	#define TEXT_PASTE_ UTF8TEXT_
#else
	#define TEXT_PASTE_ WIDETEXT_
#endif

/**
 *	@brief
 *	A convenience alternative to Unreal's own `TEXT` macro but this one doesn't require parenthesis around the text
 *	literal, relying on string literal concatenation rules of C++.
 *
 *	This operation is resolved entirely in compile time
 */
#define TEXT_ TEXT_PASTE_

/** @brief This namespace is used by MCRO text literal macros, don't use it directly! */
namespace Mcro::Text::Macros
{
	using namespace Mcro::FunctionTraits;
	
	FORCEINLINE FText AsLocalizable_Advanced(const FTextKey& Namespace, const FTextKey& Key, const TCHAR* String)
	{
		return FText::AsLocalizable_Advanced(Namespace, Key, String);
	}
	using FDefer_AsLocalizable_Advanced = TDeferFunctionArguments<&AsLocalizable_Advanced>;

	struct FInvTextFakeLiteralTag {};
	struct FStringViewFakeLiteralTag {};
	struct FStringFakeLiteralTag {};
	struct FNameFakeLiteralTag {};
	struct FStdStringLiteralTag {};
}

template <auto FunctionPtr>
auto operator % (Mcro::FunctionTraits::TDeferFunctionArguments<FunctionPtr>&& deferrer, const TCHAR* literal)
{
	return deferrer(literal);
}

/**
 *	@brief
 *	A convenience alternative to Unreal's own `LOCTEXT` macro but this one doesn't require parenthesis around the text
 *	literal
 *	
 *	This operation allocates an argument deferring struct and FText in runtime
 */
#define LOCTEXT_(key) \
	Mcro::Text::Macros::FDefer_AsLocalizable_Advanced(TEXT(LOCTEXT_NAMESPACE), TEXT(key)) % TEXT_

/**
 *	@brief
 *	A convenience alternative to Unreal's own `NSLOCTEXT` macro but this one doesn't require parenthesis around the text
 *	literal
 *	
 *	This operation allocates an argument deferring struct and FText in runtime
 */
#define NSLOCTEXT_(ns, key) \
	Mcro::Text::Macros::FDefer_AsLocalizable_Advanced(TEXT(ns), TEXT(key)) % TEXT_

FORCEINLINE FText operator % (Mcro::Text::Macros::FInvTextFakeLiteralTag&&, const TCHAR* str)
{
	return FText::AsCultureInvariant(str);
}

/**
 *	@brief
 *	A convenience alternative to Unreal's own `INVTEXT` macro but this one doesn't require parenthesis around the text
 *	literal
 *	
 *	This operation allocates FText in runtime and an empty tag struct
 */
#define INVTEXT_ \
	Mcro::Text::Macros::FInvTextFakeLiteralTag() % TEXT_

template <size_t N>
consteval FStringView operator % (Mcro::Text::Macros::FStringViewFakeLiteralTag&&, const TCHAR(& str)[N])
{
	return FStringView(str, N);
}

template <size_t N>
FString operator % (Mcro::Text::Macros::FStringFakeLiteralTag&&, const TCHAR(& str)[N])
{
	return FString::ConstructWithSlack(str, N);
}

template <size_t N>
FName operator % (Mcro::Text::Macros::FNameFakeLiteralTag&&, const TCHAR(& str)[N])
{
	return FName(N, str);
}

#if PLATFORM_TCHAR_IS_UTF8CHAR
template <size_t N>
consteval std::string_view operator % (Mcro::Text::Macros::FStdStringLiteralTag&&, const TCHAR(& str)[N])
{
	return std::string_view(str, N);
}
#else
template <size_t N>
consteval std::wstring_view operator % (Mcro::Text::Macros::FStdStringLiteralTag&&, const TCHAR(& str)[N])
{
	return std::wstring_view(str, N);
}
#endif

/**
 *	@brief
 *	A convenience alternative to Unreal's own `TEXTVIEW` macro but this one doesn't require parenthesis around the text
 *	literal.
 *
 *	This operation creates an FStringView in consteval time. This is not a custom string literal because they're not
 *	available for concatenated groups of string literals of mixed encodings.
 */
#define TEXTVIEW_ Mcro::Text::Macros::FStringViewFakeLiteralTag() % TEXT_

/**
 *	@brief
 *	A convenience alternative to Unreal's own `TEXTVIEW` macro but this one doesn't require parenthesis around the text
 *	literal and it returns an STL string view.
 *
 *	This operation creates a `std::[w]string_view` in consteval time. This is not a custom string literal because
 *	they're not available for concatenated groups of string literals of mixed encodings.
 */
#define STDVIEW_ Mcro::Text::Macros::FStdStringLiteralTag() % TEXT_

/**
 *	@brief
 *	A convenience macro to allocate an FString directly. 
 *
 *	This operation allocates FString in runtime and an empty tag struct. This is not a custom string literal because
 *	they're not available for concatenated groups of string literals of mixed encodings.
 */
#define STRING_ Mcro::Text::Macros::FStringFakeLiteralTag() % TEXT_

/**
 *	@brief
 *	A convenience macro to allocate an FName directly. 
 *
 *	This operation allocates FName in runtime and an empty tag struct. This is not a custom string literal because
 *	they're not available for concatenated groups of string literals of mixed encodings.
 */
#define NAME_ Mcro::Text::Macros::FNameFakeLiteralTag() % TEXT_