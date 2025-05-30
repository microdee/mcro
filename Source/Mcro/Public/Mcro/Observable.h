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
#include "Mcro/AssertMacros.h"
#include "Mcro/Delegates/EventDelegate.h"
#include "Mcro/Observable.Fwd.h"

namespace Mcro::Observable
{
	using namespace Mcro::Delegates;
	using namespace Mcro::InitializeOnCopy;

	/**
	 *	@brief
	 *	This struct holds the circumstances of the data change. It cannot be moved or copied and its lifespan is
	 *	managed entirely by `TState`
	 */
	template <typename T>
	struct TChangeData
	{
		template <CDefaultInitializable = T>
		TChangeData() : Next() {}
		
		template <CMoveConstructible = T>
		TChangeData(T&& value) : Next(Forward<T>(value)) {}

		template <CCopyConstructible = T>
		TChangeData(const TChangeData& from) : Next(from.Next), Previous(from.Previous) {}
		
		template <CMoveConstructible = T>
		TChangeData(TChangeData&& from) noexcept : Next(MoveTemp(from.Next)), Previous(MoveTemp(from.Previous)) {}
		
		template <typename Arg>
		requires (!CSameAs<Arg, TChangeData> && !CSameAs<Arg, T>)
		TChangeData(Arg&& arg) : Next(Forward<Arg>(arg)) {}

		template <typename... Args>
		requires (sizeof...(Args) > 1)
		TChangeData(Args&&... args) : Next(Forward<Args>(args)...) {}
		
		T Next;
		TOptional<T> Previous;
	};

	/** @brief Public API and base class for `TState` which shouldn't concern with policy flags or thread safety */
	template <typename T>
	struct IState : IStateTag
	{
		using Type = T;
		using ReadLockVariant = TVariant<FReadScopeLock, FVoid>;
		using WriteLockVariant = TVariant<FWriteScopeLock, FVoid>;
		
		virtual ~IState() = default;
		
		/**
		 *	@brief
		 *	Get the wrapped value if for some reason the conversion operator is not enough or deleted.
		 *	Thread safety is not considered in this function, use `ReadLock` before `Get`, or use `GetOnAnyThread`
		 *	which provides a read lock, if thread safety is a concern.
		 */
		virtual T const& Get() const = 0;
		
		/**
		 *	@brief
		 *	Set the wrapped value if for some reason the assignment operator is not enough or deleted. When thread
		 *	safety is enabled Set will automatically lock this state for writing.
		 *
		 *	@warning
		 *	Setting this state from within its change listeners is prohibited and will trigger a check()
		 */
		virtual void Set(T const& value) = 0;
		
		/**
		 *	@brief  Modify this state via an l-value ref in a functor
		 *	
		 *	@param  modifier  The functor which modifies this value
		 *	
		 *	@param alwaysNotify
		 *	Notify observers about the change even when the previous state is not different after the modification.
		 *	This is only applicable when T is copyable, comparable, StorePrevious flag is set and AlwaysNotify flag is
		 *	not set via policy.
		 */
		virtual void Modify(TUniqueFunction<void(T&)>&& modifier, bool alwaysNotify = true) = 0;

	protected:
		template <CChangeListener<T> Function>
		static auto DelegateValueArgument(Function const& onChange)
		{
			return [onChange](TChangeData<T> const& change)
			{
				if constexpr (CChangeNextOnlyListener<Function, T>)
					onChange(change.Next);
				if constexpr (CChangeNextPreviousListener<Function, T>)
					onChange(change.Next, change.Previous);
			};
		}
		
		virtual FDelegateHandle OnChangeImpl(TDelegate<void(TChangeData<T> const&)>&& onChange, FEventPolicy const& eventPolicy = {}) = 0;
	public:
		
		/** @brief Add a delegate which gets a `TChangeData<T> const&` if this state has been set. */
		FDelegateHandle OnChange(TDelegate<void(TChangeData<T> const&)> onChange, FEventPolicy const& eventPolicy = {})
		{
			return OnChangeImpl(MoveTemp(onChange), eventPolicy);
		};

