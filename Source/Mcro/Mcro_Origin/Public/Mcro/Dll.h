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

namespace Mcro::Dll
{
	using namespace Mcro::Concepts;
	
	/** RAII wrapper around PushDllDirectory / PopDllDirectory */
	struct MCRO_API FScopedSearchPath
	{
		FScopedSearchPath(FString const& path);
		~FScopedSearchPath();
		
	private:
		FString Path;
	};
	
	/** RAII wrapper around GetDllHandle / FreeDllHandle */
	struct MCRO_API FScopedDll
	{
		FScopedDll(const TCHAR* fileName);
		~FScopedDll();
		
	private:
		void* Handle;
	};

	/** Handle multiple DLL files in one set and an optional base path for them */
	struct MCRO_API FScopedDllSet
	{
		FScopedDllSet() {}
		
		template <CConvertibleTo<const TCHAR*>... Args>
		FScopedDllSet(FString const& pushPath, Args... args)
		{
			FScopedSearchPath pathContext(pushPath);
			(Dlls.Emplace(args), ...);
		}
		
	private:

		TArray<FScopedDll> Dlls;
	};
}
