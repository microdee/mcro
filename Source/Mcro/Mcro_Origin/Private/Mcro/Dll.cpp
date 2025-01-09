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

#include "Mcro/Dll.h"

namespace Mcro::Dll
{
	
	FScopedSearchPath::FScopedSearchPath(FString const& path) : Path(path)
	{
		if (!path.IsEmpty()) FPlatformProcess::PushDllDirectory(*path);
	}
	FScopedSearchPath::~FScopedSearchPath()
	{
		if (!Path.IsEmpty()) FPlatformProcess::PopDllDirectory(*Path);
	}

	FScopedDll::FScopedDll(const TCHAR* fileName)
	{
		Handle = FPlatformProcess::GetDllHandle(fileName);
	}

	FScopedDll::~FScopedDll()
	{
		FPlatformProcess::FreeDllHandle(Handle);
	}
}
