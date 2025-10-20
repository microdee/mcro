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

#include "Mcro/Text.h"
#include "Mcro/FunctionTraits.h"
#include "Mcro/SharedObjects.h"

namespace Mcro::Text
{
	using namespace Mcro::FunctionTraits;
	using namespace Mcro::SharedObjects;

	FString UnrealCopy(const FStdStringView& stdStr)
	{
		return FString(stdStr.data(), stdStr.size());
	}

	FName UnrealNameCopy(FStdStringView const& stdStr)
	{
		return FName(stdStr.length(), stdStr.data());
	}
	FName UnrealNameConvert(std::string_view const& stdStr)
	{
		return FName(stdStr.length(), stdStr.data());
	}
	FName UnrealNameConvert(std::wstring_view const& stdStr)
	{
		return FName(stdStr.length(), stdStr.data());
	}

	FStdString StdCopy(const FStringView& unrealStr)
	{
		return FStdString(unrealStr.GetData(), unrealStr.Len());
	}
	
	FStdString StdCopy(FName const& unrealStr)
	{
		return StdCopy(unrealStr.ToString());
	}
}
