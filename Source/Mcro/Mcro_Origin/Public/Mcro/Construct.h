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

namespace Mcro::Construct
{
	using namespace Mcro::FunctionTraits;
	
	/**
	 *	Simply makes a new object and allows to initialize it in place with a lambda function. The object type is derived
	 *	from the first argument of the initializer lambda function.
	 *	Usage:
	 *
	 *	@code
	 *	using namespace Mcro::Construct;
	 *	
	 *	auto myObject = Construct([](MyObject& _)
	 *	{
	 *		_.Foo = 42;
	 *		_.Initialize();
	 *		// etc...
	 *	});
	 *	static_assert(std::is_same_v<decltype(myObject), MyObject>);
	 *	@endcode
	 *
	 *	@param init A lambda function with a single l-value reference parameter of the object type to initialize.
	 *	@param args
	 *	@return A new instance of the object.
	 *	@remarks The C++ 20 designated initializers with named arguments has annoying limitations, therefore this exists
	 */
	template<
		CFunctorObject Initializer,
		typename... Args,
		typename ResultArg = TFunction_Arg<Initializer, 0>,
		typename Result = std::decay_t<ResultArg>
	>
	requires std::is_lvalue_reference_v<ResultArg>
	Result Construct(Initializer&& init, Args&&... args)
	{
		Result result {Forward<Args>(args)...};
		init(result);
		return result;
	}
	
	/**
	 *	Simply makes a new object on the heap and allows to initialize it in place with a lambda function. The object
	 *	type is derived from the first argument of the initializer lambda function.
	 *	Usage:
	 *
	 *	@code
	 *	using namespace Mcro::Construct;
	 *	
	 *	auto myObject = ConstructNew([](MyObject& _)
	 *	{
	 *		_.Foo = 42;
	 *		_.Initialize();
	 *		// etc...
	 *	});
	 *	static_assert(std::is_same_v<decltype(myObject), MyObject*>);
	 *	@endcode
	 *
	 *	@param init A lambda function with a single l-value reference parameter of the object type to initialize.
	 *	@param args
	 *	@return A pointer to the object instance on heap.
	 *	@remarks The C++ 20 designated initializers with named arguments has annoying limitations, therefore this exists
	 */
	template<
		CFunctorObject Initializer,
		typename... Args,
		typename ResultArg = TFunction_Arg<Initializer, 0>,
		typename Result = std::decay_t<ResultArg>
	>
	requires std::is_lvalue_reference_v<ResultArg>
	Result* ConstructNew(Initializer&& init, Args&&... args)
	{
		Result* result = new Result {Forward<Args>(args)...};
		init(*result);
		return result;
	}

	/**
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
