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
#include "Mcro/Concepts.h"

/**
 *	@brief  This namespace provides templating utilities and introspection into template instantiations.
 */
namespace Mcro::Templates
{
	using namespace Mcro::Concepts;

	namespace Detail
	{
		template <size_t I, typename... T>
		constexpr bool CheckBounds()
		{
			// If you get a division-by-zero error here that means the given parameter pack was indexed out-of-bounds.
			int ref = 1 / (I <= sizeof...(T) ? 1 : 0);
			return ref == 1;
		}
		
		template <size_t I, typename First = void, typename... Rest>
		struct TTypeAtPack_Impl
		{
			using Type = typename TTypeAtPack_Impl<I - 1, Rest...>::Type;
		};

		template <typename First, typename... Rest>
		struct TTypeAtPack_Impl<0, First, Rest...>
		{
			using Type = First;
		};
	}
	
	template <size_t I, typename... T>
	struct TTypeAtPack_Struct
	{
		using Type = std::enable_if_t<
			Detail::CheckBounds<I, T...>(),
			typename Detail::TTypeAtPack_Impl<I, T...>::Type
		>;
	};

	template <size_t I>
	struct TTypeAtPack_Struct<I> { using Type = void; };

	/**
	 *	@brief
	 *	Get a specific item from a parameter pack at given index. It is an unspecified compile error to index an empty
	 *	parameter pack.
	 */
	template<size_t I, typename... Rest>
	using TTypeAtPack = typename TTypeAtPack_Struct<I, Rest...>::Type;

	/**
	 *	@brief
	 *	Get a specific item from the end of a parameter pack at given index (0 == last). It is an unspecified compile
	 *	error to index an empty parameter pack.
	 */
	template<size_t I, typename... Rest>
	using TLastTypeAtPack = typename TTypeAtPack_Struct<sizeof...(Rest) - I, Rest...>::Type;

	/**
	 *	@brief
	 *	Get a specific item from a parameter pack at given index disregarding CV-ref qualifiers. It is an unspecified
	 *	compile error to index an empty parameter pack.
	 */
	template<size_t I, typename... Rest>
	using TTypeAtPackDecay = std::decay_t<typename TTypeAtPack_Struct<I, Rest...>::Type>;

	/**
	 *	@brief
	 *	Get a specific item from the end of a parameter pack at given index (0 == last) disregarding CV-ref qualifiers.
	 *	It is an unspecified compile error to index an empty parameter pack.
	 */
	template<size_t I, typename... Rest>
	using TLastTypeAtPackDecay = std::decay_t<typename TTypeAtPack_Struct<sizeof...(Rest) - I, Rest...>::Type>;
	
	/**
	 *	@brief
	 *	This template is used to store pack of types in other templates, or to allow parameter pack inference for
	 *	functions. This template may be referred to as 'type-list' in other parts of the documentation.
	 *
	 *	This may be much safer to use than tuples as they may try to use deleted features of listed types (especially
	 *	Unreal tuples). `TTypes` will never attempt to use its arguments (not even in `decltype` or `declval` contexts)
	 */
	template <typename... T>
	struct TTypes
	{
		static constexpr size_t Count = sizeof...(T);

		template <size_t I>
		using Get = TTypeAtPack<I, T...>;

		template <size_t I>
		using GetDecay = TTypeAtPackDecay<I, T...>;
	};

	template <typename T>
	struct TIsTypeList_Struct { static constexpr bool Value = false; };

	template <typename... T>
	struct TIsTypeList_Struct<TTypes<T...>> { static constexpr bool Value = true; };

	/** @brief Concept constraining a given type to `TTypes` */
	template <typename T>
	concept CIsTypeList = TIsTypeList_Struct<T>::Value;
	
	template <CIsTypeList T, size_t I>
	using TTypes_Get = T::template Get<I>;
	
	template <CIsTypeList T, size_t I>
	using TTypes_GetDecay = T::template GetDecay<I>;
	
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
		static constexpr size_t ParameterCount = 0;
    
		template <typename... Params>
		static constexpr size_t ParameterCount<Template<Params...>> = sizeof...(Params);

		template <typename T>
		struct Parameters
		{
			using Type = TTypes<>;
		};

