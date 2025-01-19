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
	enum EStatePolicy
	{
		/**
		 *	@brief
		 *	When the object inside TState is != comparable TState wull only emit change events when the submitted
		 *	value differs from the existing one.
		 */
		NotifyOnChangeOnly = 0,

		/** @brief Always emit change notification when a value is set on TState and don't attempt to compare them */
		AlwaysNotify = 1 << 0,

		/** @brief Always emit change notification when a value is set on TState and don't attempt to compare them */
		StorePrevious = 1 << 1,

		/**
		 *	@brief
		 *	Enable mutexes during modifications, notifications and expose a public critical section for users
		 *	of the state.
		 */
		ThreadSafeState = 1 << 2
	};

	struct IStateTag {};

	template <typename T>
	inline constexpr int32 StatePolicyFor =
		CClass<T>
			? CCoreEqualityComparable<T>
				? NotifyOnChangeOnly
				: AlwaysNotify
			: StorePrevious | NotifyOnChangeOnly;
	
	template <typename T>
	struct IState;
	
	template <typename T>
	struct TChangeData;
	
	template <typename T, int32 DefaultPolicy = StatePolicyFor<T>>
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
	template <typename T, int32 DefaultPolicy = StatePolicyFor<T>>
	using TDeclareStateRef = TSharedRef<TState<T, DefaultPolicy>>;

	/** @brief Convenience alias for declaring a state as a shared pointer. Use this only as object members */
	template <typename T, int32 DefaultPolicy = StatePolicyFor<T>>
	using TDeclareStatePtr = TSharedPtr<TState<T, DefaultPolicy>>;

	/** @brief Concept constraining given type to a state */
	template <typename T>
	concept CState = CDerivedFrom<T, IStateTag>;

	/** @brief Concept describing a function which can be a change listener on a TState */
	template <typename Function, typename T>
	concept CChangeListener = CFunctionLike<Function>
		&& TFunction_ArgCount<Function> > 0
		&& CConvertibleTo<T, TFunction_ArgDecay<Function, 0>>
	;

	/** @brief Concept describing a function which can listen to changes to the current value of a TState only */
	template <typename Function, typename T>
	concept CChangeNextOnlyListener = CChangeListener<Function, T> && TFunction_ArgCount<Function> == 1;

	/** @brief Concept describing a function which can listen to changes to the current and the previous values of a TState */
	template <typename Function, typename T>
	concept CChangeNextPreviousListener = CChangeListener<Function, T>
		&& TFunction_ArgCount<Function> == 2
		&& CConvertibleTo<TOptional<T>, TFunction_ArgDecay<Function, 1>>
	;

	/** @brief Convenience alias for thread safe states */
	template <typename T, int32 DefaultPolicy = StatePolicyFor<T> | ThreadSafeState>
	using TStateTS = TState<T, DefaultPolicy>;

	/** @brief Convenience alias for boolean states */
	using FBool = TState<bool>;
	
	/** @brief Convenience alias for thread-safe boolean states */
	using FBoolTS = TStateTS<bool>;
}
