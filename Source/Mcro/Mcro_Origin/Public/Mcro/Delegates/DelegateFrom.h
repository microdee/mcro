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

/**
 *	@brief The extra layer of namespace `Infer` is there for guarding common vocabulary (From) but still allowing the
 *	developer to use this namespace for a more terse syntax.
 */
namespace Mcro::Delegates::InferDelegate
{
	using namespace Mcro::FunctionTraits;
	using namespace Mcro::Tuples;

	/**
	 *	@brief
	 *	Instead of specifying manually a delegate type, infer it from the input function and the extra captures.
	 *	
	 *	This overload is equivalent to `TDelegate::CreateStatic`
	 */
	template <CFunctionLike Function, typename... Captures>
	requires (!CFunction_IsMember<Function>)
		&& (!CFunctorObject<Function>)
	TInferredDelegate<Function, Captures...> From(Function func, Captures&&... captures)
	{
		return TInferredDelegate<Function, Captures...>::CreateStatic(func, Forward<Captures>(captures)...);
	}

	/**
	 *	@brief
	 *	Instead of specifying manually a delegate type, infer it from the input function and the extra captures.
	 *	
	 *	This overload is equivalent to `TDelegate::CreateLambda`
	 */
	template <CFunctorObject Function>
	TDelegate<TFunction_Signature<Function>> From(Function&& func)
	{
		return TDelegate<TFunction_Signature<Function>>::CreateLambda(Forward<Function>(func));
	}
	
	/**
	 *	@brief
	 *	Instead of specifying manually a delegate type, infer it from the input function and the extra captures. Also
	 *	infer the method of object binding based on the type traits of the object argument.
	 *	
	 *	This overload is equivalent to `TDelegate::CreateSPLambda` (TSharedRef)
	 */
	template <CSharedRef Object, CFunctorObject Function>
	TDelegate<TFunction_Signature<Function>> From(Object const& self, Function&& func)
	{
		return TDelegate<TFunction_Signature<Function>>::CreateSPLambda(self, Forward<Function>(func));
	}
	
	/**
	 *	@brief
	 *	Instead of specifying manually a delegate type, infer it from the input function and the extra captures. Also
	 *	infer the method of object binding based on the type traits of the object argument.
	 *	
	 *	This overload is equivalent to `TDelegate::CreateSPLambda` (TSharedFromThis pointer)
	 */
	template <CSharedFromThis Object, CFunctorObject Function>
	TDelegate<TFunction_Signature<Function>> From(Object* self, Function&& func)
	{
		return TDelegate<TFunction_Signature<Function>>::CreateSPLambda(self, Forward<Function>(func));
	}
	
	/**
	 *	@brief
	 *	Instead of specifying manually a delegate type, infer it from the input function and the extra captures. Also
	 *	infer the method of object binding based on the type traits of the object argument.
	 *	
	 *	This overload is equivalent to `TDelegate::CreateSPLambda` (TSharedFromThis const pointer)
	 */
	template <CSharedFromThis Object, CFunctorObject Function>
	TDelegate<TFunction_Signature<Function>> From(const Object* self, Function&& func)
	{
		return TDelegate<TFunction_Signature<Function>>::CreateSPLambda(self, Forward<Function>(func));
	}
	
	/**
	 *	@brief
	 *	Instead of specifying manually a delegate type, infer it from the input function and the extra captures. Also
	 *	infer the method of object binding based on the type traits of the object argument.
	 *	
	 *	This overload is equivalent to `TDelegate::CreateWeakLambda` (plain C++ object pointer)
	 */
	template <CPlainClass Object, CFunctorObject Function>
	TDelegate<TFunction_Signature<Function>> From(Object* self, Function&& func)
	{
		return TDelegate<TFunction_Signature<Function>>::CreateWeakLambda(self, Forward<Function>(func));
	}
	
	/**
	 *	@brief
	 *	Instead of specifying manually a delegate type, infer it from the input function and the extra captures. Also
	 *	infer the method of object binding based on the type traits of the object argument.
	 *	
	 *	This overload is equivalent to `TDelegate::CreateRaw` (plain C++ object pointer)
	 */
	template <CPlainClass Object, CFunctionPtr Function, typename... Captures>
	requires CFunction_IsMember<Function> && (!CFunctorObject<Function>)
	TInferredDelegate<Function, Captures...> From(Object* self, Function func, const Captures&... captures)
	{
		return TInferredDelegate<Function, Captures...>::CreateRaw(self, func, captures...);
	}

	/**
	 *	@brief
	 *	Instead of specifying manually a delegate type, infer it from the input function and the extra captures. Also
	 *	infer the method of object binding based on the type traits of the object argument.
	 *	
	 *	This overload is equivalent to `TDelegate::CreateRaw` (plain C++ const object pointer)
	 */
	template <CPlainClass Object, CFunctionPtr Function, typename... Captures>
	requires CFunction_IsMember<Function> && (!CFunctorObject<Function>)
	TInferredDelegate<Function, Captures...> From(const Object* self, Function func, const Captures&... captures)
	{
		return TInferredDelegate<Function, Captures...>::CreateRaw(self, func, captures...);
	}

