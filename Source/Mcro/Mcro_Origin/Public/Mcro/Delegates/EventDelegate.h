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
#include "Mcro/InitializeOnCopy.h"
#include "Mcro/Delegates/AsNative.h"
#include "Mcro/Delegates/DelegateFrom.h"

namespace Mcro::Delegates
{
	using namespace Mcro::FunctionTraits;
	using namespace Mcro::InitializeOnCopy;
	
	/** @brief Settings for the TEventDelegate class, which defines optional behavior when adding a binding to it */
	struct FEventPolicy
	{
		/** @brief The binding will be automatically removed after the next broadcast */
		bool Once = false;

		/** @brief The binding will be executed immediately if the delegate has already been broadcasted */
		bool Belated = false;

		/**
		 *	@brief
		 *	Attempt to copy arguments when storing them for belated invokes, instead of perfect
		 *	forwarding them. This is only considered from the template argument
		 */
		bool CacheViaCopy = false;

		/** @brief Enable mutex locks around adding/broadcasting delegates. Only considered in DefaultPolicy */
		bool ThreadSafe = false;

		/** @brief Merge two policy flags */
		FORCEINLINE constexpr FEventPolicy With(FEventPolicy const& other) const
		{
			return {
				Once         || other.Once,
				Belated      || other.Belated,
				CacheViaCopy || other.CacheViaCopy,
				ThreadSafe   || other.ThreadSafe
			};
		}

		FORCEINLINE friend constexpr bool operator == (FEventPolicy const& lhs, FEventPolicy const& rhs)
		{
			return lhs.Once         == rhs.Once
				&& lhs.Belated      == rhs.Belated
				&& lhs.CacheViaCopy == rhs.CacheViaCopy
				&& lhs.ThreadSafe   == rhs.ThreadSafe
			;
		}

		FORCEINLINE friend constexpr bool operator != (FEventPolicy const& lhs, FEventPolicy const& rhs)
		{
			return !(lhs == rhs);
		}

		/** @brief Is this instance equivalent to a default constructed one */
		FORCEINLINE constexpr bool IsDefault() const
		{
			return *this == FEventPolicy();
		}
	};

	/**
	 *	@brief
	 *	"Extension" of a common TMulticastDelegate. It allows to define optional "flags" when adding a binding,
	 *	in order to:
	 *	- Remove the binding after the next broadcast
	 *	- Execute the associated event immediately if the delegate has already been broadcast
	 *	- Allow comfortable chaining
	 *	
	 *	The delegate can be defined with default settings that will be applied by default to all bindings (but the
	 *	behavior can still be changed per-binding if needed (except thread safety and argument retention mode).
	 *	
	 *	Example usage:
	 *
	 * @code
	 *	// The delegate will use default settings (i.e. the behavior will be the same as a TMulticastDelegate by default)
	 *	using FMyEventDelegate = TEventDelegate<void(int32 someParam)>;
	 * 
	 *	// Fire a new binding immediately if the delegate has been already broadcasted
	 *	using FMyEventDelegate = TEventDelegate<void(int32 someParam), {.Belated = true}>;
	 *
	 *	// Fire a new binding immediately if the delegate has been already broadcasted,
	 *	// AND the binding will be removed after the next broadcast
	 *	using FMyEventDelegate = TEventDelegate<void(int32 someParam), {.Belated = true, .Once = true}>;
	 *	@endcode 
	 */
	template <typename Function, FEventPolicy DefaultPolicy = {}>
	class TEventDelegate {};

	/** @copydoc TEventDelegate */
	template <typename... Args, FEventPolicy DefaultPolicy>
	class TEventDelegate<void(Args...), DefaultPolicy>
	{
	public:
		using MutexLock = std::conditional_t<DefaultPolicy.ThreadSafe, FScopeLock, FVoid>;
		
		using FunctionSignature = void(Args...);
		using FDelegate = TDelegate<FunctionSignature, FDefaultDelegateUserPolicy>;
		
		using ArgumentsCache = std::conditional_t<
			DefaultPolicy.CacheViaCopy,
			TTuple<std::decay_t<Args>...>,
			TTuple<Args...>
		>;

