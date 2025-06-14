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
#include "Mcro/Macros.h"
#include "HAL/MallocAnsi.h"

namespace Mcro::Ansi
{
	/**
	 *	@brief  Force using the ANSI memory allocation behavior, instead of the Unreal default.
	 *
	 *	This may be less performant, may need more memory and will not be considered by AutoRTFM. Use it only when Unreal
	 *	overrides cause problems and comment the reason why this was necessary. This is paired with `Ansi::Delete` instead
	 *	of `delete`.
	 *	@code
	 *	FFoobar* myVar = Ansi::New<FFoobar>();
	 *	Ansi::Delete(myVar);
	 *	@endcode 
	 */
	template <typename T, typename... Args>
	T* New(Args&&... args)
	{
		T* result = static_cast<T*>(AnsiMalloc(sizeof(T), alignof(T)));
		return new (result) T(FWD(args)...);
	}
	
	/**
	 *	@brief  Force using the ANSI memory release behavior, instead of the Unreal default.
	 *
	 *	This may be less performant and will not be considered by AutoRTFM. Use it only when Unreal overrides cause
	 *	problems and comment the reason why this was necessary. This is paired with `Ansi::New<T>()`.
	 *	@code
	 *	FFoobar* myVar = Ansi::New<FFoobar>();
	 *	Ansi::Delete(myVar);
	 *	@endcode
	 */
	template <typename T>
	void Delete(T* ptr)
	{
		if (!ptr) return;
		ptr->~T();
		AnsiFree(ptr);
	}
}