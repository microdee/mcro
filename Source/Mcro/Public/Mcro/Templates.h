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
#include "Mcro/Macros.h"
#include "Mcro/Concepts.h"

/**
 *	@brief  This namespace provides templating utilities and introspection into template instantiations.
 */
namespace Mcro::Templates
{
	using namespace Mcro::Concepts;

	namespace Detail
	{
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
		static_assert(I <= sizeof...(T), "Indexing parameter pack out of its bounds.");
		using Type = typename Detail::TTypeAtPack_Impl<I, T...>::Type;
	};

	template <size_t I>
	struct TTypeAtPack_Struct<I>
	{
		using Type = void;
	};

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
	using TLastTypeAtPack = typename TTypeAtPack_Struct<sizeof...(Rest) - I - 1, Rest...>::Type;

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
	using TLastTypeAtPackDecay = std::decay_t<typename TTypeAtPack_Struct<sizeof...(Rest) - I - 1, Rest...>::Type>;

	template <size_t I, typename T>
	struct TTupleSafeElement_Struct
	{
		static_assert(sizeof(T) == 0, "TTupleSafeElement_Struct is instantiated with non TTuple.");
	};

	template <size_t I, typename... T>
	struct TTupleSafeElement_Struct<I, TTuple<T...>>
	{
		using Type = TTypeAtPack<I, T...>;
	};
	
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
	concept CTypeList = TIsTypeList_Struct<T>::Value;
	
	template <CTypeList T, size_t I>
	using TTypes_Get = T::template Get<I>;
	
	template <CTypeList T, size_t I>
	using TTypes_GetDecay = T::template GetDecay<I>;

	/**
	 * Compose a type-list out of the elements of the input type-list based on the input index parameter pack
	 */
	template <CTypeList T, size_t... Indices>
	using TComposeTypeListFrom = TTypes<TTypes_Get<T, Indices>...>;

	template <size_t Count, CTypeList T>
	requires (T::Count >= Count)
	struct TTypesSkip_Struct
	{
		/**
		 * Since this is only meant to be used in decltype, no implementation is needed
		 */
		template <size_t... Indices>
		static consteval TComposeTypeListFrom<T, (Indices + Count)...> Compose(std::index_sequence<Indices...>&&);

		using Type = decltype(
			Compose(std::make_index_sequence<T::Count - Count>{})
		);
	};

	/**
	 * Skip the first `Count` elements of the input type-list
	 */
	template <size_t Count, CTypeList T>
	using TTypesSkip = typename TTypesSkip_Struct<Count, T>::Type;

	template <size_t Count, CTypeList T>
	requires (T::Count >= Count)
	struct TTypesTrimEnd_Struct
	{
		/**
		 * Since this is only meant to be used in decltype, no implementation is needed
		 */
		template <size_t... Indices>
		static consteval TComposeTypeListFrom<T, Indices...> Compose(std::index_sequence<Indices...>&&);

		using Type = decltype(
			Compose(std::make_index_sequence<T::Count - Count>{})
		);
	};

	/**
	 * Disregard the last `Count` elements of the input type-list
	 */
	template <size_t Count, typename T>
	using TTypesTrimEnd = typename TTypesTrimEnd_Struct<Count, T>::Type;

	template <size_t Count, typename T>
	requires (T::Count >= Count)
	struct TTypesTake_Struct
	{
		/**
		 * Since this is only meant to be used in decltype, no implementation is needed
		 */
		template <size_t... Indices>
		static consteval TComposeTypeListFrom<T, Indices...> Compose(std::index_sequence<Indices...>&&);

		using Type = decltype(
			Compose(std::make_index_sequence<Count>{})
		);
	};

	/**
	 * Take only the first `Count` elements of the input type-list
	 */
	template <size_t Count, typename T>
	using TTypesTake = typename TTypesTake_Struct<Count, T>::Type;

	namespace Detail
	{
		template <CTypeList TypeList, auto Test, size_t... Indices>
		consteval bool AllOfTypeList(bool onEmpty, std::index_sequence<Indices...>&&)
		{
			if constexpr (sizeof...(Indices) == 0)
				return onEmpty;
			else
				return (... && Test(TTypes<TTypes_Get<TypeList, Indices>>{}));
		}

