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
#include "Mcro/FunctionTraits.h"

/** @brief Mixed text utilities and type traits */
namespace Mcro::Text
{
	using namespace Mcro::Concepts;

	using FUtf16StringView = TStringView<UTF16CHAR>;
	using FUtf32StringView = TStringView<UTF32CHAR>;
	
	/** @brief cross-TCHAR typedef for `std::[w]string` */
	using FStdString = std::basic_string<TCHAR>;
	
	/** @brief cross-TCHAR typedef for `std::[w]string_view` */
	using FStdStringView = std::basic_string_view<TCHAR>;

	/** @brief Concept constraining given type to a string view of any character type */
	template<typename T>
	concept CStringViewInvariant = CSameAsDecayed<T, TStringView<typename T::ElementType>>;

	/** @brief Concept constraining given type to a string of any character type */
	template<typename T>
	concept CStringInvariant = CSameAsDecayed<T, FString>
		|| CSameAsDecayed<T, FAnsiString>
		|| CSameAsDecayed<T, FUtf8String>
	;

	/** @brief Concept constraining given type to a string or a a view of TCHAR */
	template<typename T>
	concept CStringOrView = CSameAsDecayed<T, FString> || CSameAsDecayed<T, FStringView>;

	/** @brief Concept constraining given type to a string or a a view of any character type */
	template<typename T>
	concept CStringOrViewInvariant = CStringInvariant<T> || CStringViewInvariant<T>;
	
	/** @brief Concept constraining given type to a string, a view or an FName of TCHAR */
	template<typename T>
	concept CStringOrViewOrName = CStringOrView<T> || CSameAsDecayed<T, FName>;

	/** @brief Concept constraining given type to a std::string or a view of TCHAR */
	template <typename T>
	concept CStdStringOrView = CConvertibleToDecayed<T, FStdStringView>;

	/** @brief Concept constraining given type to a std::string or a view of given character type */
	template <typename T, typename CharType>
	concept CStdStringOrViewTyped = CConvertibleToDecayed<T, std::basic_string_view<CharType>>;

	/** @brief Concept constraining given type to a std::string or a view of any character type */
	template <typename T>
	concept CStdStringOrViewInvariant =
		CConvertibleToDecayed<
			T,
			std::basic_string_view<
				typename T::value_type,
				typename T::traits_type
			>
		>
	;

	namespace Detail
	{
		using namespace Mcro::FunctionTraits;
	
		template <
			typename CharFrom, typename CharOutput,
			typename StringFrom,
			CFunctionLike PtrGetter, CFunctionLike LengthGetter, CFunctionLike Constructor,
			typename StringTo = TFunction_Return<Constructor>
		>
		StringTo HighLevelStringCast(const StringFrom& strIn, PtrGetter&& getPtr, LengthGetter&& getLength, Constructor&& construct)
		{
			if constexpr (CSameAs<CharFrom, CharOutput>)
			{
				return construct(
					reinterpret_cast<const CharOutput*>(getPtr()),
					getLength()
				);
			}
			else
			{
				auto conversion = StringCast<CharOutput>(
					reinterpret_cast<const CharFrom*>(getPtr()),
					getLength()
				);
				return construct(
					reinterpret_cast<const CharOutput*>(conversion.Get()),
					conversion.Length()
				);
			}
		}
	}

	/** @brief View an STL string object via an Unreal `TStringView` */
	template <typename CharType>
	constexpr auto UnrealView(std::basic_string_view<CharType> const& stdStr)
	{
		return TStringView<CharType>(stdStr.data(), stdStr.size());
	}

	/** @brief View an Unreal `TStringView` via an STL string view */
	template <CStringViewInvariant T>
	constexpr auto StdView(T const& unrealStr)
	{
		return std::basic_string_view<typename T::ElementType>(unrealStr.GetData(), unrealStr.Len());
	}

	/** @brief View an Unreal string via an STL string view */
	template <CConvertibleToDecayed<FString> T>
	auto StdView(T const& unrealStr)
	{
		return FStdStringView(*unrealStr, unrealStr.Len());
	}

	/** @brief Create a copy of an input STL string */
	MCRO_API FString UnrealCopy(FStdStringView const& stdStr);

	/** @brief Create a copy and convert an input STL string to TCHAR */
	template <CStdStringOrViewInvariant T>
	FString UnrealConvert(T const& stdStr)
	{
		return Detail::HighLevelStringCast<typename T::value_type, TCHAR>(
			stdStr,
			[&] { return stdStr.data(); },
			[&] { return stdStr.length(); },
			[](const TCHAR* ptr, int32 len) { return FString::ConstructFromPtrSize(ptr, len); }
		);
	}
	
	/** @brief Create a copy of an input STL string as an FName */
	MCRO_API FName UnrealNameCopy(FStdStringView const& stdStr);
	

	/** @brief Create a copy and convert an input STL string to TCHAR as an FName */
	template <CStdStringOrViewInvariant T>
	FName UnrealNameConvert(T const& stdStr)
	{
		return Detail::HighLevelStringCast<typename T::value_type, TCHAR>(
			stdStr,
			[&] { return stdStr.data(); },
			[&] { return stdStr.length(); },
			[](const TCHAR* ptr, int32 len) { return FName(len, ptr); }
		);
	}