		/**
		 *	@brief
		 *	Add a function without object binding which either has one or two arguments with the following signature:
		 *	`[](T const& next, [TOptional<T> const& previous])`
		 *
		 *	Where the argument `previous` is optional (to have, not its type). The argument `previous` when it is
		 *	present is TOptional because it may only have a value when StorePrevious policy is active and T is copyable.
		 */
		template <CChangeListener<T> Function>
		FDelegateHandle OnChange(Function const& onChange, FEventPolicy const& eventPolicy = {})
		{
			return OnChange(InferDelegate::From(DelegateValueArgument(onChange)), eventPolicy);
		}
		
		/**
		 *	@brief
		 *	Add a function with an object binding which either has one or two arguments with the following signature:
		 *	`[](T const& next, [TOptional<T> const& previous])`
		 *
		 *	Where the argument `previous` is optional (to have, not its type). The argument `previous` when it is
		 *	present is TOptional because it may only have a value when StorePrevious policy is active and T is copyable.
		 */
		template <typename Object, CChangeListener<T> Function>
		FDelegateHandle OnChange(Object&& object, Function const& onChange, FEventPolicy const& eventPolicy = {})
		{
			return OnChange(InferDelegate::From(Forward<Object>(object), DelegateValueArgument(onChange)), eventPolicy);
		}

		/**
		 *	@brief  Pull changes from another state, syncing the value between the two. Values will be copied.
		 *
		 *	@tparam Other  Type convertible to the value of this state
		 *	@tparam Guard  The type of the lifespan guarding object
		 *	
		 *	@param object
		 *	A lifespan guarding object in the context of this state, shared/weak pointer or UObject recommended.
		 *	
		 *	@param otherState  A reference to the other state.
		 */
		template <CConvertibleToDecayed<T> Other, typename Guard>
		void SyncPull(Guard&& object, IState<Other>& otherState)
		{
			otherState.OnChange(
				Forward<Guard>(object),
				[this](Other const& next) { Set(next); },
				{.Belated = true}
			);
		}

		/**
		 *	@brief  Push changes from another state, syncing the value between the two. Values will be copied.
		 *
		 *	@tparam Other  The value of this state convertible to this type
		 *	@tparam Guard  The type of the lifespan guarding object
		 *	
		 *	@param object
		 *	A lifespan guarding object in the context of the other state, shared/weak pointer or UObject recommended.
		 *	
		 *	@param otherState  A reference to the other state.
		*/
		template <typename Other, typename Guard>
		requires CConvertibleToDecayed<Other, T>
		void SyncPush(Guard&& object, IState<Other>& otherState)
		{
			OnChange(
				Forward<Guard>(object),
				[&otherState](T const& next) { otherState.Set(next); },
				{.Belated = true}
			);
		}

		/**
		 *	@brief
		 *	Given value will be stored in the state only if T is equality comparable and it differs from the current
		 *	state value. If T is not equality comparable this function is equivalent to Set and always returns true.
		 *
		 *	@return
		 *	True if the given value was different from the previous state value. Always returns true when T is is not
		 *	equality comparable. 
		 */
		virtual bool HasChangedFrom(const T& nextValue) = 0;

		/** @brief Returns true if this state has ever been changed from its initial value given at construction. */
		virtual bool HasEverChanged() const = 0;

		/** @brief Equivalent to `TMulticastDelegate::Remove` */
		virtual bool Remove(FDelegateHandle const& handle) = 0;
		
		/** @brief Equivalent to `TMulticastDelegate::RemoveAll` */
		virtual int32 RemoveAll(const void* object) = 0;
		
		/**
		 *	@brief
		 *	If thread safety is enabled in DefaultPolicy, get the value with a bundled read-scope-lock. Otherwise the
		 *	tuple returns an empty dummy struct as its second argument.
		 *	
		 *	Use C++17 structured binding for convenience:
		 *	@code
		 *	auto [value, lock] = MyState.GetOnAnyThread();
		 *	@endcode
		 *
		 *	@remarks
		 *	Unlike the placeholder `auto` keyword, the structured binding `auto` keyword preserves reference qualifiers.
		 *	See https://godbolt.org/z/jn918fKfd
		 *
		 *	@return
		 *	The lock is returned as TUniquePtr it's slightly more expensive because of ref-counting but it makes the API
		 *	so much easier to use as TState can decide to return a real lock or just a dummy.
		 */
		virtual TTuple<T const&, TUniquePtr<ReadLockVariant>> GetOnAnyThread() const = 0;