		template <CTypeList TypeList, auto Test, size_t... Indices>
		consteval bool AllOfTypeListDecay(bool onEmpty, std::index_sequence<Indices...>&&)
		{
			if constexpr (sizeof...(Indices) == 0)
				return onEmpty;
			else
				return (... && Test(TTypes<TTypes_GetDecay<TypeList, Indices>>{}));
		}

		template <CTypeList TypeList, auto Test, size_t... Indices>
		consteval bool AnyOfTypeList(bool onEmpty, std::index_sequence<Indices...>&&)
		{
			if constexpr (sizeof...(Indices) == 0)
				return onEmpty;
			else
				return (... || Test(TTypes<TTypes_Get<TypeList, Indices>>{}));
		}

		template <CTypeList TypeList, auto Test, size_t... Indices>
		consteval bool AnyOfTypeListDecay(bool onEmpty, std::index_sequence<Indices...>&&)
		{
			if constexpr (sizeof...(Indices) == 0)
				return onEmpty;
			else
				return (... || Test(TTypes<TTypes_GetDecay<TypeList, Indices>>{}));
		}

		template <CTypeList From, CTypeList To, size_t... Indices>
		consteval bool IsTypeListConvertibleTo(bool onEmpty, std::index_sequence<Indices...>&&)
		{
			if constexpr (sizeof...(Indices) == 0)
				return onEmpty;
			else
				return (... && std::convertible_to<TTypes_Get<From, Indices>, TTypes_Get<To, Indices>>);
		}

		template <CTypeList From, CTypeList To, size_t... Indices>
		consteval bool IsTypeListConvertibleToDecay(bool onEmpty, std::index_sequence<Indices...>&&)
		{
			if constexpr (sizeof...(Indices) == 0)
				return onEmpty;
			else
				return (... && CConvertibleToDecayed<TTypes_Get<From, Indices>, TTypes_Get<To, Indices>>);
		}
	}

#define MCRO_TYPE_LIST_PREDICATE(Function, TypeList, OnEmpty, ...) \
	Function< \
		BOOST_PP_REMOVE_PARENS(TypeList), \
		[] <typename T> (TTypes<T>&&) { return __VA_ARGS__; } \
	>(OnEmpty, std::make_index_sequence<BOOST_PP_REMOVE_PARENS(TypeList)::Count>())

/**
 * @brief
 * Apply an arbitrary predicate to all types in a type-list, All must pass.
 *
 * @param typeList  Input typelist
 * @param  onEmpty  Default value if type-list is empty
 */
#define ALL_OF_TYPE_LIST(typeList, onEmpty, ...) MCRO_TYPE_LIST_PREDICATE(Detail::AllOfTypeList, typeList, onEmpty, __VA_ARGS__)

/**
 * @brief
 * Apply a concept to all types in a type-list, All must pass. If a concept needs extra template arguments you can
 * provide them as variadic macro arguments.
 *
 * @param  TypeList  Input typelist
 * @param   onEmpty  Default value if type-list is empty
 * @param conceptIn  The given concept to check for all items in the typeList
 */
#define C_ALL_OF_TYPE_LIST(typeList, onEmpty, conceptIn, ...) ALL_OF_TYPE_LIST(typeList, onEmpty, conceptIn<T __VA_OPT__(,) __VA_ARGS__>)

/**
 * @brief
 * Apply an arbitrary predicate to all types in a type-list, Any of them can pass.
 *
 * @param typeList  Input typelist
 * @param  onEmpty  Default value if type-list is empty
 */
#define ANY_OF_TYPE_LIST(typeList, onEmpty, ...) MCRO_TYPE_LIST_PREDICATE(Detail::AnyOfTypeList, typeList, onEmpty, __VA_ARGS__)

/**
 * @brief
 * Apply a concept to all types in a type-list, Any of them can pass. If a concept needs extra template arguments you can
 * provide them as variadic macro arguments.
 *
 * @param  TypeList  Input typelist
 * @param   onEmpty  Default value if type-list is empty
 * @param conceptIn  The given concept to check for all items in the typeList
 */
#define C_ANY_OF_TYPE_LIST(typeList, onEmpty, conceptIn, ...) ANY_OF_TYPE_LIST(typeList, onEmpty, conceptIn<T __VA_OPT__(,) __VA_ARGS__>)

/**
 * @brief
 * Apply an arbitrary predicate to all types in input type parameter pack, All must pass.
 *
 * @param typeList  input type parameter pack without ...
 * @param  onEmpty  Default value if type parameter pack is empty
 */
#define ALL_OF_TYPE_PACK(TypePack, onEmpty, ...) ALL_OF_TYPE_LIST(TTypes<TypePack...>, onEmpty, __VA_ARGS__)

/**
 * @brief
 * Apply a concept to all types in input type parameter pack, All must pass. If a concept needs extra template arguments
 * you can provide them as variadic macro arguments.
 *
 * @param  typeList  input type parameter pack without ...
 * @param   onEmpty  Default value if type parameter pack is empty
 * @param conceptIn  The given concept to check for all items in the type parameter pack
 */
#define C_ALL_OF_TYPE_PACK(TypePack, onEmpty, conceptIn, ...)  C_ALL_OF_TYPE_LIST(TTypes<TypePack...>, onEmpty, conceptIn, __VA_ARGS__)

/**
 * @brief
 * Apply an arbitrary predicate to all types in input type parameter pack, Any of them can pass.
 *
 * @param typeList  input type parameter pack without ...
 * @param  onEmpty  Default value if type parameter pack is empty
 */
#define ANY_OF_TYPE_PACK(TypePack, onEmpty, ...)  ANY_OF_TYPE_LIST(TTypes<TypePack...>, onEmpty, __VA_ARGS__)

/**
 * @brief
 * Apply a concept to all types in input type parameter pack, Any of them can pass. If a concept needs extra template
 * arguments you can provide them as variadic macro arguments.
 *
 * @param  typeList  input type parameter pack without ...
 * @param   onEmpty  Default value if type parameter pack is empty
 * @param conceptIn  The given concept to check for all items in the type parameter pack
 */
#define C_ANY_OF_TYPE_PACK(TypePack, onEmpty, conceptIn, ...) C_ANY_OF_TYPE_LIST(TTypes<TypePack...>, onEmpty, conceptIn, __VA_ARGS__)

