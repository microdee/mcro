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

#include "CoreMinimal.h"
#include "Concepts.h"

/**
 *	@brief  This namespace provide some introspection into template instantiations.
 *
 *	@warning
 *	Members of this namespace are very limited in usage and therefore should be used with utmost care.
 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is considered
 *	seriously, template traits only work with templates which only have type-parameters. Non-type parameters even when
 *	a default is specified for them will result in compile error.
 */
namespace Mcro::Templates
{
	using namespace Mcro::Concepts;
	
	/**
	 *	@brief  Base struct containing traits of specified template (which only accepts type parameters)
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <template <typename...> typename Template>
	struct TTemplate
	{
		template <typename T>
		static constexpr bool Match = false;
    
		template <typename... Params>
		static constexpr bool Match<Template<Params...>> = true;

		template <typename T>
		struct Parameters
		{
			using Type = TTuple<>;
		};

		template <typename... Params>
		struct Parameters<Template<Params...>>
		{
			using Type = TTuple<Params...>;
		};
	};
	
	/**
	 *	@brief  Get template type parameters as a tuple
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <template <typename...> typename Template, typename T>
	using TTemplate_Params = typename TTemplate<Template>::template Parameters<T>::Type;

	/**
	 *	@brief  Check if given type is an instantiation of a given template (which only accepts type parameters)
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <typename T, template <typename...> typename Template>
	concept CIsTemplate = TTemplate<Template>::template Match<std::decay_t<T>>;

	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName>(...)`? use this instead */
	template <CConstType T>
	auto AsConst(T&& input) { return Forward<T>(input); }
	
	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName>(...)`? use this instead */
	template <CMutableType T>
	auto AsConst(T&& input) { return Forward<T>(const_cast<const T>(input)); }

	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName>(...)`? use this instead */
	template <CMutableType T>
	auto AsMutable(T&& input) { return Forward<T>(input); }
	
	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName>(...)`? use this instead */
	template <CConstType T>
	auto AsMutable(T&& input) { return Forward<T>(const_cast<T>(input)); }

	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName*>(...)`? use this instead */
	template <typename T>
	auto AsConstPtr(const T* input) { return input; }
	
	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName*>(...)`? use this instead */
	template <typename T>
	auto AsConstPtr(T* input) { return const_cast<const T*>(input); }

	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName*>(...)`? use this instead */
	template <typename T>
	auto AsMutablePtr(T* input) { return input; }
	
	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName*>(...)`? use this instead */
	template <typename T>
	auto AsMutablePtr(const T* input) { return const_cast<T*>(input); }
}