		template <typename... Params>
		struct Parameters<Template<Params...>>
		{
			using Type = TTypes<Params...>;
		};

		template <typename T>
		struct ParametersDecay
		{
			using Type = TTypes<>;
		};

		template <typename... Params>
		struct ParametersDecay<Template<Params...>>
		{
			using Type = TTypes<std::decay_t<Params>...>;
		};

		template <typename T>
		struct ParametersTuple
		{
			using Type = TTuple<>;
		};

		template <typename... Params>
		struct ParametersTuple<Template<Params...>>
		{
			using Type = TTuple<Params...>;
		};

		template <typename T>
		struct ParametersTupleDecay
		{
			using Type = TTuple<>;
		};

		template <typename... Params>
		struct ParametersTupleDecay<Template<Params...>>
		{
			using Type = TTuple<std::decay_t<Params>...>;
		};

		template <typename Instance, int I>
		using Param = Parameters<Instance>::Type::template Get<I>;

		template <typename Instance, int I>
		using ParamDecay = ParametersDecay<Instance>::Type::template GetDecay<I>;
	};
	
	/**
	 *	@brief  Get template type parameters as a `TTypes` type-list
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <template <typename...> typename Template, typename Instance>
	using TTemplate_Params = typename TTemplate<Template>::template Parameters<Instance>::Type;
	
	/**
	 *	@brief  Get decayed template type parameters as a `TTypes` type-list
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <template <typename...> typename Template, typename Instance>
	using TTemplate_ParamsDecay = typename TTemplate<Template>::template ParametersDecay<Instance>::Type;
	
	/**
	 *	@brief  Get template type parameters as a tuple
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <template <typename...> typename Template, typename Instance>
	using TTemplate_ParamsTuple = typename TTemplate<Template>::template ParametersTuple<Instance>::Type;
	
	/**
	 *	@brief  Get decayed template type parameters as a tuple
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <template <typename...> typename Template, typename Instance>
	using TTemplate_ParamsTupleDecay = typename TTemplate<Template>::template ParametersTupleDecay<Instance>::Type;

	/**
	 *	@brief  Get a type parameter at a specified position of a templated instance. 
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <template <typename...> typename Template, typename Instance, int I>
	using TTemplate_Param = typename TTemplate<Template>::template Param<Instance, I>;

	/**
	 *	@brief  Get a decayed type parameter at a specified position of a templated instance. 
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <template <typename...> typename Template, typename Instance, int I>
	using TTemplate_ParamDecay = typename TTemplate<Template>::template ParamDecay<Instance, I>;

	/**
	 *	@brief  Check if given type is an instantiation of a given template (which only accepts type parameters)
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <typename Instance, template <typename...> typename Template>
	concept CIsTemplate = TTemplate<Template>::template Match<std::decay_t<Instance>>;

	/**
	 *	@brief
	 *	Get the number of template type parameters from a specified templated instance (which only has type parameters) 
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <template <typename...> typename Template, typename Instance>
	inline constexpr size_t TTemplate_ParamCount =  TTemplate<Template>::template ParameterCount<Instance>;

	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName>(...)`? use this instead */
	template <CConstType T>
	constexpr auto&& AsConst(T&& input) { return Forward<T>(input); }
	
	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName>(...)`? use this instead */
	template <CMutableType T>
	constexpr auto&& AsConst(T&& input) { return Forward<T>(const_cast<const T>(input)); }

	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName>(...)`? use this instead */
	template <CMutableType T>
	constexpr auto&& AsMutable(T&& input) { return Forward<T>(input); }
	
	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName>(...)`? use this instead */
	template <CConstType T>
	constexpr auto&& AsMutable(T&& input) { return Forward<T>(const_cast<T>(input)); }

	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName*>(...)`? use this instead */
	template <typename T>
	constexpr auto AsConstPtr(const T* input) { return input; }
	
	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName*>(...)`? use this instead */
	template <typename T>
	constexpr auto AsConstPtr(T* input) { return const_cast<const T*>(input); }

	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName*>(...)`? use this instead */
	template <typename T>
	constexpr auto AsMutablePtr(T* input) { return input; }
	
	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName*>(...)`? use this instead */
	template <typename T>
	constexpr auto AsMutablePtr(const T* input) { return const_cast<T*>(input); }
}