		template <typename... BroadcastArgs>
		requires CConvertibleTo<TTuple<BroadcastArgs...>, TTuple<Args...>>
		void Broadcast(BroadcastArgs&&... args)
		{
			MutexLock lock(&Mutex.Get());
			bHasBroadcasted = true;
			if constexpr (DefaultPolicy.CacheViaCopy)
				Cache = ArgumentsCache(args...);
			else
				Cache = ArgumentsCache(Forward<BroadcastArgs>(args)...);
			
			MulticastDelegate.Broadcast(Forward<BroadcastArgs>(args)...);
		
			for (const FDelegateHandle& handle : OnlyNextDelegates)
				MulticastDelegate.Remove(handle);
			
			OnlyNextDelegates.Empty();
		}

		/**
		 *	@brief
		 *	Create a delegate object which is broadcasting this event. This is useful for chaining
		 *	events together like so:
		 *	@code
		 *	MyEvent.Add(MyOtherEvent.Delegation(this));
		 *	@endcode
		 *	
		 *	@param object Optionally bind an object to the event delegation 
		 */
		template <typename... OptionalObject> requires (sizeof...(OptionalObject) <= 1)
		FDelegate Delegation(OptionalObject&&... object)
		{
			return InferDelegate::From(Forward<OptionalObject>(object)..., &TEventDelegate::Broadcast);
		};

		/**
		 *	@brief  Adds a new delegate to the event delegate.
		 *	
		 *	@param delegate  The delegate to bind
		 *	
		 *	@param policy
		 *	The (optional) settings to use for this binding. Not passing anything means that it will
		 *	use the default settigns for this event delegate
		 *	
		 *	@return Handle to the delegate
		 */
		FDelegateHandle Add(FDelegate delegate, FEventPolicy const& policy = {})
		{
			MutexLock lock(&Mutex.Get());
			return AddInternal(delegate, policy);
		}
		
		/**
		 *	@brief   Bind multiple delegates at once, useful for initial mandatory bindings  
		 *	@return  This event
		 */
		template <CSameAs<FDelegate>... Delegates>
		TEventDelegate& With(Delegates&&... delegates)
		{
			MutexLock lock(&Mutex.Get());
			(AddInternal(delegates, {}), ...);
			return *this;
		}

		TEventDelegate() {};
		
		/** @brief Bind multiple delegates at once, useful for initial mandatory bindings */
		template <CSameAs<FDelegate>... Delegates>
		TEventDelegate(Delegates... delegates)
		{
			(AddInternal(delegates, {}), ...);
		}

		/**
		 *	@brief  Adds a new dynamic delegate to this event delegate.
		 *	
		 *	@param dynamicDelegate  The dynamic delegate to bind
		 *	
		 *	@param policy
		 *	The (optional) settings to use for this binding. Not passing anything means that it will
		 *	use the default settigns for this event delegate
		 *	
		 *	@return Handle to the delegate
		 */
		template <CDynamicDelegate DynamicDelegateType>
		FDelegateHandle Add(const DynamicDelegateType& dynamicDelegate, FEventPolicy const& policy = {})
		{
			MutexLock lock(&Mutex.Get());
			return AddInternal(AsNative(dynamicDelegate), policy, {}, dynamicDelegate.GetUObject(), dynamicDelegate.GetFunctionName());
		}

		/**
		 *	@brief  Adds the given dynamic delegate only if it's not already bound to this event delegate.
		 *	
		 *	@param dynamicDelegate  The dynamic delegate to bind
		 *	
		 *	@param policy
		 *	The (optional) settings to use for this binding. Not passing anything means that it will
		 *	use the default settigns for this event delegate
		 *	
		 *	@return
		 *	Handle to the delegate. If the binding already existed, the handle to the existing
		 *	binding is returned
		 */
		template <CDynamicDelegate DynamicDelegateType>
		FDelegateHandle AddUnique(const DynamicDelegateType& dynamicDelegate, FEventPolicy const& policy = {})
		{
			MutexLock lock(&Mutex.Get());
			return AddUniqueInternal(AsNative(dynamicDelegate), policy, dynamicDelegate.GetUObject(), dynamicDelegate.GetFunctionName());
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
		 *	@brief  Remove the binding associated to the given handle
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
		 *	@brief  Remove the binding associated to the dynamic delegate.
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
		 *	@brief  Removes all binding associated to the given object
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

		/** @brief Resets all states of this event delegate to their default. */
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
			FEventPolicy const& policy,
			const UObject* boundObject,
			const FName& boundFunctionName
		) {
			FDelegateHandle uniqueHandle;
			
			if (const FDelegateHandle* delegateHandle = BoundUFunctionsMap.Find(FBoundUFunction(boundObject, boundFunctionName)))
				uniqueHandle = *delegateHandle;
			
			return AddInternal(delegate, policy, uniqueHandle, boundObject, boundFunctionName);
		}

