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
		Mcro::Delegates::From(this, &MCRO_THIS_TYPE::functionName), \
		Mcro::Delegates::From(functionName##Event) \
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
	
	namespace Detail
	{
		template <typename Return, typename... Args>
		class TEventMultiplexBase
		{
		public:
			using FunctionSignature = Return(Args...);
			using EventSignature = void(Args...);
			using FDelegate = TDelegate<FunctionSignature, FDefaultDelegateUserPolicy>;
			using FEventDelegate = TDelegate<EventSignature, FDefaultDelegateUserPolicy>;

			template <CSameAs<FEventDelegate>... Delegates>
			TEventMultiplexBase(FDelegate&& function, Delegates&&... delegates)
				: Multicast(delegates...)
				, Function(function)
			{}

			TEventDelegate<void(Args...)> Multicast;

			TEventDelegate<void(Args...)>* operator -> () const
			{
				return &Multicast;
			}
		protected:
			FDelegate Function;
		};
	}

	template <typename T>
	class TEventMultiplex;

	/**
	 *	Glue together a regular function, a native event delegate and a dynamic multicast delegate, so a given API
	 *	can serve
	 *	- events via function override
	 *	- C++ listeners via a native event delegate
	 *	- Blueprint listeners via a dynamic multicast delegate
	 *	with calling only a single class member.
	 *
	 *	This is set up via the MCRO_DYNAMIC_EVENT_MULTIPLEX macro next to relevant class members.
	 */
	template <CNonVoid Return, typename... Args>
	class TEventMultiplex<Return(Args...)> : public Detail::TEventMultiplexBase<Return, Args...>
	{
	public:
		template <CSameAs<FEventDelegate>... Delegates>
		TEventMultiplex(FDelegate&& function, Delegates&&... delegates)
			: Detail::TEventMultiplexBase<void, Args...>(function, delegates...)
		{}

		Return operator() (Args... args)
		{
			Return result = Function.Execute(args...);
			Multicast.Broadcast(args...);
			return result;
		}
		
		template <typename... OptionalObject> requires (sizeof...(OptionalObject) <= 1)
		FDelegate Delegation(OptionalObject... object)
		{
			return From(object..., &TEventMultiplex::operator());
		};
	};

	template <typename... Args>
	class TEventMultiplex<void(Args...)> : public Detail::TEventMultiplexBase<void, Args...>
	{
	public:
		template <CSameAs<FEventDelegate>... Delegates>
		TEventMultiplex(FDelegate&& function, Delegates&&... delegates)
			: Detail::TEventMultiplexBase<void, Args...>(function, delegates...)
		{}

		void operator ()(Args... args)
		{
			Function.Execute(args...);
			Multicast.Broadcast(args...);
		}
		
		template <typename... OptionalObject> requires (sizeof...(OptionalObject) <= 1)
		FDelegate Delegation(OptionalObject... object)
		{
			return From(object..., &TEventMultiplex::operator());
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
			TSetReturn<Return, TDynamicSignature<Dynamic>>
		>;
	};
	
	template <
		typename Dynamic,
		typename Return = TReturnOverride<Dynamic>
	>
	using TNativeMultiplex = typename TNativeMultiplex_Struct<Dynamic, Return>::Type;
}
