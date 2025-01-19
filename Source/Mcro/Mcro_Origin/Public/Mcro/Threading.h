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
#include "Mcro/SharedObjects.h"

namespace Mcro::Threading
{
	using namespace Mcro::FunctionTraits;
	using namespace Mcro::SharedObjects;

	/**
	 *	@brief
	 *	Returns true when called in the thread which is associated with the given ENamedThreads.
	 *	If there's no such a thread or if this function is not called in that thread, return false.
	 */
	MCRO_API bool IsInThread(ENamedThreads::Type threadName);

	namespace Detail
	{
		MCRO_API auto GetThreadCheck(ENamedThreads::Type threadName) -> bool(*)();

		template <CFunctionLike When>
		requires (TFunction_ArgCount<When> == 0)
		void RunInThreadBoilerplate(
			ENamedThreads::Type threadName,
			TUniqueFunction<void()>&& func, When&& when
		) {
			if (IsInThread(threadName)) func();
			else AsyncTask(threadName, [when = MoveTemp(when), func = MoveTemp(func)]
			{
				if (auto keep = when()) func();
			});
		}

		template <CFunctionLike When>
		requires (TFunction_ArgCount<When> == 0)
		void EnqueueRenderCommandBoilerplate(TUniqueFunction<void(FRHICommandListImmediate&)>&& func, When&& when)
		{
			if (IsInRenderingThread()) func(GetImmediateCommandList_ForRenderCommand());
			else
			{
				ENQUEUE_RENDER_COMMAND(FMcroThreading)([when = MoveTemp(when), func = MoveTemp(func)](FRHICommandListImmediate& cmdList)
				{
					if (auto keep = when()) func(cmdList);
				});
			}
		}

		template <CFunctorObject Function, typename Result = TFunction_Return<Function>, CFunctionLike When>
		requires (
			TFunction_ArgCount<Function> == 0
			&& TFunction_ArgCount<When> == 0
		)
		TFuture<Result> PromiseInThreadBoilerplate(
			ENamedThreads::Type threadName,
			Function&& func,
			When&& when
		) {
			if (IsInThread(threadName))
				return MakeFulfilledPromise<Result>(func()).GetFuture();

			TPromise<Result> promise;
			auto future = promise.GetFuture();
			
			AsyncTask(threadName, [when = MoveTemp(when), func = MoveTemp(func), promise = MoveTemp(promise)]() mutable 
			{
				if (auto keep = when()) promise.SetValue(func());
				else promise.SetValue({});
			});
			return future;
		}

		template <CFunctorObject Function, typename Result = TFunction_Return<Function>, CFunctionLike When>
		requires (
			TFunction_ArgCount<Function> == 1
			&& CSameAs<FRHICommandListImmediate&, TFunction_Arg<Function, 0>>
			&& TFunction_ArgCount<When> == 0
		)
		TFuture<Result> EnqueueRenderPromiseBoilerplate(
			ENamedThreads::Type threadName,
			Function&& func,
			When&& when
		) {
			if (IsInRenderingThread())
				return MakeFulfilledPromise<Result>(func(GetImmediateCommandList_ForRenderCommand())).GetFuture();

			TPromise<Result> promise;
			auto future = promise.GetFuture();
			
			ENQUEUE_RENDER_COMMAND(FMcroThreading)([when = MoveTemp(when), func = MoveTemp(func), promise = MoveTemp(promise)](FRHICommandListImmediate& cmdList) mutable 
			{
				if (auto keep = when())
					promise.SetValue(func(cmdList));
				else promise.SetValue({});
			});
			return future;
		}
	}
	
	/**
	 *	@brief
	 *	Simply run a lambda function on the selected thread but only use AsyncTask if it's not on the selected thread
	 *	already
	 */
	MCRO_API void RunInThread(ENamedThreads::Type threadName, TUniqueFunction<void()>&& func);
	
	/**
	 *	@brief
	 *	Simply run a lambda function on the selected thread but only use AsyncTask if it's not on the selected thread
	 *	already check the validity of a target object first before running on the selected thread.
	 */
	MCRO_API void RunInThread(ENamedThreads::Type threadName, const UObject* boundToObject, TUniqueFunction<void()>&& func);
	
	/**
	 *	@brief
	 *	Simply run a lambda function on the selected thread but only use AsyncTask if it's not on the selected thread
	 *	already check the validity of a target object first before running on the selected thread.
	 */
	MCRO_API void RunInThread(ENamedThreads::Type threadName, const FWeakObjectPtr& boundToObject, TUniqueFunction<void()>&& func);
	
	/**
	 *	@brief
	 *	Simply run a lambda function on the game thread but only use AsyncTask if it's not on the game thread already
	 */
	MCRO_API void RunInGameThread(TUniqueFunction<void()>&& func);
	
