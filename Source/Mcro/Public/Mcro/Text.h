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
#include "Mcro/TypeName.h"

#ifndef MCRO_TEXT_ALLOW_UNSUPPORTED_STRING_CONVERSION
/**
 *	@brief
 *	If this flag is on, then the default string conversion behavior is returning the result of `TTypeName` and
 *	triggering an ensure. Otherwise, a missing function will result in a compile error.
 */
#define MCRO_TEXT_ALLOW_UNSUPPORTED_STRING_CONVERSION 0
#endif

/** @brief Mixed text utilities and type traits */
namespace Mcro::Text
{
	using namespace Mcro::Concepts;
	using namespace Mcro::TypeName;

	/** @brief Unreal style alias for STL strings */
	template <
		class CharT,
		class Traits = std::char_traits<CharT>,
		class Allocator = std::allocator<CharT>
	>
	using TStdString = std::basic_string<CharT, Traits, Allocator>;

	/** @brief Unreal style alias for STL string views */
	template <
		class CharT,
		class Traits = std::char_traits<CharT>
	>
	using TStdStringView = std::basic_string_view<CharT, Traits>;

	using FUtf16StringView = TStringView<UTF16CHAR>;
	using FUtf32StringView = TStringView<UTF32CHAR>;
	
	/** @brief cross-TCHAR alias for `std::[w]string` */
	using FStdString = TStdString<TCHAR>;
	
	/** @brief cross-TCHAR alias for `std::[w]string_view` */
	using FStdStringView = TStdStringView<TCHAR>;

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
	concept CStdStringOrViewTyped = CConvertibleToDecayed<T, TStdStringView<CharType>>;

	/** @brief Concept constraining given type to only a std::string of any character type */
	template <typename T>
	concept CStdStringInvariant =
		CSameAsDecayed<
			T,
			TStdString<
				typename T::value_type,
				typename T::traits_type
			>
		>
	;

	/** @brief Concept constraining given type to only a std::string_view of any character type */
	template <typename T>
	concept CStdStringViewInvariant =
		CSameAsDecayed<
			T,
			TStdStringView<
				typename T::value_type,
				typename T::traits_type
			>
		>
	;

	/** @brief Concept constraining given type to a std::string or a view of any character type */
	template <typename T>
	concept CStdStringOrViewInvariant =
		CConvertibleToDecayed<
			T,
			TStdStringView<
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
	constexpr auto UnrealView(TStdStringView<CharType> const& stdStr)
	{
		return TStringView<CharType>(stdStr.data(), stdStr.size());
	}

