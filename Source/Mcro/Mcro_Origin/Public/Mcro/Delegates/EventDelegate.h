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
#include "Mcro/Construct.h"
#include "Mcro/Delegates/AsNative.h"
#include "Mcro/Delegates/DelegateFrom.h"

namespace Mcro::Delegates
{
	using namespace Mcro::FunctionTraits;
	using namespace Mcro::Construct;
	
	/** Settings for the TEventDelegate class, which defines optional behavior when adding a binding to it */
	enum EInvokeMode
	{
		/** The event delegate will act the same as a TMulticastDelegate */
		DefaultInvocation = 0,

		/** The binding will be automatically removed after the next broadcast */
		InvokeOnce = 1 << 0,

		/** The binding will be executed immediately if the delegate has already been broadcasted */
		BelatedInvoke = 1 << 1,

		/**
		 *	Attempt to copy arguments when storing them for belated invokes, instead of perfect
		 *	forwarding them. This is only considered from the template argument
		 */
		CopyArguments = 1 << 2,

		/** Enable mutex locks around adding/broadcasting delegates. Only considered in DefaultInvokeMode */
		ThreadSafeEvent = 1 << 3
	};

	template <typename Function, int32 DefaultInvokeMode = DefaultInvocation>
	class TEventDelegate;

	/**
	 *	"Extension" of a common TMulticastDelegate. It allows to define optional "settings" when
	 *	ad1ding a binding, in order to:
	 *	- Remove the binding after the next broadcast
	 *	- Execute the associated event immediately if the delegate has already been broadcast
	 *	- Allow comfortable chaining
	 *	
	 *	The delegate can be defined with settings that will be applied by default to all bindings
	 *	(but the behavior can still be changed per-binding if needed).
	 *	
	 *	Example usage:
	 *
	 * @code
	 *	// The delegate will use default settings (i.e. the behavior will be the same as a TMulticastDelegate by default)
	 *	using FMyNativeDelegate = TEventDelegate<void(int32 someParam)>;
	 * 
	 *	// Fire a new binding immediately if the delegate has been already broadcasted
	 *	using FMyNativeDelegate = TEventDelegate<void(int32 someParam), EInvokeMode::BelatedInvoke>;
	 *
	 *	// Fire a new binding immediately if the delegate has been already broadcasted,
	 *	// AND the binding will be removed after the next broadcast
	 *	using FMyNativeDelegate = TEventDelegate<void(int32 someParam), EInvokeMode::BelatedInvoke | EInvokeMode::InvokeOnce>;
	 *	@endcode 
	 */
	template <typename... Args, int32 DefaultInvokeMode>
	class TEventDelegate<void(Args...), DefaultInvokeMode>
	{
	public:
		using MutexLock = std::conditional_t<DefaultInvokeMode & ThreadSafeEvent, FScopeLock, FVoid>;
		
		using FunctionSignature = void(Args...);
		using FDelegate = TDelegate<FunctionSignature, FDefaultDelegateUserPolicy>;
		
		using ArgumentsCache = std::conditional_t<
			DefaultInvokeMode & CopyArguments,
			TTuple<std::decay_t<Args>...>,
			TTuple<Args...>
		>;
		
		void Broadcast(Args&&... args)
		{
			MutexLock lock(&Mutex.Get());
			bHasBroadcasted = true;
			if constexpr (DefaultInvokeMode & CopyArguments)
				Cache = ArgumentsCache(args...);
			else
				Cache = ArgumentsCache(Forward<Args>(args)...);
			
			MulticastDelegate.Broadcast(Forward<Args>(args)...);
		
			for (const FDelegateHandle& handle : OnlyNextDelegates)
				MulticastDelegate.Remove(handle);
			
			OnlyNextDelegates.Empty();
		}

		/**
		 *	Create a delegate object which is broadcasting this event. This is useful for chaining
		 *	events together like so:
		 *	```
		 *	MyEvent.Add(MyOtherEvent.Delegation(this));
		 *	```
		 *	
		 *	@param object Optionally bind an object to the event delegation 
		 */
		template <typename... OptionalObject> requires (sizeof...(OptionalObject) <= 1)
		FDelegate Delegation(OptionalObject&&... object)
		{
			return From(Forward<OptionalObject>(object)..., &TEventDelegate::Broadcast);
		};

		/**
		 *	Adds a new delegate to the event delegate.
		 *	
		 *	@param delegate  The delegate to bind
		 *	
		 *	@param invokeMode
		 *	The (optional) settings to use for this binding. Not passing anything means that it will
		 *	use the default settigns for this event delegate
		 *	
		 *	@return Handle to the delegate
		 */
		FDelegateHandle Add(FDelegate delegate, const EInvokeMode& invokeMode = DefaultInvocation)
		{
			MutexLock lock(&Mutex.Get());
			return AddInternal(delegate, invokeMode);
		}
		