		/**
		 *	@brief  Lock this state for reading for the current scope.
		 *
		 *	@return
		 *	The lock is returned as TUniquePtr it's slightly more expensive because of ref-counting but it makes the API
		 *	so much easier to use as TState can decide to return a real lock or just a dummy.
		 */
		virtual TUniquePtr<ReadLockVariant> ReadLock() const = 0;
		
		/**
		 *	@brief  Lock this state for writing for the current scope.
		 *	
		 *	@return
		 *	The lock is returned as TUniquePtr it's slightly more expensive because of ref-counting but it makes the API
		 *	so much easier to use as TState can decide to return a real lock or just a dummy.
		 */
		virtual TUniquePtr<WriteLockVariant> WriteLock() = 0;

		/** @brief Get the previous value if StorePrevious is enabled and there was at least one change */
		virtual TOptional<T> const& GetPrevious() const = 0;
		
		/**
		 *	@brief
		 *	Get the previous value if StorePrevious is enabled and there was at least one change. Get a fallback value
		 *	otherwise.
		 *
		 *	@param fallback  Return this value if there's no previous one available 
		 */
		virtual T const& GetPrevious(T const& fallback) const = 0;
		
		/**
		 *	@brief
		 *	Get the previous value if StorePrevious is enabled and there was at least one change or the current value
		 *	otherwise.
		 */
		virtual T const& GetPreviousOrCurrent() const = 0;

		/**
		 *	@brief  Set the previous value to the current one. Useful in Ticks.
		 *
		 *	It will not trigger change notifications but it will use a write-lock when thread-safety is enabled.
		 *	If `StorePrevious` is disabled it will do nothing.
		 */
		virtual void NormalizePrevious() = 0;

		/**
		 *	@brief  Returns true when this state is currently true, but previously it wasn't
		 *
		 *	@param fallback  Use this as the fallback previous state.
		 */
		template <CBooleanTestable = T>
		bool BecameTrue(bool fallback = false) const
		{
			bool previous = GetPrevious().IsSet()
				? static_cast<bool>(GetPrevious().GetValue())
				: fallback;
			
			return static_cast<bool>(Get()) && !previous; 
		}
		
		/**
		 *	@brief  Returns true when this state is currently true, but previously it wasn't
		 *
		 *	@param fallback  Use this as the fallback previous state.
		 */
		template <CBooleanTestable = T>
		bool OnDown(bool fallback = false) const { return BecameTrue(fallback); }

		/**
		 *	@brief  Returns true when this state is currently false, but previously it wasn't
		 *
		 *	@param fallback  Use this as the fallback previous state.
		 */
		template <CBooleanTestable = T>
		bool BecameFalse(bool fallback = false) const
		{
			bool previous = GetPrevious().IsSet()
				? static_cast<bool>(GetPrevious().GetValue())
				: fallback;
			
			return !static_cast<bool>(Get()) && previous; 
		}
		
		/**
		 *	@brief  Returns true when this state is currently false, but previously it wasn't
		 *
		 *	@param fallback  Use this as the fallback previous state.
		 */
		template <CBooleanTestable = T>
		bool OnUp(bool fallback = false) const { return BecameFalse(); }

		/** @brief Returns true when current value is not equal to previous one. */
		template <CCoreEqualityComparable = T>
		bool HasChanged() const
		{
			return Get() != GetPreviousOrCurrent();
		}
		
		/** @brief Returns true when current value is not equal to previous one. */
		template <CCoreEqualityComparable = T>
		bool HasChanged(T const& fallback) const
		{
			return Get() != GetPrevious(fallback);
		}

		template <typename Self>
		operator T const& (this Self&& self)
		{
			return self.Get();
		}
		
		template <typename Self>
		const T* operator -> (this Self&& self)
		{
			return &self.Get();
		}
	};

