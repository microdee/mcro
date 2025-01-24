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
#include "Mcro/FunctionTraits.h"
#include "Mcro/Delegates/EventDelegate.h"

/**
 *	@brief
 *	Glue together a regular function, a native event delegate and a dynamic multicast delegate, so a given API
 *	can serve
 *	- events via function override
 *	- C++ listeners via a native event delegate
 *	- Blueprint listeners via a dynamic multicast delegate
 *	with calling only a single class member.
 *
 *	@param functionName  The base name of the function which can be listened to with multiplexing
 */
#define MCRO_DYNAMIC_EVENT_MULTIPLEX(functionName) \
	Mcro::Delegates::TNativeMultiplex<F##functionName> functionName##Multiplex \
	{ \
		Mcro::Delegates::InferDelegate::From(this, &MCRO_THIS_TYPE::functionName), \
		Mcro::Delegates::InferDelegate::From(functionName##Event) \
	} \

/**
 *	In case the multiplexed function returns a value instead of just returning void, add this macro below the dynamic
 *	multicast delegate declaration
 *	
 *	@param delegate  The name of the dynamic multicast delegate
 *	@param type      The return type of the multiplexed function
 */
#define MCRO_DYNAMIC_RETURN(delegate, type) \
	template<> struct Mcro::Delegates::TReturnOverride_Struct<delegate> { using Type = type; }

namespace Mcro::Delegates
{
	using namespace Mcro::Concepts;

	/**
	 *	@brief
	 *	Glue together a regular function, a native event delegate and a dynamic multicast delegate, so a given API
	 *	can serve
	 *	- events via function override
	 *	- C++ listeners via a native event delegate
	 *	- Blueprint listeners via a dynamic multicast delegate
	 *	with calling only a single class member.
	 *
	 *	@important
	 *	This is set up via the MCRO_DYNAMIC_EVENT_MULTIPLEX macro next to relevant class members.
	 *	Using this template directly is not recommended
	 */
	template <typename T>
	class TEventMultiplex {};

	/** @copydoc TEventMultiplex */
	template <typename Return, typename... Args>
	class TEventMultiplex<Return(Args...)>
	{
	public:
		using FunctionSignature = Return(Args...);
		using EventSignature = void(Args...);
		using FDelegate = TDelegate<FunctionSignature, FDefaultDelegateUserPolicy>;
		using FVoidDelegate = TDelegate<EventSignature>;
		using FEventDelegate = TEventDelegate<EventSignature>;
		
		FEventDelegate Multicast;
		
	protected:
		FDelegate Function;
		
	public:
		template <CConvertibleToDecayed<FVoidDelegate>... Delegates>
		TEventMultiplex(FDelegate const& function, Delegates&&... delegates)
			: Multicast(Forward<Delegates>(delegates)...)
			, Function(function)
		{}

		template <CNonVoid = Return>
		Return operator () (Args&&... args)
		{
			Return result = Function.Execute(Forward<Args>(args)...);
			Multicast.Broadcast(Forward<Args>(args)...);
			return result;
		}

		template <CVoid = Return>
		void operator () (Args&&... args)
		{
			Function.Execute(Forward<Args>(args)...);
			Multicast.Broadcast(Forward<Args>(args)...);
		}

		FEventDelegate* operator -> () const
		{
			return &Multicast;
		}
		
		template <typename... OptionalObject> requires (sizeof...(OptionalObject) <= 1)
		FDelegate Delegation(OptionalObject&&... object)
		{
			return InferDelegate::From(Forward<OptionalObject>(object)..., &TEventMultiplex::operator()<Return>);
		};
	};

	template <CDynamicMulticastDelegate Dynamic>
	struct TReturnOverride_Struct
	{
		using Type = void;
	};

	template <typename Dynamic>
	using TReturnOverride = typename TReturnOverride_Struct<Dynamic>::Type;

	template <
		CDynamicMulticastDelegate Dynamic,
		typename Return = TReturnOverride<Dynamic>
	>
	struct TNativeMultiplex_Struct
	{
		using Type = TEventMultiplex<
			TSetReturnDecay<Return, TDynamicSignature<Dynamic>>
		>;
	};
	
	template <
		typename Dynamic,
		typename Return = TReturnOverride<Dynamic>
	>
	using TNativeMultiplex = typename TNativeMultiplex_Struct<Dynamic, Return>::Type;
}