		/**
		 *	Bind multiple delegates at once, useful for initial mandatory bindings  
		 *	@return  This event
		 */
		template <CSameAs<FDelegate>... Delegates>
		TEventDelegate& With(Delegates&&... delegates)
		{
			MutexLock lock(&Mutex.Get());
			(AddInternal(delegates, DefaultInvocation), ...);
			return *this;
		}

		TEventDelegate() {};
		
		/** Bind multiple delegates at once, useful for initial mandatory bindings */
		template <CSameAs<FDelegate>... Delegates>
		TEventDelegate(Delegates... delegates)
		{
			WithHelper(Add(delegates)...);
		}

		/**
		 *	Adds a new dynamic delegate to this event delegate.
		 *	
		 *	@param dynamicDelegate  The dynamic delegate to bind
		 *	
		 *	@param invokeMode
		 *	The (optional) settings to use for this binding. Not passing anything means that it will
		 *	use the default settigns for this event delegate
		 *	
		 *	@return Handle to the delegate
		 */
		template <CDynamicDelegate DynamicDelegateType>
		FDelegateHandle Add(const DynamicDelegateType& dynamicDelegate, const EInvokeMode& invokeMode = DefaultInvocation)
		{
			MutexLock lock(&Mutex.Get());
			return AddInternal(AsNative(dynamicDelegate), invokeMode, FDelegateHandle(), dynamicDelegate.GetUObject(), dynamicDelegate.GetFunctionName());
		}

		/**
		 *	Adds the given dynamic delegate only if it's not already bound to this event delegate.
		 *	
		 *	@param dynamicDelegate  The dynamic delegate to bind
		 *	
		 *	@param invokeMode
		 *	The (optional) settings to use for this binding. Not passing anything means that it will
		 *	use the default settigns for this event delegate
		 *	
		 *	@return
		 *	Handle to the delegate. If the binding already existed, the handle to the existing
		 *	binding is returned
		 */
		template <CDynamicDelegate DynamicDelegateType>
		FDelegateHandle AddUnique(const DynamicDelegateType& dynamicDelegate, EInvokeMode invokeMode = DefaultInvocation)
		{
			MutexLock lock(&Mutex.Get());
			return AddUniqueInternal(AsNative(dynamicDelegate), invokeMode, dynamicDelegate.GetUObject(), dynamicDelegate.GetFunctionName());
		}

	private:
		bool RemoveInternal(const FDelegateHandle& delegateHandle)
		{
			const bool result = MulticastDelegate.Remove(delegateHandle);

			if (const FBoundUFunction* key = BoundUFunctionsMap.FindKey(delegateHandle))
				BoundUFunctionsMap.Remove(*key);

			OnlyNextDelegates.Remove(delegateHandle);

			return result;
		}
		
	public:
		/**
		 *	Remove the binding associated to the given handle
		 *	
		 *	@param delegateHandle  Handle of the binding to remove
		 *	
		 *	@return True if a binding was removed; false otherwise 
		 */
		bool Remove(const FDelegateHandle& delegateHandle)
		{
			MutexLock lock(&Mutex.Get());
			return RemoveInternal(delegateHandle);
		}

		/**
		 *	Remove the binding associated to the dynamic delegate.
		 *	
		 *	@param dynamicDelegate  The dynamic delegate to remove
		 *	
		 *	@return True if a binding was removed; false otherwise 
		 */
		template <class DynamicDelegateType>
		bool Remove(const DynamicDelegateType& dynamicDelegate)
		{
			MutexLock lock(&Mutex.Get());
			FDelegateHandle delegateHandle;
			if (BoundUFunctionsMap.RemoveAndCopyValue(FBoundUFunction(dynamicDelegate.GetUObject(), dynamicDelegate.GetFunctionName()), delegateHandle))
				return RemoveInternal(delegateHandle);

			return false;
		}

		/**
		 *	Removes all binding associated to the given object
		 *	
		 *	@param inUserObject  The object to remove the bindings for
		 *	
		 *	@return The total number of bindings that were removed
		 */
		int32 RemoveAll(const void* inUserObject)
		{
			MutexLock lock(&Mutex.Get());
			for (auto it = BoundUFunctionsMap.CreateIterator(); it; ++it)
				if (!it.Key().Key.IsValid() || it.Key().Key.Get() == inUserObject)
					it.RemoveCurrent();

			return MulticastDelegate.RemoveAll(inUserObject);
		}