	/** @brief View an Unreal `TStringView` via an STL string view */
	template <CStringViewInvariant T>
	constexpr auto StdView(T const& unrealStr)
	{
		return TStdStringView<typename T::ElementType>(unrealStr.GetData(), unrealStr.Len());
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
	auto StdConvert(FStringView const& unrealStr) -> TStdString<ConvertTo>
	{
		if constexpr (CSameAs<TCHAR, ConvertTo>)
			return TStdString<ConvertTo>(unrealStr.GetData(), unrealStr.Len());
		else
		{
			return Detail::HighLevelStringCast<TCHAR, ConvertTo>(
				unrealStr,
				[&] { return unrealStr.GetData(); },
				[&] { return unrealStr.Len(); },
				[](const ConvertTo* ptr, int32 len) { return TStdString(ptr, len); }
			);
		}
	}

	/** @brief Create a copy and convert an input STL string of TCHAR to the given character type */
	template <typename ConvertTo>
	auto StdConvert(FStdStringView const& stdStr) -> TStdString<ConvertTo>
	{
		if constexpr (CSameAs<TCHAR, ConvertTo>)
			return TStdString<ConvertTo>(stdStr.data(), stdStr.size());
		else
		{
			return Detail::HighLevelStringCast<TCHAR, ConvertTo>(
				stdStr,
				[&] { return stdStr.data(); },
				[&] { return stdStr.size(); },
				[](const ConvertTo* ptr, int32 len) { return TStdString(ptr, len); }
			);
		}
	}

	/** @brief Create a copy and convert an input FName to the given character type */
	template <typename ConvertTo>
	auto StdConvert(FName const& name) -> TStdString<ConvertTo>
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

	/** @brief A type which which provides a `ToString()` member method */
	template <typename T>
	concept CHasPtrToString = !CDirectStringFormatArgument<T> && !CHasToString<T> && requires(T t)
	{
		{ t->ToString() } -> CDirectStringFormatArgument;
	};

	template <typename T>
	struct TAsFormatArgument
	{
#if MCRO_TEXT_ALLOW_UNSUPPORTED_STRING_CONVERSION
		template <CConvertibleToDecayed<T> Arg>
		FStringView operator () (Arg&& left) const
		{
			ensureAlwaysMsgf(false,
				TEXT("Given type %s couldn't be converted to a string. Typename is returned instead"),
				*TTypeString<T>()
			);
			return TTypeName<T>;
		}
#endif
	};

	/** @brief A type which can be used with FStringFormatArg via a `TAsFormatArgument` specialization. */
	template <typename T>
	concept CHasStringFormatArgumentConversion = !CDirectStringFormatArgument<T> && requires(T&& t)
	{
		{ TAsFormatArgument<T>()(t) } -> CDirectStringFormatArgument;
	};

	/** @brief A type which can be converted to FStringFormatArg via any method. */
	template <typename T>
	concept CStringFormatArgument =
		CDirectStringFormatArgument<std::decay_t<T>>
		|| CHasToString<std::decay_t<T>>
		|| CHasPtrToString<std::decay_t<T>>
		|| CHasStringFormatArgumentConversion<std::decay_t<T>>
	;

	template <CDirectStringFormatArgument Operand>
	requires (!CEnum<Operand>)
	struct TAsFormatArgument<Operand>
	{
		template <CConvertibleToDecayed<Operand> Arg>
		Operand operator () (Arg&& left) const { return left; }
	};

	template <CHasToString Operand>
	struct TAsFormatArgument<Operand>
	{
		template <CConvertibleToDecayed<Operand> Arg>
		auto operator () (Arg&& left) const { return left.ToString(); }
	};

	template <CHasPtrToString Operand>
	struct TAsFormatArgument<Operand>
	{
		template <CConvertibleToDecayed<Operand> Arg>
		auto operator () (Arg&& left) const { return left->ToString(); }
	};

	template <typename CharType>
	struct TAsFormatArgument<TStdString<CharType>>
	{
		const CharType* operator () (TStdString<CharType> const& left) const
		{
			return left.c_str();
		}
	};

	template <>
	struct TAsFormatArgument<FStdStringView>
	{
		FStringView operator () (FStdStringView const& left) const
		{
			return UnrealView(left);
		}
	};

	template <CStdStringViewInvariant Operand>
	requires (!CSameAsDecayed<Operand, FStdStringView>)
	struct TAsFormatArgument<Operand>
	{
		template <CConvertibleToDecayed<Operand> Arg>
		auto operator () (Arg&& left) const { return UnrealConvert(left); }
	};

	template <CStringFormatArgument T>
	auto AsFormatArgument(T&& input)
	{
		return TAsFormatArgument<std::decay_t<T>>()(input);
	}

	/**
	 *	@brief  Attempt to convert anything to string which can tell via some method how to do so
	 *
	 *	This may be more expensive than directly using an already existing designated string conversion for a given
	 *	type, because it uses `FStringFormatArg` and a `TAsFormatArgument` as intermediate steps. However, this can be
	 *	still useful for types where such conversion doesn't already exist or when using this in a template.
	 *
	 *	It's still much faster than using `FMT_(myVar) "{0}"`
	 */
	template <CStringFormatArgument T>
	requires(!CSameAsDecayed<T, FString>)
	FString AsString(T&& input)
	{
		FStringFormatArg format(AsFormatArgument(input));
		return MoveTemp(format.StringValue);
	}
	
	template <CSameAsDecayed<FString> T>
	decltype(auto) AsString(T&& input)
	{
		return Forward<T>(input);
	}

	/**
	 *	@brief  Create an ordered argument list for a string format from input arguments
	 *
	 *	While you can it is not recommended to be used directly because the boilerplate it still needs is very verbose.
	 *	Check `_FMT` or `FMT_` macros for a comfortable string format syntax.
	 */
	template <CStringFormatArgument... Args>
	FStringFormatOrderedArguments OrderedArguments(Args&&... args)
	{
		return FStringFormatOrderedArguments { FStringFormatArg(AsFormatArgument(args)) ... };
	}

	/**
	 *	@brief  Create a named argument list for a string format from input argument tuples
	 *
	 *	While you can it is not recommended to be used directly because the boilerplate it still needs is very verbose.
	 *	Check `_FMT` or `FMT_` macros for a comfortable string format syntax.
	 */
	template <CStringFormatArgument... Args>
	FStringFormatNamedArguments NamedArguments(TTuple<FString, Args>... args)
	{
		return FStringFormatNamedArguments {
			{ args.template Get<0>(), FStringFormatArg(AsFormatArgument(args.template Get<1>())) }
			...
		};
	}
}
