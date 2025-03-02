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
 * @file
 * @brief  Convert types to string
 */

#pragma once

#include <string>
#include "ConstexprXXH3.h"

#include "CoreMinimal.h"
#include "UnrealCtre.h"
#include "Mcro/TextMacros.h"
#include "Mcro/Platform.h"

#define MCRO_EXPLICIT_TYPE_EXTRACTION defined(MCRO_EXPLICIT_PRETTY_FUNCTION) && defined(MCRO_EXPLICIT_TYPE_EXTRACT_REGEX)

// TODO: C++ 20's std::source_location was used instead of this bouquet of macros, but then some platforms don't have <source_location>
#if MCRO_COMPILER_GCC || MCRO_COMPILER_CLANG
#define PRETTY_FUNC __PRETTY_FUNCTION__
#elif MCRO_COMPILER_MSVC
#define PRETTY_FUNC __FUNCSIG__
#elif MCRO_EXPLICIT_TYPE_EXTRACTION
#define PRETTY_FUNC MCRO_EXPLICIT_PRETTY_FUNCTION
#else
#error "TTypeName is only compatible with Clang, GCC and MSVC. Please create a PR for your compiler at https://github.com/microdee/mcro/pulls.
#error "If your compiler is under NDA add the following definitions in your target rules (values are examples):"
#error "`MCRO_EXPLICIT_PRETTY_FUNCTION=__PRETTY_FUNCTION__`"
#error "`MCRO_EXPLICIT_TYPE_EXTRACT_REGEX=\"GetCompileTimeTypeName\\\\<((struct|class)\\\\s)?(?<TYPE>.+?)\\\\>\\\\(\"`"
#endif

// disable this warning to allow us exploit division by zero error in consteval
#pragma warning(push)
#pragma warning(disable: 4804) // unsafe use of type 'bool' in operation

/**
 *	@brief  Get a string view of the compile time typename
 *	
 *	@note
 *	Using std::(w)string here instead of FString or FStringView for consteval compatibility
 *	
 *	@tparam  T  target type
 *	@return  string view of the name of the type
 */
template <typename T>
consteval std::basic_string_view<TCHAR> GetCompileTimeTypeName()
{
	using namespace Mcro::Text;
	std::basic_string_view<TCHAR> thisFunctionName { TEXT(PRETTY_FUNC) };
	
#if MCRO_COMPILER_GCC || MCRO_COMPILER_CLANG
	
	// on Clang thisFunctionName looks like this:
	//        matching: [--------------------------------------------]
	//                  |                  capturing: [-------------]|
	// std::string_view GetCompileTimeTypeName() [T = MyTemplate<Vec>]
	
	// on GCC thisFunctionName has a similar enough format so we can use the same pattern:
	//                  matching: [-------------------------------------------------]
	//                            |                       capturing: [-------------]|
	// consteval std::string_view GetCompileTimeTypeName() [with T = MyTemplate<Vec>; std::string_view = std::basic_string_view<char>]
	
	auto result = ctre::search<TEXT_ R"(\s\[(?:with\s)?T\s=\s(?<TYPE>.+?)[;\]])">(thisFunctionName);
	
#elif MCRO_COMPILER_MSVC
	
	// on MSVC thisFunctionName looks like this:
	//                                                                 matching: [----------------------------------------------------]
	//                                                                           |                  capturing: [---------------------]|
	// class std::basic_string_view<char,struct std::char_traits<char> > __cdecl GetCompileTimeTypeName<struct MyTemplate<struct Vec>>(void)
	
	auto result = ctre::search<TEXT_ R"(GetCompileTimeTypeName\<((struct|class)\s)?(?<TYPE>.+?)\>\()">(thisFunctionName);

#elif MCRO_EXPLICIT_TYPE_EXTRACTION
	auto result = ctre::search<TEXT(MCRO_EXPLICIT_TYPE_EXTRACT_REGEX)>(thisFunctionName);
#endif
	
	std::basic_string_view<TCHAR> output = result.get<TEXT_"TYPE">().to_view();
	size_t size = output.size();
	size /= output.size() > 0; // Fail compilation with division-by-zero if we couldn't extract a typename
	
	return size ? output : std::basic_string_view<TCHAR>();
}

