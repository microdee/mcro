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
#include "Mcro/Tuples.h"
#include "Mcro/Concepts.h"

namespace Mcro::FunctionTraits
{
	using namespace Mcro::Concepts;
	using namespace Mcro::Tuples;
	
	/** @brief Concept constraining input T to a lambda function or a functor object. */
	template <typename T>
	concept CFunctorObject = requires { &std::decay_t<T>::operator(); };

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
			using ArgumentsDecay = TTuple<std::decay_t<Args>...>;

			/** The pure function signature with other information stripped from it */
			using Signature = Return(Args...);

			template <int I>
			using Arg = TTypeAt<I, Arguments>;

			template <int I>
			using ArgDecay = TTypeAt<I, ArgumentsDecay>;
		};
	}
	
	/**
	 *	@brief
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
		
	/** @brief Specialization for functor structs / lambda functions. */
	template <CFunctorObject T>
	struct TFunctionTraits<T> : TFunctionTraits<decltype(&std::decay_t<T>::operator())>
	{
		static constexpr bool IsFunction = true;
		static constexpr bool IsPointer = false;
		static constexpr bool IsFunctor = true;
		static constexpr bool IsMember = false;
		static constexpr bool IsConst = false;
	};

	/** @brief Specialization extracting the types from the compound function pointer type of a const member function. */
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

	/** @brief Specialization extracting the types from the compound function pointer type of a member function. */
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

	/** @brief Specialization extracting the types from the compound function pointer type. */
	template <typename ReturnIn, typename... Args>
	struct TFunctionTraits<ReturnIn(*)(Args...)> : Detail::TFunctionMeta<ReturnIn, Args...>
	{
		static constexpr bool IsFunction = true;
		static constexpr bool IsPointer = true;
		static constexpr bool IsFunctor = false;
		static constexpr bool IsMember = false;
		static constexpr bool IsConst = false;
	};

	/** @brief Specialization extracting the types from the compound function type. */
	template <typename ReturnIn, typename... Args>
	struct TFunctionTraits<ReturnIn(Args...)> : Detail::TFunctionMeta<ReturnIn, Args...>
	{
		static constexpr bool IsFunction = true;
		static constexpr bool IsPointer = false;
		static constexpr bool IsFunctor = false;
		static constexpr bool IsMember = false;
		static constexpr bool IsConst = false;
	};

	/** @brief Shorthand for getting a tuple representing the function arguments. */
	template <typename T>
	using TFunction_Arguments = typename TFunctionTraits<std::decay_t<T>>::Arguments;

	/** @brief Shorthand for getting a tuple representing the decayed function arguments. */
	template <typename T>
	using TFunction_ArgumentsDecay = typename TFunctionTraits<std::decay_t<T>>::ArgumentsDecay;

	/** @brief Shorthand for getting a type of a function argument at given position I. */
	template <typename T, int I>
	using TFunction_Arg = typename TFunctionTraits<std::decay_t<T>>::template Arg<I>;

	/** @brief Shorthand for getting a decayed type of a function argument at given position I. */
	template <typename T, int I>
	using TFunction_ArgDecay = typename TFunctionTraits<std::decay_t<T>>::template ArgDecay<I>;

	/** @brief Shorthand for getting a function argument count. */
	template <typename T>
	inline constexpr size_t TFunction_ArgCount = TFunctionTraits<std::decay_t<T>>::ArgumentCount;

	/** @brief Shorthand for getting a function return type. */
	template <typename T>
	using TFunction_Return = typename TFunctionTraits<std::decay_t<T>>::Return;

	/** @brief Shorthand for getting a function return type discarding qualifiers. */
	template <typename T>
	using TFunction_ReturnDecay = typename TFunctionTraits<std::decay_t<T>>::Return;

	/** @brief Shorthand for getting a pur function signature. */
	template <typename T>
	using TFunction_Signature = typename TFunctionTraits<std::decay_t<T>>::Signature;

	template <typename T>
	concept CFunction_IsMember = TFunctionTraits<std::decay_t<T>>::IsMember;

	/** @brief Shorthand for getting the class of a member function. */
	template <CFunction_IsMember T>
	using TFunction_Class = typename TFunctionTraits<std::decay_t<T>>::Class;

	/** @brief Shorthand for getting the constness of a member function. */
	template <typename T>
	concept CFunction_IsConst = TFunctionTraits<std::decay_t<T>>::IsConst;
	
	/** @brief A concept accepting any function like entity (function pointer or functor object) */
	template <typename T>
	concept CFunctionLike = TFunctionTraits<std::decay_t<T>>::IsFunction;

	/** @brief A concept accepting function pointer types */
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

	/** @brief Compose a function type from a tuple of arguments and a return type */
	template <typename Return, typename Tuple>
	using TFunctionFromTuple = typename Detail::TFunctionFromTuple_Struct<Return, std::decay_t<Tuple>>::Type;

	/** @brief Override the return type of an input function signature */
	template <typename Return, typename DstFunction>
	using TSetReturn = TFunctionFromTuple<Return, TFunction_Arguments<DstFunction>>;

	/** @brief Override the return type of an input function signature, and discard its qualifiers */
	template <typename Return, typename DstFunction>
	using TSetReturnDecay = TFunctionFromTuple<std::decay_t<Return>, TFunction_Arguments<DstFunction>>;

	/** @brief Copy the return type from source function signature to the destination one */
	template <typename SrcFunction, typename DstFunction>
	using TCopyReturn = TSetReturn<TFunction_Return<SrcFunction>, DstFunction>;

	/** @brief Copy the return type from source function signature to the destination one, and discard its qualifiers */
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
			return function(arguments.template Get<Sequence>()...);
		}
		
		template<typename Object, typename Function, size_t... Sequence>
		TFunction_Return<Function> InvokeWithTuple_Impl(
			Object* object,
			Function&& function,
			TFunction_Arguments<Function> const& arguments,
			std::index_sequence<Sequence...>&&
		)
		{
			return (object->*function)(arguments.template Get<Sequence>()...);
		}
	}

	/**
	 *	@brief
	 *	A clone of std::apply for Unreal tuples which also supports function pointers.
	 *	TL;DR: It calls a function with arguments supplied from a tuple.
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
	 *	@brief
	 *	A clone of std::apply for Unreal tuples which also supports function pointers. This overload can bind an object
	 *	TL;DR: It calls a function with arguments supplied from a tuple.
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

	/** @brief Concept matching the return of a type with compatible return types, disregarding CV-ref qualifiers. */
	template <typename F, typename Return>
	concept CFunctionCompatible_ReturnDecay =
		CFunctionLike<F>
		&& CConvertibleToDecayed<TFunction_ReturnDecay<F>, Return>
	;
	
	/** @brief Concept matching the return of a type with compatible return types, preserving CV-ref qualifiers. */
	template <typename F, typename Return>
	concept CFunctionCompatible_Return =
		CFunctionLike<F>
		&& CConvertibleTo<TFunction_Return<F>, Return>
	;

	/** @brief Concept matching function types with compatible set of arguments, disregarding CV-ref qualifiers. */
	template <typename F, typename With>
	concept CFunctionCompatible_ArgumentsDecay =
		CFunctionLike<F>
		&& CFunctionLike<With>
		&& CConvertibleToDecayed<
			TFunction_ArgumentsDecay<With>,
			TFunction_ArgumentsDecay<F>
		>
	;

	/** @brief Concept matching function types with compatible set of arguments, preserving CV-ref qualifiers. */
	template <typename F, typename With>
	concept CFunctionCompatible_Arguments =
		CFunctionLike<F>
		&& CFunctionLike<With>
		&& CConvertibleTo<
			TFunction_Arguments<With>,
			TFunction_Arguments<F>
		>
	;

	/**
	 *	@brief
	 *	Concept constraining a function type to another one which arguments and return types are compatible,
	 *	disregarding CV-ref qualifiers
	 */
	template <typename F, typename With>
	concept CFunctionCompatibleDecay =
		CFunctionLike<F>
		&& CFunctionLike<With>
		&& CFunctionCompatible_ReturnDecay<F, TFunction_ReturnDecay<With>>
		&& CFunctionCompatible_ArgumentsDecay<F, With>
	;

	/**
	 *	@brief
	 *	Concept constraining a function type to another one which arguments and return types are compatible,
	 *	preserving CV-ref qualifiers
	 */
	template <typename F, typename With>
	concept CFunctionCompatible =
		CFunctionLike<F>
		&& CFunctionLike<With>
		&& CFunctionCompatible_Return<F, TFunction_Return<With>>
		&& CFunctionCompatible_Arguments<F, With>
	;

	/** @brief Concept matching function types returning void. */
	template <typename F>
	concept CFunctionReturnsVoid = CFunctionLike<F> && std::is_void_v<TFunction_Return<F>>;

	/**
	 *	@brief
	 *	Tests if a provided class member function pointer instance (not type!) is indeed an instance member method.
	 *	Negating it can assume static class member function 
	 */
	template <auto FuncPtr>
	concept CInstanceMethod = requires(
		TFunction_Class<decltype(FuncPtr)>* instance,
		TFunction_Arguments<decltype(FuncPtr)> argsTuple
	) {
		InvokeWithTuple(instance, FuncPtr, argsTuple);
	};

	/**
	 *	@brief
	 *	Defers a set of arguments for a function call later with its first argument. This is useful for developing
	 *	fluent API operators.
	 */
	template <auto FuncPtr, CFunctionPtr Function = decltype(FuncPtr)>
	requires (
		CFunctionPtr<decltype(FuncPtr)>
		&& TFunction_ArgCount<decltype(FuncPtr)> > 0
	)
	struct TDeferFunctionArguments
	{
		using FirstArg = TFunction_Arg<Function, 0>;
		using ExtraArgs = TSkip<1, TFunction_Arguments<Function>>;
		using Return = TFunction_Return<Function>;

		template <typename... Args>
		TDeferFunctionArguments(Args... args)
			: Storage(Forward<Args>(args)...)
		{}

		template <CConvertibleToDecayed<FirstArg> FirstArgRef>
		Return operator () (FirstArgRef&& arg)
		{
			auto args = arg >> Storage;
			return InvokeWithTuple<Function>(FuncPtr, args);
		}

	private:
		ExtraArgs Storage;
	};
}
