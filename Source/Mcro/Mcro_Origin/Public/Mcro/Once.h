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

namespace Mcro::Once
{
	/**
	 *	@brief 
	 *	Used for lambdas which supposed to run only once. The first time it is converted to bool it returns true
	 *	but every other times it will return false.
	 *	
	 *	Usage:
	 *	@code
	 *	using namespace BaseUtils;
	 *	SomeEvent.AddLambda([once = FOnce()]
	 *	{
	 *	    // Just use it inside a condition
	 *	    if (once) { ... }
	 *	    
	 *	    // exploiting short-circuit:
	 *	    if (MyCondition && once) { ... }
	 *	
	 *	    // but this will not produce the expected result:
	 *	    if (once && MyCondition) { ... } // BAD
	 *	}
	 *	@endcode
	 */
	struct FOnce
	{
	private:
		bool bIsValid = true;
        
	public:
		FORCEINLINE FOnce() {}
		FORCEINLINE FOnce(const FOnce& from) : bIsValid(from.bIsValid) {}
		FORCEINLINE FOnce(FOnce&& from) noexcept
		{
			from.bIsValid = false;
		}

		FORCEINLINE operator bool()
		{
			const bool result = bIsValid;
			bIsValid = false;
			return result;
		}

		FORCEINLINE bool IsTriggered() const
		{
			return !bIsValid;
		}

		FORCEINLINE void Reset()
		{
			bIsValid = true;
		}
	};
}
