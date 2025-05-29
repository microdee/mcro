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
 *	This is a forward declaration for types in Observable.h. Unless the full TState type is used for class member
 *	declarations, use this header in other header files.
 */

#include "CoreMinimal.h"
#include "Mcro/FunctionTraits.h"

namespace Mcro::Observable
{
	using namespace Mcro::FunctionTraits;

	/** @brief Flags expressing how TState should handle object comparison and lifespan */
	struct FStatePolicy
	{
		/**
		 *	@brief
		 *	When the object inside TState is != comparable TState wull only emit change events when the submitted
		 *	value differs from the existing one.
		 */
		bool NotifyOnChangeOnly = false;

		/** @brief Always emit change notification when a value is set on TState and don't attempt to compare them */
		bool AlwaysNotify = false;

		/** @brief Store previous value as well. If the value is equality comparable store only when it's changed. */
		bool StorePrevious = false;

		/**
		 *	@brief
		 *	If the state value is equality comparable, store the previous value even when that's equal to the new value.
		 *	This flag doesn't do anything unless StorePrevious is also true.
		 */
		bool AlwaysStorePrevious = false;

		/**
		 *	@brief
		 *	Enable mutexes during modifications, notifications and expose a public read-lock for users
		 *	of the state.
		 */
		bool ThreadSafe = false;

		/** @brief Merge two policy flags */
		FORCEINLINE constexpr FStatePolicy With(FStatePolicy const& other) const
		{
			return {
				NotifyOnChangeOnly  || other.NotifyOnChangeOnly,
				AlwaysNotify        || other.AlwaysNotify,
				StorePrevious       || other.StorePrevious,
				AlwaysStorePrevious || other.AlwaysStorePrevious,
				ThreadSafe          || other.ThreadSafe
			};
		}

		FORCEINLINE friend constexpr bool operator == (FStatePolicy const& lhs, FStatePolicy const& rhs)
		{
			return lhs.NotifyOnChangeOnly  == rhs.NotifyOnChangeOnly
				&& lhs.AlwaysNotify        == rhs.AlwaysNotify
				&& lhs.StorePrevious       == rhs.StorePrevious
				&& lhs.AlwaysStorePrevious == rhs.AlwaysStorePrevious
				&& lhs.ThreadSafe          == rhs.ThreadSafe
			;
		}

		FORCEINLINE friend constexpr bool operator != (FStatePolicy const& lhs, FStatePolicy const& rhs)
		{
			return !(lhs == rhs);
		}

		/** @brief Is this instance equivalent to a default constructed one */
		FORCEINLINE constexpr bool IsDefault() const
		{
			return *this == FStatePolicy();
		}
	};

	struct IStateTag {};

	template <typename T>
	inline constexpr FStatePolicy StatePolicyFor =
		CClass<T>
			? CCoreEqualityComparable<T>
				? FStatePolicy {.NotifyOnChangeOnly = true}
				: FStatePolicy {.AlwaysNotify = true}
			: FStatePolicy {.NotifyOnChangeOnly = true, .StorePrevious = true}; 
	
	template <typename T>
	struct IState;
	
	template <typename T>
	struct TChangeData;
	
	template <typename T, FStatePolicy DefaultPolicy = StatePolicyFor<T>>
	struct TState;

	/**
	 *	@brief
	 *	Convenience alias for shared reference to a base type of TState. Use this in APIs which may modify or get the
	 *	value of a state declared elsewhere.
	 */
	template <typename T>
	using TStateRef = TSharedRef<IState<T>>;

	/**
	 *	@brief
	 *	Convenience alias for shared pointer to a base type of TState. Use this in APIs which may modify or get the
	 *	value of a state declared elsewhere.
	 */
	template <typename T>
	using TStatePtr = TSharedPtr<IState<T>>;

	/**
	 *	@brief
	 *	Convenience alias for weak pointer to a base type of TState. Use this in APIs which may modify or get the
	 *	value of a state declared elsewhere.
	 */
	template <typename T>
	using TStateWeakPtr = TWeakPtr<IState<T>>;

	/** @brief Convenience alias for declaring a state as a shared reference. Use this only as object members */
	template <typename T, FStatePolicy DefaultPolicy = StatePolicyFor<T>>
	using TDeclareStateRef = TSharedRef<TState<T, DefaultPolicy>>;

	/** @brief Convenience alias for declaring a state as a shared pointer. Use this only as object members */
	template <typename T, FStatePolicy DefaultPolicy = StatePolicyFor<T>>
	using TDeclareStatePtr = TSharedPtr<TState<T, DefaultPolicy>>;

	/** @brief Convenience alias for declaring a thread-safe state as a shared reference. Use this only as object members */
	template <typename T, FStatePolicy DefaultPolicy = StatePolicyFor<T>>
	using TDeclareStateTSRef = TSharedRef<TState<T, DefaultPolicy.With({.ThreadSafe = true})>>;

	/** @brief Convenience alias for declaring a thread-safe state as a shared pointer. Use this only as object members */
	template <typename T, FStatePolicy DefaultPolicy = StatePolicyFor<T>>
	using TDeclareStateTSPtr = TSharedPtr<TState<T, DefaultPolicy.With({.ThreadSafe = true})>>;

	/** @brief Concept constraining given type to a state */
	template <typename T>
	concept CState = CDerivedFrom<T, IStateTag>;

	namespace Detail
	{
		template <typename Function, typename T>
		concept CChangeListenerCandidate = CFunctionLike<Function>
			&& TFunction_ArgCount<Function> > 0
			&& CConvertibleTo<T, TFunction_ArgDecay<Function, 0>>
		;
	}


	/** @brief Concept describing a function which can listen to changes to the current value of a TState only */
	template <typename Function, typename T>
	concept CChangeNextOnlyListener = Detail::CChangeListenerCandidate<Function, T> && TFunction_ArgCount<Function> == 1;

	/** @brief Concept describing a function which can listen to changes to the current and the previous values of a TState */
	template <typename Function, typename T>
	concept CChangeNextPreviousListener = Detail::CChangeListenerCandidate<Function, T>
		&& TFunction_ArgCount<Function> == 2
		&& CConvertibleTo<TOptional<T>, TFunction_ArgDecay<Function, 1>>
	;

	/** @brief Concept describing a function which can be a change listener on a TState */
	template <typename Function, typename T>
	concept CChangeListener = CChangeNextOnlyListener<Function, T> || CChangeNextPreviousListener<Function, T>;

	/** @brief Convenience alias for thread safe states */
	template <typename T, FStatePolicy DefaultPolicy = StatePolicyFor<T>>
	using TStateTS = TState<T, DefaultPolicy.With({.ThreadSafe = true})>;

	/** @brief Convenience alias for boolean states */
	using FBool = TState<bool>;
	
	/** @brief Convenience alias for thread-safe boolean states */
	using FBoolTS = TStateTS<bool>;
}
