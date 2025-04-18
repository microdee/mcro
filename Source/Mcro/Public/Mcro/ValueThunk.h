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
#include "Mcro/Concepts.h"
#include "Mcro/FunctionTraits.h"

namespace Mcro::ValueThunk
{
	using namespace Mcro::Concepts;
	using namespace Mcro::FunctionTraits;
	
	/** @brief Options for value thunks */
	struct FValueThunkOptions
	{
		/**
		 *	@brief
		 *	If Memoize is true and associated value thunk is functional, cache the result of the first invocation.
		 *	This makes it basically a "lazy value"
		 */
		bool Memoize = false;
	};

	/**
	 *	@brief
	 *	Either a singular value of T, or a function returning T. It's very similar to TAttribute, however it doesn't
	 *	rely on delegates, and has implicit conversions. TValueThunk owns its wrapped value.
	 */
	template <typename T>
	struct TValueThunk
	{
		template <typename Other>
		friend TValueThunk<Other>;
		
		using FunctionStorage = std::conditional_t<CCopyConstructible<T>, TFunction<T()>, TUniqueFunction<T()>>;

		template <CDefaultInitializable = T>
		TValueThunk() : bIsSet(true) {}

		template <CCopyConstructible = T>
		TValueThunk(T const& value) : Storage(value), bIsSet(true) {};

		template <CConvertibleTo<T> Other, CCopyConstructible = T>
		TValueThunk(TValueThunk<Other> const& other)
			: bIsSet(other.bIsSet)
			, Options(other.Options)
			, Function(other.Function)
		{
			if (other.bIsSet) Storage = other.Storage;
		};

		template <CMoveConstructible = T>
		TValueThunk(T&& value) : Storage(Forward<T>(value)), bIsSet(true) {};
		
		template <CMoveConstructible = T>
		TValueThunk(TValueThunk&& other)
			: bIsSet(other.bIsSet)
			, Options(MoveTemp(other.Options))
			, Function(MoveTemp(other.Function))
		{
			if (other.bIsSet) Storage = MoveTemp(other.Storage);
		};

		template <CFunctorObject Functor>
		requires CConvertibleTo<TFunction_ReturnDecay<Functor>, T>
		TValueThunk(Functor&& value, FValueThunkOptions const& options = {})
			: bIsSet(true)
			, Options(options)
			, Function(Forward<Functor>(value))
		{};

	private:
		void Evaluate() const
		{
			if (Function.IsSet() && (!Options.Memoize || !bIsSet))
			{
				Storage = Function();
				bIsSet = true;
			}
		}
		
	public:
		/** @brief Evaluate the optional functor and get the cached result */
		T& Get() { Evaluate(); return Storage; }
		
		/** @brief Evaluate the optional functor and get the cached result */
		T const& Get() const { Evaluate(); return Storage; }
		
		/** @brief Evaluate the optional functor and move the cached result */
		T&& Steal() && { Evaluate(); return MoveTemp(Storage); }

		/** @brief Get a cached/last result without calling the optional functor */
		T& GetLast() { return Storage; }
		
		/** @brief Get a cached/last result without calling the optional functor */
		T const& GetLast() const { return Storage; }
		
		/** @brief Move a cached/last result without calling the optional functor */
		T&& StealLast() && { return MoveTemp(Storage); }

		operator T&       ()       { return Get(); }
		operator T const& () const { return Get(); }
		operator T&&      () &&    { return Steal(); }

		bool IsSet() const { return bIsSet; }
		
	private:
		mutable T Storage {};
		mutable bool bIsSet = false;
		FValueThunkOptions Options {};
		FunctionStorage Function {};
	};
}
