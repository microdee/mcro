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
#include "Mcro/UObjects/Init.h"

namespace Mcro::UObjects::Init
{
	/**
	 *	@brief
	 *	A struct to emulate regular C++ RAII object behavior with UObjects. When this struct is instantiated the given
	 *	object type is also created. An object wrapped in this struct is never invalid, and doesn't get garbage
	 *	collected until it's in scope.
	 *
	 *	It is safe to create this struct on any thread, GC is deferred until constructor is finished. The underlying
	 *	storage is a `TStrongObjectPtr`.
	 *
	 *	If the given object type happen to have an `Intialize` member method, that's also called with the extra
	 *	arguments provided in the constructor.
	 */
	template <CUObject T>
	struct TScopeObject
	{
		template <typename... Args>
		TScopeObject(FConstructObjectParameters&& params, Args&&... args)
		{
			FGCScopeGuard avoidGC;
			Storage.Reset(NewInit<T>(Forward<FConstructObjectParameters>(params), Forward<Args>(args)...));
		}

		const T* operator -> () const { return Storage.Get(); }
		      T* operator -> ()       { return Storage.Get(); }

		operator const T* () const { return Storage.Get(); }
		operator       T* ()       { return Storage.Get(); }

		T const& Get() const { return *Storage.Get(); }
		T&       Get()       { return *Storage.Get(); }
		
	private:
		TStrongObjectPtr<T> Storage;
	};
}