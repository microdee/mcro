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

namespace Mcro::InitializeOnCopy
{
	using namespace Mcro::FunctionTraits;

	/**
	 *	@brief
	 *	A type wrapper around a default initializeable object which may not be copyable but which needs to be a member
	 *	of a copyable class. On each instance of such class the wrapped value may not need to be copied and default
	 *	constructing it is enough. Useful for mutexes for example.
	 */
	template <CDefaultInitializable T>
	requires (!CCopyable<T>)
	struct TInitializeOnCopy
	{
		TInitializeOnCopy() : Value() {}
		TInitializeOnCopy(TInitializeOnCopy const&) : Value() {}
		TInitializeOnCopy(TInitializeOnCopy&&) noexcept : Value() {}
		auto operator=(TInitializeOnCopy const&) -> TInitializeOnCopy& { return *this; }
		auto operator=(TInitializeOnCopy&& other) noexcept -> TInitializeOnCopy& { return *this; }

		TUniqueObj<T> Value;

		      T* operator -> ()       { return &Value.Get(); }
		const T* operator -> () const { return &Value.Get(); }
		
		      T& Get()       { return Value.Get(); }
		const T& Get() const { return Value.Get(); }
		
		template <typename Self>
		operator typename TCopyQualifiersFromTo<Self, T&>::Type (this Self&& self) { return self.Get(); }
	};
}