	/**
	 * @brief
	 * Is given type-list contains all types convertible to the types of another type-list. The number and order of types
	 * are significant.
	 */
	template <typename From, typename To, bool OnEmpty = true>
	concept CTypesConvertibleTo =
		CTypeList<From>
		&& CTypeList<To>
		&& From::Count == To::Count
		&& Detail::IsTypeListConvertibleTo<From, To>(
			OnEmpty,
			std::make_index_sequence<From::Count>()
		)
	;

	/**
	 * @brief
	 * Is given type-list contains all types convertible to the types of another type-list. The number and order of types
	 * are significant. Qualifiers are not taken into account.
	 */
	template <typename From, typename To, bool OnEmpty = true>
	concept CTypesConvertibleToDecayed =
		CTypeList<From>
		&& CTypeList<To>
		&& From::Count == To::Count
		&& Detail::IsTypeListConvertibleToDecay<From, To>(
			OnEmpty,
			std::make_index_sequence<From::Count>()
		)
;

	/**
	 *	@brief  Base struct for matching templates disregarding their arguments
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <template <typename...> typename Template>
	struct TTemplate_Match
	{
		template <typename T>
		static constexpr bool Match = false;

		template <typename... Params>
		static constexpr bool Match<Template<Params...>> = true;
	};

	template <typename>
	struct TTemplate_Struct
	{
		static constexpr bool IsTemplate = false;
	};

	/**
	 * @brief  Base struct containing traits of specified template instance (which only accepts type parameters)
	 *
	 * @warning
	 * Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 * considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 * parameters even when a default is specified for them will result in compile error.
	 */
	template <template <typename...> typename Template, typename... Params>
	struct TTemplate_Struct<Template<Params...>>
	{
		static constexpr bool IsTemplate = true;