		/** Resets all states of this event delegate to their default. */
		void Reset()
		{
			MutexLock lock(&Mutex.Get());
			MulticastDelegate.Clear();
			OnlyNextDelegates.Reset();
			BoundUFunctionsMap.Reset();
			bHasBroadcasted = false;
			Cache.Reset();
		}

		/** @returns true if this event delegate was ever broadcasted. */
		bool IsBroadcasted() const
		{
			return bHasBroadcasted;
		}

	private:

		FDelegateHandle AddUniqueInternal(
			FDelegate delegate,
			const EInvokeMode& invokeMode,
			const UObject* boundObject,
			const FName& boundFunctionName
		) {
			FDelegateHandle uniqueHandle;
			
			if (const FDelegateHandle* delegateHandle = BoundUFunctionsMap.Find(FBoundUFunction(boundObject, boundFunctionName)))
				uniqueHandle = *delegateHandle;
			
			return AddInternal(delegate, invokeMode, uniqueHandle, boundObject, boundFunctionName);
		}

		FDelegateHandle AddInternal(
			FDelegate delegate,
			EInvokeMode invokeMode,
			FDelegateHandle const& uniqueHandle = FDelegateHandle(), 
			const UObject* boundObject = nullptr,
			FName const& boundFunctionName = NAME_None
		) {
			const int32 actualInvokeMode = invokeMode == DefaultInvocation ? DefaultInvokeMode : invokeMode;

			if (bHasBroadcasted && actualInvokeMode & (BelatedInvoke | InvokeOnce))
			{
				CallBelated(delegate);
				return FDelegateHandle();
			}
			
			FDelegateHandle outputHandle = uniqueHandle;
			if (!outputHandle.IsValid())
			{
				outputHandle = MulticastDelegate.Add(delegate);

				if (boundObject && boundFunctionName != NAME_None)
					BoundUFunctionsMap.Add(FBoundUFunction(boundObject, boundFunctionName), outputHandle);

				if (actualInvokeMode & InvokeOnce)
					OnlyNextDelegates.Add(outputHandle);
			}

			if (bHasBroadcasted && actualInvokeMode & BelatedInvoke)
				CallBelated(delegate);
			
			return outputHandle;
		}

		void CallBelated(FDelegate& delegate)
		{
			InvokeWithTuple(&delegate, &FDelegate::Execute, Cache.GetValue());
		}
		
		using FBoundUFunction = TPair<TWeakObjectPtr<const UObject>, FName>;

		bool                                        bHasBroadcasted = false;
		mutable TInitializeOnCopy<FCriticalSection> Mutex;
		TSet<FDelegateHandle>                       OnlyNextDelegates;
		TMap<FBoundUFunction, FDelegateHandle>      BoundUFunctionsMap;
		TOptional<ArgumentsCache>                   Cache;
		TMulticastDelegate<void(Args...), FDefaultDelegateUserPolicy> MulticastDelegate;
	};

	template <typename Signature>
	using TRetainingEventDelegate = TEventDelegate<Signature, CopyArguments>;

	template <typename Signature>
	using TBelatedEventDelegate = TEventDelegate<Signature, BelatedInvoke>;

	template <typename Signature>
	using TBelatedRetainingEventDelegate = TEventDelegate<Signature, BelatedInvoke | CopyArguments>;

	template <typename Signature>
	using TOneTimeEventDelegate = TEventDelegate<Signature, InvokeOnce>;
	
	template <typename Signature>
	using TOneTimeRetainingEventDelegate = TEventDelegate<Signature, InvokeOnce | CopyArguments>;

	template <typename Signature>
	using TOneTimeBelatedEventDelegate = TEventDelegate<Signature, InvokeOnce | BelatedInvoke>;

	template <typename Signature>
	using TOneTimeRetainingBelatedEventDelegate = TEventDelegate<Signature,
		InvokeOnce | BelatedInvoke | CopyArguments
	>;

	/** Map the input dynamic multicast delegate to a conceptually compatible native event delegate type */
	template <CDynamicMulticastDelegate Dynamic, int32 DefaultSettings = DefaultInvocation>
	struct TNativeEvent_Struct
	{
		using Type = TEventDelegate<TDynamicSignature<Dynamic>, DefaultSettings>;
	};
	
	/** Map the input dynamic multicast delegate to a conceptually compatible native event delegate type */
	template <typename Dynamic, int32 DefaultSettings = DefaultInvocation>
	using TNativeEvent = typename TNativeEvent_Struct<Dynamic, DefaultSettings>::Type;
}