	/**
	 *	@brief
	 *	Simply run a lambda function on the game thread but only use AsyncTask if it's not on the game thread already
	 *	Check the validity of a target object first before running on the game thread.
	 */
	MCRO_API void RunInGameThread(const UObject* boundToObject, TUniqueFunction<void()>&& func);
	
	/**
	 *	@brief
	 *	Simply run a lambda function on the game thread but only use AsyncTask if it's not on the game thread already
	 *	Check the validity of a target object first before running on the game thread.
	 */
	MCRO_API void RunInGameThread(const FWeakObjectPtr& boundToObject, TUniqueFunction<void()>&& func);
	
	/**
	 *	@brief
	 *	Simply run a lambda function on the render thread but only use AsyncTask if it's not on the render thread already
	 */
	MCRO_API void EnqueueRenderCommand(TUniqueFunction<void(FRHICommandListImmediate&)>&& func);
	
	/**
	 *	@brief
	 *	Simply run a lambda function on the render thread but only use AsyncTask if it's not on the render thread already
	 *	Check the validity of a target object first before running on the render thread.
	 */
	MCRO_API void EnqueueRenderCommand(const UObject* boundToObject, TUniqueFunction<void(FRHICommandListImmediate&)>&& func);
	
	/**
	 *	@brief
	 *	Simply run a lambda function on the render thread but only use AsyncTask if it's not on the render thread already
	 *	Check the validity of a target object first before running on the render thread.
	 */
	MCRO_API void EnqueueRenderCommand(const FWeakObjectPtr& boundToObject, TUniqueFunction<void(FRHICommandListImmediate&)>&& func);

	/**
	 *	@brief
	 *	Simply run a lambda function on the selected thread but only use AsyncTask if it's not on the selected thread
	 *	already Check the validity of a target object first before running on the selected thread.
	 */
	template <CSharedOrWeak Object>
	void RunInThread(ENamedThreads::Type threadName, const Object& boundToObject, TUniqueFunction<void()>&& func)
	{
		TWeakPtrFrom<Object> weakObject(boundToObject);
		Detail::RunInThreadBoilerplate(threadName, MoveTemp(func), [weakObject = MoveTemp(weakObject)]
		{
			return weakObject.Pin();
		});
	}

	/**
	 *	@brief
	 *	Simply run a lambda function on the game thread but only use AsyncTask if it's not on the game thread already
	 *	Check the validity of a target object first before running on the game thread.
	 */
	template <CSharedOrWeak Object>
	void RunInGameThread(const Object& boundToObject, TUniqueFunction<void()>&& func)
	{
		RunInThread<Object>(ENamedThreads::GameThread, boundToObject, MoveTemp(func));
	}
	
	/**
	 *	@brief
	 *	Simply run a lambda function on the game thread but only use AsyncTask if it's not on the game thread already
	 *	Check the validity of a target object first before running on the game thread.
	 */
	template <CSharedOrWeak Object>
	void EnqueueRenderCommand(const Object& boundToObject, TUniqueFunction<void(FRHICommandListImmediate&)>&& func)
	{
		TWeakPtrFrom<Object> weakObject(boundToObject);
		Detail::EnqueueRenderCommandBoilerplate(MoveTemp(func), [weakObject = MoveTemp(weakObject)]
		{
			return weakObject.Pin();
		});
	}

	/**
	 *	@brief
	 *	Simply run a lambda function on the selected thread but only use AsyncTask if it's not on the selected thread
	 *	already This overload doesn't check object lifespans.
	 */
	template <
		CFunctorObject Function,
		typename Result = TFunction_Return<Function>
	>
	requires (TFunction_ArgCount<Function> == 0)
	TFuture<Result> PromiseInThread(ENamedThreads::Type threadName, Function&& func)
	{
		return Detail::PromiseInThreadBoilerplate(threadName, MoveTemp(func), []{ return true; });
	}

	/**
	 *	@brief
	 *	Simply run a lambda function on the selected thread but only use AsyncTask if it's not on the selected thread
	 *	already Check the validity of a target object first before running on the selected thread.
	 */
	template <
		CSharedOrWeak Object,
		CFunctorObject Function,
		typename Result = TFunction_Return<Function>
	>
	requires (TFunction_ArgCount<Function> == 0)
	TFuture<Result> PromiseInThread(ENamedThreads::Type threadName, const Object& boundToObject, Function&& func)
	{
		TWeakPtrFrom<Object> weakObject(boundToObject);
		return Detail::PromiseInThreadBoilerplate(threadName, MoveTemp(func), [weakObject = MoveTemp(weakObject)]
		{
			return weakObject.Pin();
		});
	}

