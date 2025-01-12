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

#include <string>

#include "CoreMinimal.h"
#include "Mcro/Concepts.h"

namespace Mcro::Text
{
	using namespace Mcro::Concepts;

	using FUtf16StringView = TStringView<UTF16CHAR>;
	using FUtf32StringView = TStringView<UTF32CHAR>;

#if PLATFORM_TCHAR_IS_UTF8CHAR
	using FStdString = std::string;
	using FStdStringView = std::string_view;
#else
	using FStdString = std::wstring;
	using FStdStringView = std::wstring_view;
#endif

	template<typename T>
	concept CStringView = CSameAsDecayed<T, TStringView<typename T::ElementType>>;

	template<typename T>
	concept CStringOrView = CSameAsDecayed<T, FString> || CStringView<T>;
	
	template<typename T>
	concept CStringOrViewOrName = CStringOrView<T> || CSameAsDecayed<T, FName>;

	template <typename T>
	concept CStdStringOrViewUtf8 = CConvertibleToDecayed<T, std::string_view>;

	template <typename T>
	concept CStdStringOrViewWide = CConvertibleToDecayed<T, std::wstring_view>;

	template <typename T>
	concept CStdStringOrView = CConvertibleToDecayed<T, FStdStringView>;

	template <typename T>
	concept CStdStringOrViewInvariant = CStdStringOrViewUtf8<T> || CStdStringOrViewWide<T>;

	MCRO_API FStringView UnrealView(FStdStringView const& stdStr);
	MCRO_API FUtf8StringView UnrealViewUtf8(std::string_view const& stdStr);
	MCRO_API FUtf16StringView UnrealViewUtf16(std::wstring_view const& stdStr);
	MCRO_API FStdStringView StdView(FString const& unrealStr);
	MCRO_API FStdStringView StdView(FStringView const& unrealStr);
	MCRO_API std::string_view StdView(FUtf8StringView const& unrealStr);
	MCRO_API std::wstring_view StdView(FUtf16StringView const& unrealStr);

	MCRO_API FString UnrealCopy(FStdStringView const& stdStr);
	MCRO_API FString UnrealConvert(std::string_view const& stdStr);
	MCRO_API FString UnrealConvert(std::wstring_view const& stdStr);
	
	MCRO_API FName UnrealNameCopy(FStdStringView const& stdStr);
	MCRO_API FName UnrealNameConvert(std::string_view const& stdStr);
	MCRO_API FName UnrealNameConvert(std::wstring_view const& stdStr);

	MCRO_API FStdString StdCopy(FStringView const& unrealStr);
	MCRO_API FStdString StdCopy(FName const& unrealStr);
	MCRO_API std::string StdConvertUtf8(FStringView const& unrealStr);
	MCRO_API std::wstring StdConvertWide(FStringView const& unrealStr);
	
	MCRO_API std::string StdConvertUtf8(FStdStringView const& stdStr);
	MCRO_API std::wstring StdConvertWide(FStdStringView const& stdStr);
	
	MCRO_API std::string StdConvertUtf8(FName const& unrealName);
	MCRO_API std::wstring StdConvertWide(FName const& unrealName);

	template <CSameAs<FString>... Args>
	FString Join(const TCHAR* separator, Args... args)
	{
		return FString::Join(
			TArray<FString>{args...}.FilterByPredicate([](const FString& str) { return !str.IsEmpty(); }),
			separator
		);
	}

	/** Copy of FString::PrintfImpl but not private so it can work with strings which were not literals */
	MCRO_API FString DynamicPrintf(const TCHAR* fmt, ...);
}
