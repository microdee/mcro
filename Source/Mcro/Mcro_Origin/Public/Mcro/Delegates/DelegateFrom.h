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

/**
 *	@file
 *	Unreal delegates while being great they have the problem that they're pretty verbose to use, as the usage site
 *	requires the developer to spell out the delegate types when they're being bound to something. `Mcro::Delegate::From`
 *	overloads not only infer delegate types from input function, but they also infer how the delegate is being used.
 *	For example take `From(this, &FStuff::MyFunc)` can map to several classic delegate usages depending on the type of
 *	`this`:
 *	- CreateSP if `this` is TSharedFromThis 
 *	- CreateUObject if `this` is a UObject
 *	- CreateRaw in case `this` is just a plain old C++ object
 *	@file
 *	`From` can also deal with
 *	- usages of Lambda functions with same API
 *	- correct delegate type from combination of given function signature and given captures
 *	  - So `From(this, &FStuff::MyFunc, TEXT("my capture")` will correctly remove the last argument from the function
 *	    signature of `FStuff::MyFunc` when inferring the delegate type.
 *	- Chain multicast delegates together
 */

#include "CoreMinimal.h"
#include "Mcro/FunctionTraits.h"
#include "Mcro/Delegates/Traits.h"
#include "Mcro/Tuples.h"

namespace Mcro::Delegates
{
	using namespace Mcro::FunctionTraits;
	using namespace Mcro::Tuples;
	
	template <CFunctionLike Function, typename... Captures>
	using TInferredDelegate = TDelegate<
			TFunctionFromTuple<
				TFunction_Return<Function>,
				TTrimEnd<sizeof...(Captures), typename TFunction<Function>::Arguments>
			>,
			FDefaultDelegateUserPolicy
		>
	;

	/** Maps to CreateStatic */
	template <CFunctionLike Function, typename... Captures>
	requires (!CFunction_IsMember<Function>)
		&& (!CFunctorObject<Function>)
	TInferredDelegate<Function, Captures...> From(Function func, const Captures&... captures)
	{
		return TInferredDelegate<Function, Captures...>::CreateStatic(func, captures...);
	}

	/** Maps to CreateLambda */
	template <CFunctorObject Function>
	TDelegate<TFunction_Signature<Function>> From(Function&& func)
	{
		return TDelegate<TFunction_Signature<Function>>::CreateLambda(MoveTemp(func));
	}
	
	/** Maps to CreateSPLambda */
	template <CSharedRef Object, CFunctorObject Function>
	TDelegate<TFunction_Signature<Function>> From(const Object& self, Function&& func)
	{
		return TDelegate<TFunction_Signature<Function>>::CreateSPLambda(self, MoveTemp(func));
	}
	
	/** Maps to CreateSPLambda (SharedFromThis) */
	template <CSharedFromThis Object, CFunctorObject Function>
	TDelegate<TFunction_Signature<Function>> From(Object* self, Function&& func)
	{
		return TDelegate<TFunction_Signature<Function>>::CreateSPLambda(self, MoveTemp(func));
	}
	
	/** Maps to CreateSPLambda (SharedFromThis) */
	template <CSharedFromThis Object, CFunctorObject Function>
	TDelegate<TFunction_Signature<Function>> From(const Object* self, Function&& func)
	{
		return TDelegate<TFunction_Signature<Function>>::CreateSPLambda(self, MoveTemp(func));
	}
	
	/** Maps to CreateWeakLambda */
	template <CPlainClass Object, CFunctorObject Function>
	TDelegate<TFunction_Signature<Function>> From(Object* self, Function&& func)
	{
		return TDelegate<TFunction_Signature<Function>>::CreateWeakLambda(self, MoveTemp(func));
	}

	/** Maps to CreateRaw */
	template <CPlainClass Object, CFunctionLike Function, typename... Captures>
	requires CFunction_IsMember<Function> && (!CFunctorObject<Function>)
	TInferredDelegate<Function, Captures...> From(Object* self, Function func, const Captures&... captures)
	{
		return TInferredDelegate<Function, Captures...>::CreateRaw(self, func, captures...);
	}

	/** Maps to CreateRaw */
	template <CPlainClass Object, CFunctionLike Function, typename... Captures>
	requires CFunction_IsMember<Function> && (!CFunctorObject<Function>)
	TInferredDelegate<Function, Captures...> From(const Object* self, Function func, const Captures&... captures)
	{
		return TInferredDelegate<Function, Captures...>::CreateRaw(self, func, captures...);
	}

	/** Maps to CreateSP */
	template <CSharedRef Object, CFunctionLike Function, typename... Captures>
	requires CFunction_IsMember<Function> && (!CFunctorObject<Function>)
	TInferredDelegate<Function, Captures...> From(const Object& self, Function func, const Captures&... captures)
	{
		return TInferredDelegate<Function, Captures...>::CreateSP(self, func, captures...);
	}

