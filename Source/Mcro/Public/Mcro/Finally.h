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

namespace Mcro::Finally
{
	/**
	 *	@brief
	 *	Run arbitrary finalizers on destruction. It has similar purpose to `ON_SCOPE_EXIT`, however FFinally can be
	 *	moved around into different scopes, Payload will be only executed if the finalizer hasn't been moved from,
	 *	otherwise the payload will be ignored in the source of move assignments
	 *
	 *	Usage:
	 *	@code
	 *	{
	 *		FFinally fin([] {...})
	 *		Async(EAsyncExecution::ThreadPool, [fin = MoveTemp(fin)]
	 *		{
	 *			// Payload is executed at the end of this scope
	 *		}
	 *		// Payload is not executed here
	 *	}
	 *	@endcode
	 *	Finalizers can be moved into nested scopes
	 *	@code
	 *	{
	 *		FFinally fin([] {...})
	 *		Async(EAsyncExecution::ThreadPool, [fin = MoveTemp(fin)] mutable
	 *		{
	 *			AsyncTask(ENamedThreads::GameThread, [fin = MoveTemp(fin)]
	 *			{
	 *				// Payload is executed at the end of this scope
	 *			}
	 *			// Payload is not executed here
	 *		}
	 *		// Payload is not executed here
	 *	}
	 *	@endcode 
	 */
	struct FFinally : FNoncopyable
	{
	private:
		TUniqueFunction<void()> Payload;
		bool bIsValid = true;
	
	public:
		FFinally(TUniqueFunction<void()>&& payload) : Payload(MoveTemp(payload)) {}
		FFinally(FFinally&& from) noexcept : Payload(MoveTemp(from.Payload))
		{
			from.bIsValid = false;
		}
	
		~FFinally()
		{
			if(bIsValid) Payload();
		}
	};

	/**
	 *	@warning
	 *	Do not use this directly, use FINALLY macro. Similar intentions to ON_SCOPE_EXIT but gives more freedom
	 */
	struct FFinallySyntaxSupport
	{
		FFinally operator+(TUniqueFunction<void()>&& payload)
		{
			return FFinally(MoveTemp(payload));
		}
	};
}

/**
 *	A more convenient way to use `FFinally`.
 *
 *	@code
 *	auto fin = FINALLY(=, this) mutable
 *	{
 *		...
 *	};
 *	AsyncTask(ENamedThreads::GameThread, [fin = MoveTemp(fin)]
 *	{
 *		// Payload is executed at the end of this scope
 *	}
 *	@endcode 
 */
#define FINALLY(...) Mcro::Finally::FFinallySyntaxSupport() + [__VA_ARGS__]()
