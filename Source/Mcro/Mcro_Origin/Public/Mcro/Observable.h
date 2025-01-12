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
#include "Mcro/Construct.h"
#include "Mcro/Observable.Fwd.h"

namespace Mcro::Observable
{
	using namespace Mcro::Delegates;
	using namespace Mcro::Construct;

	/**
	 * This struct holds the circumstances of the data change. It cannot be moved or copied and its lifespan is
	 * managed entirely by `TState`
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
		TChangeData(TChangeData&& from) : Next(MoveTemp(from.Next)), Previous(MoveTemp(from.Previous)) {}
		
		template <typename Arg>
		requires (!CSameAs<Arg, TChangeData> && !CSameAs<Arg, T>)
		TChangeData(Arg&& arg) : Next(Forward<Arg>(arg)) {}

		template <typename... Args>
		requires (sizeof...(Args) > 1)
		TChangeData(Args&&... args) : Next(Forward<Args>(args)...) {}
		
		T Next;
		TOptional<T> Previous;
	};

	/** Public API and base class for `TState` which shouldn't concern with policy flags or thread safety */
	template <typename T>
	struct IState : IStateTag
	{
		using Type = T;
		using ReadLockVariant = TVariant<FReadScopeLock, FVoid>;
		using WriteLockVariant = TVariant<FWriteScopeLock, FVoid>;
		
		virtual ~IState() = default;
		
		/**
		 *	Get the wrapped value if for some reason the conversion operator is not enough or deleted.
		 *	Thread safety is not considered in this function, use `ReadLock` before `Get`, or use `GetOnAnyThread`
		 *	which provides a read lock, if thread safety is a concern.
		 */
		virtual T const& Get() const = 0;
		
		/**
		 *	Set the wrapped value if for some reason the assignment operator is not enough or deleted. When thread
		 *	safety is enabled Set will automatically lock this state for writing.
		 *
		 *	@warning
		 *	Setting this state from within its change listeners is prohibited and will trigger a check()
		 */
		virtual void Set(T const& value) = 0;
		
		/**
		 *	Modify this state via an l-value ref in a functor
		 *
		 *	@param modifier  The functor which modifies this value
		 *	
		 *	@param alwaysNotify
		 *	Notify observers about the change even when the previous state is not different after the modification.
		 *	This is only applicable when T is copyable, comparable, StorePrevious flag is set and AlwaysNotify flag is
		 *	not set via policy.
		 */
		virtual void Modify(TUniqueFunction<void(T&)>&& modifier, bool alwaysNotify = true) = 0;

	protected:
		template <CChangeListener<T> Function>
		static auto DelegateValueArgument(Function const& onChange, EInvokeMode invokeMode = DefaultInvocation)
		{
			return [onChange](TChangeData<T> const& change)
			{
				if constexpr (CChangeNextOnlyListener<Function, T>)
					onChange(change.Next);
				if constexpr (CChangeNextPreviousListener<Function, T>)
					onChange(change.Next, change.Previous);
			};
		}
	public:
		
		/** Add a delegate which gets a `TChangeData<T> const&` if this state has been set. */
		virtual FDelegateHandle OnChange(TDelegate<void(TChangeData<T> const&)> onChange, EInvokeMode invokeMode = DefaultInvocation) = 0;

		/**
		 *	Add a function without object binding which either has one or two arguments with the following signature:
		 *
		 *	@code
		 *	[](T const& next, [TOptional<T> const& previous])
		 *	@endcode
		 *
		 *	Where the argument `previous` is optional (to have, not its type). The argument `previous` when it is
		 *	present is TOptional because it may only have a value when StorePrevious policy is active and T is copyable.
		 */
		template <CChangeListener<T> Function>
		FDelegateHandle OnChange(Function const& onChange, EInvokeMode invokeMode = DefaultInvocation)
		{
			return OnChange(From(DelegateValueArgument(onChange)), invokeMode);
		}
		
		/**
		 *	Add a function with an object binding which either has one or two arguments with the following signature:
		 *
		 *	@code
		 *	[](T const& next, [TOptional<T> const& previous])
		 *	@endcode
		 *
		 *	Where the argument `previous` is optional (to have, not its type). The argument `previous` when it is
		 *	present is TOptional because it may only have a value when StorePrevious policy is active and T is copyable.
		 */
		template <typename Object, CChangeListener<T> Function>
		FDelegateHandle OnChange(Object&& object, Function const& onChange, EInvokeMode invokeMode = DefaultInvocation)
		{
			return OnChange(From(Forward<Object>(object), DelegateValueArgument(onChange)), invokeMode);
		}

		/**
		 *	Given value will be stored in the state only if T is equality comparable and it differs from the current
		 *	state value. If T is not equality comparable this function is equivalent to Set and always returns true.
		 *
		 *	@return
		 *	True if the given value was different from the previous state value. Always returns true when T is is not
		 *	equality comparable. 
		 */
		virtual bool HasChangedFrom(const T& nextValue) = 0;

		/** Returns true if this state has ever been changed from its initial value given at construction. */
		virtual bool HasEverChanged() const = 0;

		/** Equivalent to `TMulticastDelegate::Remove` */
		virtual bool Remove(FDelegateHandle const& handle) = 0;
		
		/** Equivalent to `TMulticastDelegate::RemoveAll` */
		virtual int32 RemoveAll(const void* object) = 0;
		