	/**
	 *	@brief
	 *	Simply run a lambda function on the selected thread but only use AsyncTask if it's not on the selected thread
	 *	already Check the validity of a target object first before running on the selected thread.
	 */
	template <
		CUObject Object,
		CFunctorObject Function,
		typename Result = TFunction_Return<Function>
	>
	requires (TFunction_ArgCount<Function> == 0)
	TFuture<Result> PromiseInThread(ENamedThreads::Type threadName, const Object* boundToObject, Function&& func)
	{
		return Detail::PromiseInThreadBoilerplate(threadName, MoveTemp(func), [boundToObject]
		{
			return IsValid(boundToObject) ? TStrongObjectPtr(boundToObject) : nullptr;
		});
	}

	/**
	 *	@brief
	 *	Simply run a lambda function on the game thread but only use AsyncTask if it's not on the game thread already
	 *	This overload doesn't check object lifespans.
	 */
	template <
		CFunctorObject Function,
		typename Result = TFunction_Return<Function>
	>
	requires (TFunction_ArgCount<Function> == 0)
	TFuture<Result> PromiseInGameThread(Function&& func)
	{
		return Detail::PromiseInThreadBoilerplate(ENamedThreads::GameThread, MoveTemp(func), []{ return true; });
	}

	/**
	 *	@brief
	 *	Simply run a lambda function on the game thread but only use AsyncTask if it's not on the game thread already
	 *	Check the validity of a target object first before running on the game thread.
	 */
	template <
		CSharedOrWeak Object,
		CFunctorObject Function,
		typename Result = TFunction_Return<Function>
	>
	requires (TFunction_ArgCount<Function> == 0)
	TFuture<Result> PromiseInGameThread(const Object& boundToObject, Function&& func)
	{
		TWeakPtrFrom<Object> weakObject(boundToObject);
		return Detail::PromiseInThreadBoilerplate(ENamedThreads::GameThread, MoveTemp(func), [weakObject = MoveTemp(weakObject)]
		{
			return weakObject.Pin();
		});
	}

	/**
	 *	@brief
	 *	Simply run a lambda function on the game thread but only use AsyncTask if it's not on the game thread already
	 *	Check the validity of a target object first before running on the game thread.
	 */
	template <
		CUObject Object,
		CFunctorObject Function,
		typename Result = TFunction_Return<Function>
	>
	requires (TFunction_ArgCount<Function> == 0)
	TFuture<Result> PromiseInGameThread(const Object* boundToObject, Function&& func)
	{
		return Detail::PromiseInThreadBoilerplate(ENamedThreads::GameThread, MoveTemp(func), [boundToObject]
		{
			return IsValid(boundToObject) ? TStrongObjectPtr(boundToObject) : nullptr;
		});
	}

	/**
	 *	@brief
	 *	Simply run a lambda function on the render thread but only use AsyncTask if it's not on the render thread already
	 *	This overload doesn't check object lifespans.
	 */
	template <
		CFunctorObject Function,
		typename Result = TFunction_Return<Function>
	>
	requires (
		TFunction_ArgCount<Function> == 1
		&& CSameAs<FRHICommandListImmediate&, TFunction_Arg<Function, 0>>
	)
	TFuture<Result> EnqueueRenderPromise(Function&& func)
	{
		return Detail::EnqueueRenderPromiseBoilerplate(ENamedThreads::GameThread, MoveTemp(func), []{ return true; });
	}

	/**
	 *	@brief 
	 *	Simply run a lambda function on the render thread but only use AsyncTask if it's not on the render thread already
	 *	Check the validity of a target object first before running on the render thread.
	 */
	template <
		CSharedOrWeak Object,
		CFunctorObject Function,
		typename Result = TFunction_Return<Function>
	>
	requires (
		TFunction_ArgCount<Function> == 1
		&& CSameAs<FRHICommandListImmediate&, TFunction_Arg<Function, 0>>
	)
	TFuture<Result> EnqueueRenderPromise(const Object& boundToObject, Function&& func)
	{
		TWeakPtrFrom<Object> weakObject(boundToObject);
		return Detail::EnqueueRenderPromiseBoilerplate(ENamedThreads::GameThread, MoveTemp(func), [weakObject = MoveTemp(weakObject)]
		{
			return weakObject.Pin();
		});
	}

	/**
	 *	@brief 
	 *	Simply run a lambda function on the render thread but only use AsyncTask if it's not on the render thread already
	 *	Check the validity of a target object first before running on the render thread.
	 */
	template <
		CUObject Object,
		CFunctorObject Function,
		typename Result = TFunction_Return<Function>
	>
	requires (
		TFunction_ArgCount<Function> == 1
		&& CSameAs<FRHICommandListImmediate&, TFunction_Arg<Function, 0>>
	)
	TFuture<Result> EnqueueRenderPromise(const Object* boundToObject, Function&& func)
	{
		return Detail::EnqueueRenderPromiseBoilerplate(ENamedThreads::GameThread, MoveTemp(func), [boundToObject]
		{
			return IsValid(boundToObject) ? TStrongObjectPtr(boundToObject) : nullptr;
		});
	}
}
