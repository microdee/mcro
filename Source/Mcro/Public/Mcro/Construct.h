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
	 *	@brief
	 *	Simply makes a new object and allows to initialize it in place with a lambda function. The object type is derived
	 *	from the first argument of the initializer lambda function.
	 *	
	 *	Usage:
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
	 *	@param    init  A lambda function with a single l-value reference parameter of the object type to initialize.
	 *	@param    args  Arguments of the object constructor
	 *	@return   A new instance of the object.
	 *	@remarks  The C++ 20 designated initializers with named arguments has annoying limitations, therefore this exists
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
		Result result {FWD(args)...};
		init(result);
		return result;
	}
	
	/**
	 *	@brief
	 *	Simply makes a new object on the heap and allows to initialize it in place with a lambda function. The object
	 *	type is derived from the first argument of the initializer lambda function.
	 *	
	 *	Usage:
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
	 *	@param    init  A lambda function with a single l-value reference parameter of the object type to initialize.
	 *	@param    args  Arguments of the object constructor
	 *	@return   A pointer to the object instance on heap.
	 *	@remarks  The C++ 20 designated initializers with named arguments has annoying limitations, therefore this exists
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
		Result* result = new Result {FWD(args)...};
		init(*result);
		return result;
	}
}