template <typename T>
consteval uint64 GetCompileTimeTypeHash()
{
	std::string_view thisFunctionName { PRETTY_FUNC };
	return constexpr_xxh3::XXH3_64bits_const(thisFunctionName);
}

/** Convert types to string */
namespace Mcro::TypeName
{
	using namespace Mcro::Text;
	
	/**
	 *	@brief
	 *	Get a friendly string of an input type without using `typeid(T).name()`.
	 *
	 *	Actually this method gives more predictable results across compilers than `typeid(T).name()` but still not 100%
	 *	the same. Besides TTypeName can deal with incomplete types as well, whereas `typeid(T)` cannot (more on this in
	 *	remarks). While it is more consistent across compilers, it is still not 100% consistent. Take templated types
	 *	for instance where MSVC compiler will add "class" prefix in front of type arguments where GCC or CLang might not.
	 *	Because of subtle differences like this, it is strongly discouraged to assume the exact format of the output of
	 *	TTypeName
	 *	
	 *	Usage:
	 *	@code
	 *	FStringView myTypeName = TTypeName<FMyType>;
	 *	@endcode
	 *	
	 *	It is useful in templates where a type name should be known in runtime as well. (e.g.: Modular features)
	 *	
	 *	@remarks
	 *	Note on incomplete types: incomplete types on MSVC are localized to current scope. which means
	 *	@code
	 *	class I;
	 *	
	 *	class A
	 *	{
	 *		FStringView GetName() { return TTypeName<I>; }
	 *		// Returns "I"
	 *	}
	 *	@endcode
	 *	BUT:
	 *	@code
	 *	class A
	 *	{
	 *		FStringView GetName() { return TTypeName<class I>; }
	 *		// Returns "A::GetName::I"
	 *	}
	 *	@endcode
	 *	as you can see they are not equal. For sake of simplicity TTypeName only supports incomplete
	 *	types when they're declared in global scope. Any other case might yield undesired results
	 *	(e.g.: `TTypeName<class I>` of incomplete `I` might not be the same as `TTypeName<I>` somewhere
	 *	else with `I` being fully defined)
	 *	
	*	@tparam  T  The type to be named
	 *
	 *	@warning
	 *	Do not use exact type comparison with serialized data or network communication, as the actual value of the type
	 *	is different between compilers. Only use this for runtime data. For such scenarios just use Unreal's own UObjects.
	 */
	template <typename T>
	constexpr FStringView TTypeName = FStringView(
		GetCompileTimeTypeName<std::decay_t<T>>().data(),
		GetCompileTimeTypeName<std::decay_t<T>>().size()
	);

	/** @brief Same as `TTypeName` just stored as STL string made during compile time */
	template <typename T>
	constexpr std::basic_string_view<TCHAR> TTypeNameStd = GetCompileTimeTypeName<std::decay_t<T>>();

	/** @brief Get a fixed `uint64` hash representation of the given type. Have similar caveats as `TTypeName` */
	template <typename T>
	constexpr uint64 TTypeHash = GetCompileTimeTypeHash<T>();

	/**
	 *	@brief
	 *	Same as `TTypeName` converted to FName. This is not cached and a new FName is created every time this is called.
	 */
	template <typename T>
	FName TTypeFName()
	{
		return FName(TTypeName<T>.Len(), TTypeName<T>.GetData());
	}

	/**
	 *	@brief
	 *	Same as `TTypeName` converted to FString. This is not cached and a new FString is created every time this is called.
	 */
	template <typename T>
	FString TTypeString()
	{
		return FString::ConstructFromPtrSize(TTypeName<T>.GetData(), TTypeName<T>.Len());
	}
}

#pragma warning(pop)