	/**
	 *	@brief
	 *	Instead of specifying manually a delegate type, infer it from the input function and the extra captures. Also
	 *	infer the method of object binding based on the type traits of the object argument.
	 *	
	 *	This overload is equivalent to `TDelegate::CreateSP` (TSharedRef)
	 */
	template <CSharedRef Object, CFunctionPtr Function, typename... Captures>
	requires CFunction_IsMember<Function> && (!CFunctorObject<Function>)
	TInferredDelegate<Function, Captures...> From(const Object& self, Function func, const Captures&... captures)
	{
		return TInferredDelegate<Function, Captures...>::CreateSP(self, func, captures...);
	}

	/**
	 *	@brief
	 *	Instead of specifying manually a delegate type, infer it from the input function and the extra captures. Also
	 *	infer the method of object binding based on the type traits of the object argument.
	 *	
	 *	This overload is equivalent to `TDelegate::CreateSP` (TSharedFromThis pointer)
	 */
	template <CSharedFromThis Object, CFunctionPtr Function, typename... Captures>
	requires CFunction_IsMember<Function> && (!CFunctorObject<Function>)
	TInferredDelegate<Function, Captures...> From(Object* self, Function func, const Captures&... captures)
	{
		return TInferredDelegate<Function, Captures...>::CreateSP(self, func, captures...);
	}

	/**
	 *	@brief
	 *	Instead of specifying manually a delegate type, infer it from the input function and the extra captures. Also
	 *	infer the method of object binding based on the type traits of the object argument.
	 *	
	 *	This overload is equivalent to `TDelegate::CreateSP` (TSharedFromThis const pointer)
	 */
	template <CSharedFromThis Object, CFunctionPtr Function, typename... Captures>
	requires CFunction_IsMember<Function> && (!CFunctorObject<Function>)
	TInferredDelegate<Function, Captures...> From(const Object* self, Function func, const Captures&... captures)
	{
		return TInferredDelegate<Function, Captures...>::CreateSP(self, func, captures...);
	}

	/**
	 *	@brief
	 *	Instead of specifying manually a delegate type, infer it from the input function and the extra captures. Also
	 *	infer the method of object binding based on the type traits of the object argument.
	 *	
	 *	This overload is equivalent to `TDelegate::CreateUObject` (UObject pointer)
	 */
	template <CUObject Object, CFunctionPtr Function, typename... Captures>
	requires CFunction_IsMember<Function>
		&& (!CFunctorObject<Function>)
	TInferredDelegate<Function, Captures...> From(Object* self, Function func, const Captures&... captures)
	{
		return TInferredDelegate<Function, Captures...>::CreateUObject(self, func, captures...);
	}
	
	/**
	 *	@brief
	 *	Instead of specifying manually a delegate type, infer it from the input function and the extra captures. Also
	 *	infer the method of object binding based on the type traits of the object argument.
	 *	
	 *	This overload is equivalent to `TDelegate::CreateUObject` (UObject const pointer)
	 */
	template <CUObject Object, CFunctionPtr Function, typename... Captures>
	requires CFunction_IsMember<Function>
		&& (!CFunctorObject<Function>)
	TInferredDelegate<Function, Captures...> From(const Object* self, Function func, const Captures&... captures)
	{
		return TInferredDelegate<Function, Captures...>::CreateUObject(self, func, captures...);
	}
	
	/**
	 *	@brief
	 *	Instead of specifying manually a delegate type, infer it from the input function and the extra captures. Also
	 *	infer the method of object binding based on the type traits of the object argument.
	 *	
	 *	This overload is equivalent to `TDelegate::CreateUObject` (TObjectPtr)
	 */
	template <CUObject Object, CFunctionPtr Function, typename... Captures>
	requires CFunction_IsMember<Function>
		&& (!CFunctorObject<Function>)
	TInferredDelegate<Function, Captures...> From(TObjectPtr<Object> self, Function func, const Captures&... captures)
	{
		return TInferredDelegate<Function, Captures...>::CreateUObject(self, func, captures...);
	}

	/**
	 *	@brief  Broadcast a multicast delegate when the returned delegate is executed.
	 *	@param  multicast  input multicast delegate
	 *	
	 *	@todo   Captures... and bound object
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
	 *	@brief  Broadcast a multicast delegate when the returned delegate is executed with a binding object.
	 *	@param  self       any type of binding object other `From` overloads accept
	 *	@param  multicast  input multicast delegate
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
	 *	@brief  Broadcast a dynamic multicast delegate when the returned delegate is executed.
	 *	@tparam Dynamic    The type of the input dynamic multicast delegate
	 *	@param  multicast  input dynamic multicast delegate
	 *	
	 *	@todo   Captures... and bound object
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
	 *	@brief  Broadcast a dynamic multicast delegate when the returned delegate is executed.
	 *	@tparam Dynamic    The type of the input dynamic multicast delegate
	 *	@param  self       any type of binding object other `From` overloads accept
	 *	@param  multicast  input dynamic multicast delegate
	 *	
	 *	@todo   Captures... and bound object
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
