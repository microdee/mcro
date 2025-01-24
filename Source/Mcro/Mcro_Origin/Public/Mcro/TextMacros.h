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
 *	literal, relying on string literal concatenation rules of C++
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
	
	FORCEINLINE FText AsCultureInvariant(const TCHAR* String)
	{
		return FText::AsCultureInvariant(String);
	}
	using FDefer_AsCultureInvariant = TDeferFunctionArguments<&AsCultureInvariant>;
}

template <auto FunctionPtr>
auto operator / (Mcro::FunctionTraits::TDeferFunctionArguments<FunctionPtr>&& deferrer, const TCHAR* literal)
{
	return deferrer(literal);
}

/**
 *	@brief
 *	A convenience alternative to Unreal's own `LOCTEXT` macro but this one doesn't require parenthesis around the text literal
 */
#define LOCTEXT_(key) \
	Mcro::Text::Macros::FDefer_AsLocalizable_Advanced(TEXT(LOCTEXT_NAMESPACE), TEXT(key)) / TEXT_

/**
 *	@brief
 *	A convenience alternative to Unreal's own `NSLOCTEXT` macro but this one doesn't require parenthesis around the text literal
 */
#define NSLOCTEXT_(ns, key) \
	Mcro::Text::Macros::FDefer_AsLocalizable_Advanced(TEXT(ns), TEXT(key)) / TEXT_

/**
 *	@brief
 *	A convenience alternative to Unreal's own `INVTEXT` macro but this one doesn't require parenthesis around the text literal
 */
#define INVTEXT_ \
	Mcro::Text::Macros::FDefer_AsCultureInvariant() / TEXT_