		/**
		 *	If thread safety is enabled in DefaultPolicy, get the value with a bundled read-scope-lock. Otherwise the
		 *	tuple returns an empty dummy struct as its second argument.
		 *	Use C++17 structured binding for convenience:
		 *
		 *	@code
		 *	auto [value, lock] = MyState.GetOnAnyThread();
		 *	@endcode
		 *
		 *	Unlike the placeholder `auto` keyword, the structured binding `auto` keyword preserves reference qualifiers.
		 *	See https://godbolt.org/z/jn918fKfd
		 *
		 *	@return
		 *	The lock is returned as TUniquePtr it's slightly more expensive because of ref-counting but it makes the API
		 *	so much easier to use as TState can decide to return a real lock or just a dummy.
		 */
		virtual TTuple<T const&, TUniquePtr<ReadLockVariant>> GetOnAnyThread() const = 0;

		/**
		 *	Lock this state for reading for the current scope
		 *
		 *	@return
		 *	The lock is returned as TUniquePtr it's slightly more expensive because of ref-counting but it makes the API
		 *	so much easier to use as TState can decide to return a real lock or just a dummy.
		 */
		virtual TUniquePtr<ReadLockVariant> ReadLock() const = 0;
		
		/**
		 *	Lock this state for writing for the current scope
		 *	
		 *	@return
		 *	The lock is returned as TUniquePtr it's slightly more expensive because of ref-counting but it makes the API
		 *	so much easier to use as TState can decide to return a real lock or just a dummy.
		 */
		virtual TUniquePtr<WriteLockVariant> WriteLock() = 0;

		template <typename Self>
		operator const T& (this Self&& self)
		{
			return self.Get();
		}
		
		template <typename Self>
		const T* operator -> (this Self&& self)
		{
			return &self.Get();
		}
		
		template <typename Self, CConvertibleTo<T> Other>
		requires (!CState<Other>)
		Self& operator = (this Self&& self, Other&& value)
		{
			if constexpr (CCopyable<Other>)
				self.Set(value);
			else if constexpr (CMovable<Other>)
				self.Set(MoveTemp(value));
			return self;
		}
	};

	/**
	 * Storage wrapper for any value which state needs to be tracked or their change needs to be observed.
	 * By default `TState` is not thread-safe unless ThreadSafeState policy is active in `DefaultPolicy`
	 */
	template <typename T, int32 DefaultPolicy>
	struct TState : IState<T>
	{
		template <typename ThreadSafeType, typename NaiveType>
		using ThreadSafeSwitch = std::conditional_t<static_cast<bool>(DefaultPolicy & ThreadSafeState), ThreadSafeType, NaiveType>;
		
		using StateBase = IState<T>;

		using typename StateBase::ReadLockVariant;
		using typename StateBase::WriteLockVariant;
		
		using ReadLockType = ThreadSafeSwitch<FReadScopeLock, FVoid>;
		using WriteLockType = ThreadSafeSwitch<FWriteScopeLock, FVoid>;
		
		static constexpr int32 DefaultPolicyFlags = DefaultPolicy;
		
		/** Enable default constructor only when T is default initializable */
		template <CDefaultInitializable = T>
		TState() : Value() {}
		
		/** Enable copy constructor for T only when T is copy constructable */
		template <CCopyConstructible = T>
		TState(T const& value) : Value(value) {}
		
		/** Enable move constructor for T only when T is move constructable */
		template <CMoveConstructible = T>
		TState(T&& value) : Value(MoveTemp(value)) {}
		
		/** Enable copy constructor for the state only when T is copy constructable */
		template <CCopyConstructible = T>
		TState(TState const& other) : Value(other.Value.Next) {}
		
		/** Enable move constructor for the state only when T is move constructable */
		template <CMoveConstructible = T>
		TState(TState&& other) : Value(MoveTemp(other.Value.Next)) {}

		/** Construct value in-place with non-semantic single argument constructor */
		template <typename Arg>
		requires (!CConvertibleTo<Arg, TState> && !CSameAs<Arg, T>)
		TState(Arg&& arg) : Value(Forward<Arg>(arg)) {}

		/** Construct value in-place with multiple argument constructor */
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
				->WithMessage(TEXT("Attempting to set this state while this state is already being set from somewhere else."))
			);
			TGuardValue modifyingGuard(Modifying, true);
			auto lock = WriteLock();
			bool broadcast = true;

			if constexpr (CCoreEqualityComparable<T>)
				broadcast = PolicyFlags & AlwaysNotify || Value.Next != value;
			
			if constexpr (CCopyable<T>)
			if (PolicyFlags & StorePrevious)
				Value.Previous = Value.Next;

			Value.Next = value;

			if (broadcast)
				OnChangeEvent.Broadcast(Value);
		}
		
		virtual void Modify(TUniqueFunction<void(T&)>&& modifier, bool alwaysNotify = true) override
		{
			ASSERT_QUIT(!Modifying, ,
				->WithMessage(TEXT("Attempting to set this state while this state is already being set from somewhere else."))
			);
			TGuardValue modifyingGuard(Modifying, true);
			auto lock = WriteLock();
			bool broadcast = true;
			
			if constexpr (CCopyable<T>)
			{
				if (PolicyFlags & StorePrevious)
				{
					Value.Previous = Value.Next;
				}
			}
			
			modifier(Value.Next);

			if constexpr (CCopyable<T> && CCoreEqualityComparable<T>)
				broadcast = alwaysNotify
					|| !(PolicyFlags & StorePrevious)
					|| PolicyFlags & AlwaysNotify
					|| !Value.Previous.IsSet()
					|| Value.Previous.GetValue() != Value.Next;
			
			if (broadcast)
				OnChangeEvent.Broadcast(Value);
		}
		
		virtual FDelegateHandle OnChange(TDelegate<void(TChangeData<T> const&)> onChange, EInvokeMode invokeMode = DefaultInvocation) override
		{
			auto lock = WriteLock();
			return OnChangeEvent.Add(onChange, invokeMode);
		}

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
			if constexpr  (CCoreEqualityComparable<T>)
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

		int32 PolicyFlags = DefaultPolicy;
		
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
