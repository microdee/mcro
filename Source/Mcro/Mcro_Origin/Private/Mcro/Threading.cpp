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

#include "Mcro/Threading.h"

namespace Mcro::Threading
{
	auto Detail::GetThreadCheck(ENamedThreads::Type threadName) -> bool(*)()
	{
		switch (threadName)
		{
		case ENamedThreads::RHIThread: return &IsInRHIThread;
		case ENamedThreads::GameThread: return &IsInGameThread;
		case ENamedThreads::ActualRenderingThread: return &IsInActualRenderingThread;
		case ENamedThreads::GameThread_Local: return &IsInGameThread;
		case ENamedThreads::ActualRenderingThread_Local: return &IsInActualRenderingThread;
		default: ensureMsgf(false, TEXT("GetThreadCheck cannot get this thread predicate."));
		}
		return nullptr;
	}

	bool IsInThread(ENamedThreads::Type threadName)
	{
		auto isInThreadPtr = Detail::GetThreadCheck(threadName);
		return isInThreadPtr && isInThreadPtr();
	}

	void RunInThread(ENamedThreads::Type threadName, TUniqueFunction<void()>&& func)
	{
		Detail::RunInThreadBoilerplate(threadName, MoveTemp(func), []{ return true; });
	}

	void RunInThread(ENamedThreads::Type threadName, const UObject* boundToObject, TUniqueFunction<void()>&& func)
	{
		Detail::RunInThreadBoilerplate(threadName, MoveTemp(func), [boundToObject]
		{
			return IsValid(boundToObject) ? TStrongObjectPtr(boundToObject) : nullptr;
		});
	}

	void RunInThread(ENamedThreads::Type threadName, const FWeakObjectPtr& boundToObject, TUniqueFunction<void()>&& func)
	{
		Detail::RunInThreadBoilerplate(threadName, MoveTemp(func), [boundToObject]
		{
			return TStrongObjectPtr(boundToObject.Get());
		});
	}

	void RunInGameThread(TUniqueFunction<void()>&& func)
	{
		RunInThread(ENamedThreads::GameThread, MoveTemp(func));
	}

	void RunInGameThread(const UObject* boundToObject, TUniqueFunction<void()>&& func)
	{
		RunInThread(ENamedThreads::GameThread, boundToObject, MoveTemp(func));
	}

	void RunInGameThread(const FWeakObjectPtr& boundToObject, TUniqueFunction<void()>&& func)
	{
		RunInThread(ENamedThreads::GameThread, boundToObject, MoveTemp(func));
	}

	void EnqueueRenderCommand(TUniqueFunction<void(FRHICommandListImmediate&)>&& func)
	{
	}

	void EnqueueRenderCommand(const UObject* boundToObject, TUniqueFunction<void(FRHICommandListImmediate&)>&& func)
	{
	}

	void EnqueueRenderCommand(const FWeakObjectPtr& boundToObject, TUniqueFunction<void(FRHICommandListImmediate&)>&& func)
	{
	}
}