	/** Maps to CreateSP (SharedFromThis) */
	template <CSharedFromThis Object, CFunctionLike Function, typename... Captures>
	requires CFunction_IsMember<Function> && (!CFunctorObject<Function>)
	TInferredDelegate<Function, Captures...> From(Object* self, Function func, const Captures&... captures)
	{
		return TInferredDelegate<Function, Captures...>::CreateSP(self, func, captures...);
	}

	/** Maps to CreateSP (SharedFromThis) */
	template <CSharedFromThis Object, CFunctionLike Function, typename... Captures>
	requires CFunction_IsMember<Function> && (!CFunctorObject<Function>)
	TInferredDelegate<Function, Captures...> From(const Object* self, Function func, const Captures&... captures)
	{
		return TInferredDelegate<Function, Captures...>::CreateSP(self, func, captures...);
	}

	/** Maps to CreateUObject */
	template <CUObject Object, CFunctionLike Function, typename... Captures>
	requires CFunction_IsMember<Function>
		&& (!CFunctorObject<Function>)
	TInferredDelegate<Function, Captures...> From(Object* self, Function func, const Captures&... captures)
	{
		return TInferredDelegate<Function, Captures...>::CreateUObject(self, func, captures...);
	}
	
	/** Maps to CreateUObject */
	template <CUObject Object, CFunctionLike Function, typename... Captures>
	requires CFunction_IsMember<Function>
		&& (!CFunctorObject<Function>)
	TInferredDelegate<Function, Captures...> From(const Object* self, Function func, const Captures&... captures)
	{
		return TInferredDelegate<Function, Captures...>::CreateUObject(self, func, captures...);
	}

	/**
	 *	Broadcast a multicast delegate when the returned delegate is executed.
	 *	@param multicast  input multicast delegate
	 *	
	 *	@todo  Captures... and bound object
	 */
	template <typename... Args>
	TDelegate<void(Args...)> From(TMulticastDelegate<void(Args...)>& multicast)
	{
		return From([&](Args... args)
		{
			multicast.Broadcast(args...);
		});
	}

	/**
	 *	Broadcast a multicast delegate when the returned delegate is executed with a binding object.
	 *	@param self       any type of binding object other `From` overloads accept
	 *	@param multicast  input multicast delegate
	 *	
	 *	@todo  Captures... and bound object
	 */
	template <typename Object, typename... Args>
	TDelegate<void(Args...)> From(Object&& self, TMulticastDelegate<void(Args...)>& multicast)
	{
		return From(Forward<Object>(self), [&](Args... args)
		{
			multicast.Broadcast(args...);
		});
	}

	namespace Detail
	{
		template <CDynamicMulticastDelegate Dynamic, size_t... ArgIndices>
		TNative<typename Dynamic::FDelegate> FromDynamicMulticastDelegate(Dynamic& multicast, std::index_sequence<ArgIndices...>&&)
		{
			return From([&](TFunction_Arg<TDynamicMethodPtr<Dynamic>, ArgIndices>... args)
			{
				multicast.Broadcast(args...);
			});
		}

		template <typename Object, CDynamicMulticastDelegate Dynamic, size_t... ArgIndices>
		TNative<typename Dynamic::FDelegate> FromDynamicMulticastDelegate(Object&& self, Dynamic& multicast, std::index_sequence<ArgIndices...>&&)
		{
			return From(Forward<Object>(self), [&](TFunction_Arg<TDynamicMethodPtr<Dynamic>, ArgIndices>... args)
			{
				multicast.Broadcast(args...);
			});
		}
	}

	/**
	 *	Broadcast a dynamic multicast delegate when the returned delegate is executed.
	 *	@tparam Dynamic   The type of the input dynamic multicast delegate
	 *	@param multicast  input dynamic multicast delegate
	 *	
	 *	@todo  Captures... and bound object
	 */
	template <CDynamicMulticastDelegate Dynamic>
	TNative<typename Dynamic::FDelegate> From(Dynamic& multicast)
	{
		return Detail::FromDynamicMulticastDelegate(
			multicast,
			std::make_index_sequence<
				TFunction_ArgCount<TDynamicMethodPtr<Dynamic>>
			>{}
		);
	}
	

	/**
	 *	Broadcast a dynamic multicast delegate when the returned delegate is executed.
	 *	@tparam Dynamic   The type of the input dynamic multicast delegate
	 *	@param self       any type of binding object other `From` overloads accept
	 *	@param multicast  input dynamic multicast delegate
	 *	
	 *	@todo  Captures... and bound object
	 */
	template <typename Object, CDynamicMulticastDelegate Dynamic>
	TNative<typename Dynamic::FDelegate> From(Object&& self, Dynamic& multicast)
	{
		return Detail::FromDynamicMulticastDelegate(
			Forward<Object>(self), multicast,
			std::make_index_sequence<
				TFunction_ArgCount<TDynamicMethodPtr<Dynamic>>
			>{}
		);
	}
}