		FDelegateHandle AddInternal(
			FDelegate delegate,
			FEventPolicy const& policy,
			FDelegateHandle const& uniqueHandle = {}, 
			const UObject* boundObject = nullptr,
			FName const& boundFunctionName = NAME_None
		) {
			const FEventPolicy actualPolicy = policy.IsDefault() ? DefaultPolicy : policy;

			if (bHasBroadcasted && actualPolicy.Belated && actualPolicy.Once)
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

				if (actualPolicy.Once)
					OnlyNextDelegates.Add(outputHandle);
			}

			if (bHasBroadcasted && actualPolicy.Belated)
				CallBelated(delegate);
			
			return outputHandle;
		}

		void CallBelated(FDelegate& delegate)
		{
			InvokeWithTuple(&delegate, &FDelegate::Execute, Cache.GetValue());
		}
		
		using FBoundUFunction = TPair<TWeakObjectPtr<const UObject>, FName>;

		bool bHasBroadcasted = false;
		
		mutable TInitializeOnCopy<FCriticalSection> Mutex;
		TSet<FDelegateHandle>                       OnlyNextDelegates;
		TMap<FBoundUFunction, FDelegateHandle>      BoundUFunctionsMap;
		TOptional<ArgumentsCache>                   Cache;
		TMulticastDelegate<void(Args...), FDefaultDelegateUserPolicy> MulticastDelegate;
	};

	/** @brief Shorthand alias for TEventDelegate which copies arguments to its cache regardless of their qualifiers */
	template <typename Signature, FEventPolicy DefaultPolicy = {}>
	using TRetainingEventDelegate = TEventDelegate<Signature, DefaultPolicy.With({.CacheViaCopy = true})>;

	/** @brief Shorthand alias for TEventDelegate which broadcasts listeners immediately once they're added */
	template <typename Signature, FEventPolicy DefaultPolicy = {}>
	using TBelatedEventDelegate = TEventDelegate<Signature, DefaultPolicy.With({.Belated = true})>;

	/** @brief Shorthand alias for combination of TRetainingEventDelegate and TBelatedEventDelegate */
	template <typename Signature, FEventPolicy DefaultPolicy = {}>
	using TBelatedRetainingEventDelegate = TEventDelegate<Signature, DefaultPolicy.With({.Belated = true, .CacheViaCopy = true})>;

	/** @brief Shorthand alias for TEventDelegate which broadcasts listeners only once and then they're removed */
	template <typename Signature, FEventPolicy DefaultPolicy = {}>
	using TOneTimeEventDelegate = TEventDelegate<Signature, DefaultPolicy.With({.Once = true})>;
	
	/** @brief Shorthand alias for combination of TRetainingEventDelegate and TOneTimeEventDelegate */
	template <typename Signature, FEventPolicy DefaultPolicy = {}>
	using TOneTimeRetainingEventDelegate = TEventDelegate<Signature, DefaultPolicy.With({.Once = true, .CacheViaCopy = true})>;

	/** @brief Shorthand alias for combination of TBelatedEventDelegate and TOneTimeEventDelegate */
	template <typename Signature, FEventPolicy DefaultPolicy = {}>
	using TOneTimeBelatedEventDelegate = TEventDelegate<Signature, DefaultPolicy.With({.Once = true, .Belated = true})>;

	/** @brief Collect'em all */
	template <typename Signature, FEventPolicy DefaultPolicy = {}>
	using TOneTimeRetainingBelatedEventDelegate = TEventDelegate<Signature,
		DefaultPolicy.With({.Once = true, .Belated = true, .CacheViaCopy = true})
	>;

	/** @brief Map the input dynamic multicast delegate to a conceptually compatible native event delegate type */
	template <CDynamicMulticastDelegate Dynamic, FEventPolicy DefaultPolicy = {}>
	struct TNativeEvent_Struct
	{
		using Type = TEventDelegate<TDynamicSignature<Dynamic>, DefaultPolicy>;
	};
	
	/** @brief Map the input dynamic multicast delegate to a conceptually compatible native event delegate type */
	template <typename Dynamic, FEventPolicy DefaultPolicy = {}>
	using TNativeEvent = typename TNativeEvent_Struct<Dynamic, DefaultPolicy>::Type;
}
