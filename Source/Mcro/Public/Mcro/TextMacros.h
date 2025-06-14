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
	struct FDefer_AsLocalizable_Advanced : TDeferFunctionArguments<&AsLocalizable_Advanced>
	{
		FORCEINLINE FText operator % (const TCHAR* literal)
		{
			return (*this)(literal);
		}
	};

	struct FInvTextFakeLiteralTag
	{
		FORCEINLINE FText operator % (const TCHAR* str) const
		{
			return FText::AsCultureInvariant(str);
		}
	};
	
	struct FStringViewFakeLiteralTag
	{
		template <size_t N>
		consteval FStringView operator % (const TCHAR(& str)[N]) const
		{
			return FStringView(str, N-1);
		}
	};
	
	struct FStringFakeLiteralTag
	{
		template <size_t N>
		FString operator % (const TCHAR(& str)[N]) const
		{
			return FString::ConstructFromPtrSize(str, N-1);
		}
	};
	
	struct FNameFakeLiteralTag
	{
		template <size_t N>
		FName operator % (const TCHAR(& str)[N]) const
		{
			return FName(N-1, str);
		}
	};
	
	struct FStdStringLiteralTag
	{
		template <size_t N>
		consteval std::basic_string_view<TCHAR> operator % (const TCHAR(& str)[N]) const
		{
			return std::basic_string_view<TCHAR>(str, N-1);
		}
	};

	template <typename... Args>
	struct TStringPrintfLiteral
	{
		using Arguments = TTuple<Args...>;
		TStringPrintfLiteral(Args&&... args) : Storage(FWD(args)...) {}

	private:
		template <size_t CharN, size_t... Sequence>
		FString Invoke_Impl(const TCHAR(& format)[CharN], std::index_sequence<Sequence...>&&)
		{
			return FString::Printf(
				format,
				FWD(Storage.template Get<Sequence>())...
			);
		}
		
	public:
		template <size_t CharN>
		FString Invoke(const TCHAR(& format)[CharN])
		{
			return Invoke_Impl(format, std::make_index_sequence<sizeof...(Args)>());
		}

		template <size_t N>
		friend FString operator % (const TCHAR(& format)[N], TStringPrintfLiteral&& tag)
		{
			return tag.Invoke(format);
		}

		template <size_t N>
		friend FString operator % (TStringPrintfLiteral&& tag, const TCHAR(& format)[N])
		{
			return tag.Invoke(format);
		}

		Arguments Storage;
	};

	template <typename... Args>
	TStringPrintfLiteral<Args...> MakePrintfLiteral(Args&&... args)
	{
		return TStringPrintfLiteral<Args...>(FWD(args)...);
	}
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

/**
 *	@brief
 *	A convenience alternative to Unreal's own `INVTEXT` macro but this one doesn't require parenthesis around the text
 *	literal
 *	
 *	This operation allocates FText in runtime and an empty tag struct
 */
#define INVTEXT_ \
	Mcro::Text::Macros::FInvTextFakeLiteralTag() % TEXT_

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

/**
 *	@brief
 *	Trailing fake text literal which shortens down the `FString::Printf(TEXT("..."), ...);` expression.
 *
 *	Usage:
 *	@code
 *	int count, FString type;
 *	TEXT_"My format literal with %d and %s" _PRINTF(count, type);
 *	//                                     ^ this space is important
 *	@endcode
 *
 *	This operation runs `FString::Printf` in runtime and allocates a struct which stores the format arguments
 *	(preserving CV-ref qualifiers).
 */
#define _PRINTF(...) % Mcro::Text::Macros::MakePrintfLiteral(__VA_ARGS__)

/**
 *	@brief
 *	Leading fake text literal which shortens down the `FString::Printf(TEXT("..."), ...);` expression.
 *
 *	Usage:
 *	@code
 *	int count, FString type;
 *	PRINTF_(count, type) "My format literal with %d and %s";
 *	//                  ^ this space is optional
 *	@endcode
 *
 *	This operation runs `FString::Printf` in runtime and allocates a struct which stores the format arguments
 *	(preserving CV-ref qualifiers).
 */
#define PRINTF_(...) Mcro::Text::Macros::MakePrintfLiteral(__VA_ARGS__) % TEXT_