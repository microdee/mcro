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

#include <utility>

#include "CoreMinimal.h"
#include "Mcro/Concepts.h"

namespace Mcro::FunctionTraits
{
	using namespace Mcro::Concepts;
	
	/** Concept constraining input T to a lambda function or a functor object. */
	template <typename T>
	concept CFunctorObject = requires { &std::decay_t<T>::operator(); };

	/** @internal */
	namespace Detail
	{
		template <typename ReturnIn, typename... Args>
		struct TFunctionMeta
		{
			static constexpr size_t ArgumentCount = sizeof...(Args);

			using Return = ReturnIn;
			using ReturnDecay = std::decay_t<ReturnIn>;

			/** The input parameters of the function as a tuple type. Types are not decayed. */
			using Arguments = TTuple<Args...>;

			/** The input parameters of the function as a tuple type. Types are decayed (useful for storage) */
			using ArgumentsDecay = TTuple<typename TDecay<Args>::Type...>;

			/** The pure function signature with other information stripped from it */
			using Signature = Return(Args...);

			template <int I>
			using Arg = typename TTupleElement<I, Arguments>::Type;

			template <int I>
			using ArgDecay = typename TTupleElement<I, ArgumentsDecay>::Type;
		};
	}
	
	/**
	 *	Get signature information about any function declaring type (function pointer or functor
	 *	structs including lambda functions). It should be used in other templates.
	 *	
	 *	@tparam T  the inferred type of the input function. 99% of cases this should be inferred.
	 */
	template <typename T>
	struct TFunctionTraits
	{
		static constexpr size_t ArgumentCount = 0;
		static constexpr bool IsFunction = false;
		static constexpr bool IsPointer = false;
		static constexpr bool IsFunctor = false;
		static constexpr bool IsMember = false;
		static constexpr bool IsConst = false;
	};
		
	/** Specialization for functor structs / lambda functions. */
	template <CFunctorObject T>
	struct TFunctionTraits<T> : TFunctionTraits<decltype(&std::decay_t<T>::operator())>
	{
		static constexpr bool IsFunction = true;
		static constexpr bool IsPointer = false;
		static constexpr bool IsFunctor = true;
		static constexpr bool IsMember = false;
		static constexpr bool IsConst = false;
	};

	/** Specialization extracting the types from the compound function pointer type of a const member function. */
	template <typename ClassIn, typename ReturnIn, typename... Args>
	struct TFunctionTraits<ReturnIn(ClassIn::*)(Args...) const> : Detail::TFunctionMeta<ReturnIn, Args...>
	{
		using Class = ClassIn;
		static constexpr bool IsFunction = true;
		static constexpr bool IsPointer = true;
		static constexpr bool IsFunctor = false;
		static constexpr bool IsMember = true;
		static constexpr bool IsConst = true;
	};

	/** Specialization extracting the types from the compound function pointer type of a member function. */
	template <typename ClassIn, typename ReturnIn, typename... Args>
	struct TFunctionTraits<ReturnIn(ClassIn::*)(Args...)> : Detail::TFunctionMeta<ReturnIn, Args...>
	{
		using Class = ClassIn;
		static constexpr bool IsFunction = true;
		static constexpr bool IsPointer = true;
		static constexpr bool IsFunctor = false;
		static constexpr bool IsMember = true;
		static constexpr bool IsConst = false;
	};

	/** Specialization extracting the types from the compound function pointer type. */
	template <typename ReturnIn, typename... Args>
	struct TFunctionTraits<ReturnIn(*)(Args...)> : Detail::TFunctionMeta<ReturnIn, Args...>
	{
		static constexpr bool IsFunction = true;
		static constexpr bool IsPointer = true;
		static constexpr bool IsFunctor = false;
		static constexpr bool IsMember = false;
		static constexpr bool IsConst = false;
	};

	/** Specialization extracting the types from the compound function type. */
	template <typename ReturnIn, typename... Args>
	struct TFunctionTraits<ReturnIn(Args...)> : Detail::TFunctionMeta<ReturnIn, Args...>
	{
		static constexpr bool IsFunction = true;
		static constexpr bool IsPointer = false;
		static constexpr bool IsFunctor = false;
		static constexpr bool IsMember = false;
		static constexpr bool IsConst = false;
	};

	/** Shorthand for getting a tuple representing the function arguments. */
	template <typename T>
	using TFunction_Arguments = typename TFunctionTraits<std::decay_t<T>>::Arguments;

	/** Shorthand for getting a tuple representing the decayed function arguments. */
	template <typename T>
	using TFunction_ArgumentsDecay = typename TFunctionTraits<std::decay_t<T>>::ArgumentsDecay;

	/** Shorthand for getting a type of a function argument at given position I. */
	template <typename T, int I>
	using TFunction_Arg = typename TFunctionTraits<std::decay_t<T>>::template Arg<I>;