	/**
	 *	@brief 
	 *	Storage wrapper for any value which state needs to be tracked or their change needs to be observed.
	 *	By default, TState is not thread-safe unless FStatePolicy::ThreadSafe policy is active in DefaultPolicy
	 *
	 *	TState and IState allows developers an expressive API for tracking/syncing changes of a stateful entity. Like
	 *	button presses, resolution changes or storing the last error. It doesn't only give a storage for the value but
	 *	triggers events when the value changes. This alleviates the need for explicit `OnStuffChanged` events in the API
	 *	where `TState` is used.
	 *
	 *	Use `TState` on class members where a default set of policy can be declared in compile time, and use `IState`
	 *	where these states should be referenced in a function argument for example. In the latter case the policy flags
	 *	are erased so multiple states with different policies are compatible with each-other. Although that is not
	 *	necessary and not recommended if we would only need to consume their value, as states can have implicit
	 *	conversion to their value (returning a const-ref).
	 *
	 *	If given value is equality comparable, TState will only trigger change events when the previous and the current
	 *	values are different. Unless that behavior is overridden by `FStatePolicy` flags. A default set of flags are
	 *	determined by `StatePolicyFor` template for any given type.
	 */
	template <typename T, FStatePolicy DefaultPolicy>
	struct TState : IState<T>
	{
		template <typename ThreadSafeType, typename NaiveType>
		using ThreadSafeSwitch = std::conditional_t<DefaultPolicy.ThreadSafe, ThreadSafeType, NaiveType>;
		
		using StateBase = IState<T>;

		using typename StateBase::ReadLockVariant;
		using typename StateBase::WriteLockVariant;
		
		using ReadLockType = ThreadSafeSwitch<FReadScopeLock, FVoid>;
		using WriteLockType = ThreadSafeSwitch<FWriteScopeLock, FVoid>;
		
		static constexpr FStatePolicy DefaultPolicyFlags = DefaultPolicy;
		
		/** @brief Enable default constructor only when T is default initializable */
		template <CDefaultInitializable = T>
		TState() : Value() {}
		
		/** @brief Enable copy constructor for T only when T is copy constructable */
		template <CCopyConstructible = T>
		explicit TState(T const& value) : Value(value) {}
		
		/** @brief Enable move constructor for T only when T is move constructable */
		template <CMoveConstructible = T>
		explicit TState(T&& value) : Value(MoveTemp(value)) {}
		
		/** @brief Enable copy constructor for the state only when T is copy constructable */
		template <CCopyConstructible = T>
		TState(TState const& other) : Value(other.Value.Next) {}
		
		/** @brief Enable move constructor for the state only when T is move constructable */
		template <CMoveConstructible = T>
		TState(TState&& other) : Value(MoveTemp(other.Value.Next)) {}

		/** @brief Construct value in-place with non-semantic single argument constructor */
		template <typename Arg>
		requires (!CConvertibleTo<Arg, TState> && !CSameAs<Arg, T>)
		TState(Arg&& arg) : Value(Forward<Arg>(arg)) {}

		/** @brief Construct value in-place with multiple argument constructor */
		template <typename... Args>
		requires (sizeof...(Args) > 1)
		TState(Args&&... args) : Value(Forward<Args>(args)...) {}
		
		virtual T const& Get() const override { return Value.Next; }
		
		virtual TTuple<T const&, TUniquePtr<ReadLockVariant>> GetOnAnyThread() const override
		{
			return { Value.Next, ReadLock() };
		}
		
		virtual void Set(T const& value) override
		{
			ASSERT_QUIT(!Modifying, ,
				->WithMessage(TEXT_"Attempting to set this state while this state is already being set from somewhere else.")
			);
			TGuardValue modifyingGuard(Modifying, true);
			auto lock = WriteLock();
			bool allow = true;

			if constexpr (CCoreEqualityComparable<T>)
				allow = PolicyFlags.AlwaysNotify || Value.Next != value;
			
			if constexpr (CCopyable<T>)
			if (PolicyFlags.StorePrevious && (allow || PolicyFlags.AlwaysStorePrevious))
				Value.Previous = Value.Next;

			if (allow)
			{
				Value.Next = value;
				OnChangeEvent.Broadcast(Value);
			}
		}
		