	/** @brief Create an Stl copy of an input Unreal string view */
	MCRO_API FStdString StdCopy(FStringView const& unrealStr);
	
	/** @brief Create an Stl copy of an input Unreal string */
	MCRO_API FStdString StdCopy(FName const& unrealStr);

	/** @brief Create a copy and convert an input Unreal string to the given character type */
	template <typename ConvertTo>
	auto StdConvert(FStringView const& unrealStr) -> std::basic_string<ConvertTo>
	{
		if constexpr (CSameAs<TCHAR, ConvertTo>)
			return std::basic_string<ConvertTo>(unrealStr.GetData(), unrealStr.Len());
		else
		{
			return Detail::HighLevelStringCast<TCHAR, ConvertTo>(
				unrealStr,
				[&] { return unrealStr.GetData(); },
				[&] { return unrealStr.Len(); },
				[](const ConvertTo* ptr, int32 len) { return std::basic_string(ptr, len); }
			);
		}
	}

	/** @brief Create a copy and convert an input STL string of TCHAR to the given character type */
	template <typename ConvertTo>
	auto StdConvert(FStdStringView const& stdStr) -> std::basic_string<ConvertTo>
	{
		if constexpr (CSameAs<TCHAR, ConvertTo>)
			return std::basic_string<ConvertTo>(stdStr.data(), stdStr.size());
		else
		{
			return Detail::HighLevelStringCast<TCHAR, ConvertTo>(
				stdStr,
				[&] { return stdStr.data(); },
				[&] { return stdStr.size(); },
				[](const ConvertTo* ptr, int32 len) { return std::basic_string(ptr, len); }
			);
		}
	}

	/** @brief Create a copy and convert an input FName to the given character type */
	template <typename ConvertTo>
	auto StdConvert(FName const& name) -> std::basic_string<ConvertTo>
	{
		return StdConvert<ConvertTo>(name.ToString());
	}

	/** @brief Join the given string arguments with a delimiter and skip empty entries */
	template <CStringOrView... Args>
	FString Join(const TCHAR* separator, Args... args)
	{
		return FString::Join(
			TArray<FString>{args...}.FilterByPredicate([](FStringView const& str) { return !str.IsEmpty(); }),
			separator
		);
	}

	/** @brief Copy of FString::PrintfImpl but not private so it can work with strings which were not literals */
	MCRO_API FString DynamicPrintf(const TCHAR* fmt, ...);

	/** @brief A type which is directly convertible to FStringFormatArg */
	template <typename T>
	concept CDirectStringFormatArgument = CConvertibleTo<T, FStringFormatArg>;

	/** @brief A type which which provides a `ToString()` member method */
	template <typename T>
	concept CHasToString = !CDirectStringFormatArgument<T> && requires(T t)
	{
		{ t.ToString() } -> CDirectStringFormatArgument;
	};

	/** @brief An empty tag struct used to extend rigid types to be convertible to FStringFormatArg */
	struct FStringFormatArgumentTag {};

	/** @brief A type which can be used with FStringFormatArg via a `% FStringFormatArgumentTag()` operator. */
	template <typename T>
	concept CHasStringFormatArgumentConversion = !CDirectStringFormatArgument<T> && requires(T&& t)
	{
		{ t % FStringFormatArgumentTag() } -> CDirectStringFormatArgument;
	};

	/** @brief A type which can be converted to FStringFormatArg via any method. */
	template <typename T>
	concept CStringFormatArgument = CDirectStringFormatArgument<T>
		|| CHasToString<T>
		|| CHasStringFormatArgumentConversion<T>
	;
	
	template <CDirectStringFormatArgument Operand>
	Operand operator % (Operand&& left, FStringFormatArgumentTag&&)
	{
		return left;
	}

	template <CHasToString Operand>
	auto operator % (Operand&& left, FStringFormatArgumentTag&&)
	{
		return left.ToString();
	}

	template <typename CharType>
	const CharType* operator % (std::basic_string<CharType> const& left, FStringFormatArgumentTag&&)
	{
		return left.c_str();
	}

	template <CStdStringOrView Operand>
	FStringView operator % (Operand&& left, FStringFormatArgumentTag&&)
	{
		return UnrealView(left);
	}

	/**
	 *	@brief Create an ordered argument list for a string format from input arguments
	 *
	 *	While you can it is not recommended to be used directly because the boilerplate it still needs is very verbose.
	 *	Check `_FMT` or `FMT_` macros for a comfortable string format syntax.
	 */
	template <CStringFormatArgument... Args>
	FStringFormatOrderedArguments OrderedArguments(Args&&... args)
	{
		return FStringFormatOrderedArguments { FStringFormatArg(args % FStringFormatArgumentTag()) ... };
	}

	/**
	 *	@brief Create a named argument list for a string format from input argument tuples
	*
	 *	While you can it is not recommended to be used directly because the boilerplate it still needs is very verbose.
	 *	Check `_FMT` or `FMT_` macros for a comfortable string format syntax.
	 */
	template <CStringFormatArgument... Args>
	FStringFormatNamedArguments NamedArguments(TTuple<FString, Args>... args)
	{
		return FStringFormatNamedArguments {
			{ args.template Get<0>(), FStringFormatArg(args.template Get<1>() % FStringFormatArgumentTag()) }
			...
		};
	}
}