	/** Shorthand for getting a decayed type of a function argument at given position I. */
	template <typename T, int I>
	using TFunction_ArgDecay = typename TFunctionTraits<std::decay_t<T>>::template ArgDecay<I>;

	template <typename T>
	inline constexpr size_t TFunction_ArgCount = TFunctionTraits<std::decay_t<T>>::ArgumentCount;

	template <typename T>
	using TFunction_Return = typename TFunctionTraits<std::decay_t<T>>::Return;

	template <typename T>
	using TFunction_ReturnDecay = typename TFunctionTraits<std::decay_t<T>>::Return;

	template <typename T>
	using TFunction_Signature = typename TFunctionTraits<std::decay_t<T>>::Signature;

	template <typename T>
	using TFunction_Class = typename TFunctionTraits<std::decay_t<T>>::Class;

	template <typename T>
	concept CFunction_IsMember = TFunctionTraits<std::decay_t<T>>::IsMember;

	template <typename T>
	concept CFunction_IsConst = TFunctionTraits<std::decay_t<T>>::IsConst;
	
	/** A concept accepting any function like entity (function pointer or functor object) */
	template <typename T>
	concept CFunctionLike = TFunctionTraits<std::decay_t<T>>::IsFunction;

	template <typename T>
	concept CFunctionPtr = TFunctionTraits<std::decay_t<T>>::IsPointer;

	template <typename Class, typename Function>
	concept CHasFunction = CFunction_IsMember<Function>
		&& (CDerivedFrom<Class, TFunction_Class<Function>> || CSameAs<Class, TFunction_Class<Function>>)
	;

	namespace Detail
	{
		template <typename Return, typename Tuple, size_t... Indices>
		using TFunctionFromTupleIndices = Return(typename TTupleElement<Indices, Tuple>::Type...);

		template <typename Return, typename Tuple>
		struct TFunctionFromTuple_Struct
		{
			template <size_t... Indices>
			static consteval TFunctionFromTupleIndices<Return, Tuple, Indices...>* Compose(std::index_sequence<Indices...>&&);

			using Type = std::remove_pointer_t<decltype(
				Compose(std::make_index_sequence<TTupleArity<Tuple>::Value>{})
			)>;
		};
	}

	template <typename Return, typename Tuple>
	using TFunctionFromTuple = typename Detail::TFunctionFromTuple_Struct<Return, std::decay_t<Tuple>>::Type;

	template <typename Return, typename DstFunction>
	using TSetReturn = TFunctionFromTuple<Return, TFunction_Arguments<DstFunction>>;

	template <typename Return, typename DstFunction>
	using TSetReturnDecay = TFunctionFromTuple<std::decay_t<Return>, TFunction_Arguments<DstFunction>>;

	template <typename SrcFunction, typename DstFunction>
	using TCopyReturn = TSetReturn<TFunction_Return<SrcFunction>, DstFunction>;

	template <typename SrcFunction, typename DstFunction>
	using TCopyReturnDecay = TSetReturnDecay<TFunction_ReturnDecay<SrcFunction>, DstFunction>;
	
	namespace Detail
	{
		template<typename Function, size_t... Sequence>
		TFunction_Return<Function> InvokeWithTuple_Impl(
			Function&& function,
			TFunction_Arguments<Function> const& arguments,
			std::index_sequence<Sequence...>&&
		)
		{
			return function(Forward<TFunction_Arg<Function, Sequence>>(arguments.template Get<Sequence>())...);
		}
		
		template<typename Object, typename Function, size_t... Sequence>
		TFunction_Return<Function> InvokeWithTuple_Impl(
			Object* object,
			Function&& function,
			TFunction_Arguments<Function> const& arguments,
			std::index_sequence<Sequence...>&&
		)
		{
			return (object->*function)(Forward<TFunction_Arg<Function, Sequence>>(arguments.template Get<Sequence>())...);
		}
	}

	/**
	 * A clone of std::apply for Unreal tuples which also supports function pointers.
	 * TL;DR: It calls a function with arguments supplied from a tuple.
	 */
	template<typename Function>
	TFunction_Return<Function> InvokeWithTuple(Function&& function, TFunction_Arguments<Function> const& arguments)
	{
		return Detail::InvokeWithTuple_Impl(
			Forward<Function>(function), arguments,
			std::make_index_sequence<TFunction_ArgCount<Function>>()
		);
	}

	/**
	 * A clone of std::apply for Unreal tuples which also supports function pointers. This overload can bind an object
	 * TL;DR: It calls a function with arguments supplied from a tuple.
	 */
	template<CFunctionPtr Function, CHasFunction<Function> Object>
	TFunction_Return<Function> InvokeWithTuple(Object* object, Function&& function, TFunction_Arguments<Function> const& arguments)
	{
		return Detail::InvokeWithTuple_Impl(
			object,
			Forward<Function>(function), arguments,
			std::make_index_sequence<TFunction_ArgCount<Function>>()
		);
	}
}