		virtual void Modify(TUniqueFunction<void(T&)>&& modifier, bool alwaysNotify = true) override
		{
			ASSERT_QUIT(!Modifying, ,
				->WithMessage(TEXT_"Attempting to set this state while this state is already being set from somewhere else.")
			);
			TGuardValue modifyingGuard(Modifying, true);
			auto lock = WriteLock();
			bool allow = true;
			TOptional<T> previous;
			
			if constexpr (CCopyable<T>)
			if (PolicyFlags.StorePrevious)
				previous = Value.Next;
			
			modifier(Value.Next);

			if constexpr (CCopyable<T> && CCoreEqualityComparable<T>)
				allow = alwaysNotify
					||  PolicyFlags.AlwaysNotify
					|| !Value.Previous.IsSet()
					||  previous.GetValue() != Value.Next;
			
			if constexpr (CCopyable<T>)
			if (PolicyFlags.StorePrevious && (allow || PolicyFlags.AlwaysStorePrevious))
				Value.Previous = previous;
			
			if (allow)
				OnChangeEvent.Broadcast(Value);
		}

	protected:
		virtual FDelegateHandle OnChangeImpl(TDelegate<void(TChangeData<T> const&)>&& onChange, FEventPolicy const& eventPolicy = {}) override
		{
			auto lock = WriteLock();
			return OnChangeEvent.Add(onChange, eventPolicy);
		}

	public:
		virtual bool Remove(FDelegateHandle const& handle) override
		{
			auto lock = WriteLock();
			return OnChangeEvent.Remove(handle);
		}

		virtual int32 RemoveAll(const void* object) override
		{
			auto lock = WriteLock();
			return OnChangeEvent.RemoveAll(object);
		}

		virtual bool HasChangedFrom(const T& nextValue) override
		{
			if constexpr (CCoreEqualityComparable<T>)
			{
				bool hasChanged = Value.Next != nextValue;
				Set(nextValue);
				return hasChanged;
			}
			else
			{
				Set(nextValue);
				return true;
			}
		}

		virtual bool HasEverChanged() const override
		{
			return OnChangeEvent.IsBroadcasted();
		}
		
		virtual TUniquePtr<ReadLockVariant> ReadLock() const override
		{
			return MakeUnique<ReadLockVariant>(TInPlaceType<ReadLockType>(), Mutex.Get());
		}
		
		virtual TUniquePtr<WriteLockVariant> WriteLock() override
		{
			return MakeUnique<WriteLockVariant>(TInPlaceType<WriteLockType>(), Mutex.Get());
		}

		virtual TOptional<T> const& GetPrevious() const override
		{
			return Value.Previous;
		}

		virtual T const& GetPrevious(T const& fallback) const override
		{
			return Value.Previous.Get(fallback);
		}
		
		virtual T const& GetPreviousOrCurrent() const override
		{
			return Value.Previous.IsSet() ? Value.Previous.GetValue() : Value.Next;
		}

		virtual void NormalizePrevious() override
		{
			if (!PolicyFlags.StorePrevious) return;
			ASSERT_QUIT(!Modifying, ,
				->WithMessage(TEXT_"Attempting to set this state while this state is already being set from somewhere else.")
			);
			TGuardValue modifyingGuard(Modifying, true);
			auto lock = WriteLock();
			Value.Previous = Value.Next;
		}
		
		template <CConvertibleTo<T> Other>
		requires (!CState<Other>)
		TState& operator = (Other&& value)
		{
			if constexpr (CCopyable<Other>)
				Set(value);
			else if constexpr (CMovable<Other>)
				Set(MoveTemp(value));
			return *this;
		}

		FStatePolicy PolicyFlags { DefaultPolicy };
		
	private:
		TEventDelegate<void(TChangeData<T> const&)> OnChangeEvent;
		TChangeData<T> Value;
		bool Modifying = false;
		mutable TInitializeOnCopy<FRWLock> Mutex;
	};

	template <typename LeftValue, CWeaklyEqualityComparableWith<LeftValue> RightValue>
	bool operator == (IState<LeftValue> const& left, IState<RightValue> const& right)
	{
		return left.Get() == right.Get();
	}
	
	template <typename LeftValue, CPartiallyOrderedWith<LeftValue> RightValue>
	bool operator <=> (IState<LeftValue> const& left, IState<RightValue> const& right)
	{
		return left.Get() <=> right.Get();
	}
}