		static constexpr size_t ParameterCount = sizeof...(Params);
		using Parameters = TTypes<Params...>;
		using ParametersDecay = TTypes<std::decay_t<Params>...>;

		template <size_t I>
		using Param = TTypeAtPack<I, Params...>;

		template <size_t I>
		using ParamDecay = TTypes_Get<ParametersDecay, I>;
	};

	/**
	 * @brief  Checks if input is a template which only has type parameters
	 *
	 * @warning
	 * Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 * considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 * parameters even when a default is specified for them will result in compile error.
	 */
	template <typename T>
	concept CTypeOnlyTemplate = TTemplate_Struct<T>::IsTemplate;
	
	/**
	 *	@brief  Get template type parameters as a tuple
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <CTypeOnlyTemplate Instance>
	using TTemplate_Params = typename TTemplate_Struct<Instance>::Parameters;
	
	/**
	 *	@brief  Get decayed template type parameters as a tuple
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <CTypeOnlyTemplate Instance>
	using TTemplate_ParamsDecay = typename TTemplate_Struct<Instance>::ParametersDecay;

	/**
	 *	@brief  Get a type parameter at a specified position of a templated instance. 
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <CTypeOnlyTemplate Instance, int I>
	using TTemplate_Param = typename TTemplate_Struct<Instance>::template Param<I>;

	/**
	 *	@brief  Get a decayed type parameter at a specified position of a templated instance. 
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <CTypeOnlyTemplate Instance, int I>
	using TTemplate_ParamDecay = typename TTemplate_Struct<Instance>::template ParamDecay<I>;

	/**
	 *	@brief  Check if given type is an instantiation of a given template (which only accepts type parameters)
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <typename Instance, template <typename...> typename Template>
	concept CMatchTemplate =
		CTypeOnlyTemplate<std::decay_t<Instance>>
		&& TTemplate_Match<Template>::template Match<std::decay_t<Instance>>
	;

	/**
	 *	@brief
	 *	Get the number of template type parameters from a specified templated instance (which only has type parameters) 
	 *
	 *	@warning
	 *	Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 *	considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 *	parameters even when a default is specified for them will result in compile error.
	 */
	template <CTypeOnlyTemplate Instance>
	inline constexpr size_t TTemplate_ParamCount = TTemplate_Struct<Instance>::ParameterCount;

	template <template <typename...> typename, typename>
	struct TTemplateMap_Struct
	{
		using Type = void;
	};

	template <
		template <typename...> typename TemplateOut,
		template <typename...> typename TemplateIn,
		typename... Params
	>
	struct TTemplateMap_Struct<TemplateOut, TemplateIn<Params...>>
	{
		using Type = TemplateOut<Params...>;
	};

	/**
	 * @brief
	 * Transfer parameters from one template to another. Or in other words replace the template part of the input
	 * template instance with `TemplateOut`.
	 *
	 * @warning
	 * Until this proposal https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1985r0.pdf or equivalent is
	 * considered seriously, template traits only work with templates which only have type-parameters. Non-type
	 * parameters even when a default is specified for them will result in compile error.
	 */
	template <template <typename...> typename TemplateOut, CTypeOnlyTemplate FromInstance>
	using TTemplateMap = typename TTemplateMap_Struct<TemplateOut, FromInstance>::Type;

	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName>(...)`? use this instead */
	template <CConstType T>
	constexpr auto&& AsConst(T&& input) { return FWD(input); }
	
	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName>(...)`? use this instead */
	template <CMutableType T>
	constexpr auto&& AsConst(T&& input) { return FWD(const_cast<const T>(input)); }

	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName>(...)`? use this instead */
	template <CMutableType T>
	constexpr auto&& AsMutable(T&& input) { return FWD(input); }
	
	/** @brief Tired of typing `const_cast<FMyLongUnwieldyTypeName>(...)`? use this instead */
	template <CConstType T>
	constexpr auto&& AsMutable(T&& input) { return FWD(const_cast<T>(input)); }

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
