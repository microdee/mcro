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
#include "Misc/Timespan.h"

namespace Mcro::Timespan::Literals
{
	using namespace Mcro::Concepts;

	namespace Detail
	{
		template <auto Function, char... ValueText>
		constexpr FTimespan CreateFromParamPack()
		{
			static constexpr char string [sizeof...(ValueText) + 1] { ValueText..., '\0' };
			double value = TCString<char>::Atod(string);
			return Function(value);
		}
	}
	
	template <char... Value>
	constexpr FTimespan operator ""_Day ()
	{
		return Detail::CreateFromParamPack<&FTimespan::FromDays, Value...>();
	}
	
	template <char... Value>
	constexpr FTimespan operator ""_Hour ()
	{
		return Detail::CreateFromParamPack<&FTimespan::FromHours, Value...>();
	}
	
	template <char... Value>
	constexpr FTimespan operator ""_Min ()
	{
		return Detail::CreateFromParamPack<&FTimespan::FromMinutes, Value...>();
	}
	
	template <char... Value>
	constexpr FTimespan operator ""_Sec ()
	{
		return Detail::CreateFromParamPack<&FTimespan::FromSeconds, Value...>();
	}
	
	template <char... Value>
	constexpr FTimespan operator ""_mSec ()
	{
		return Detail::CreateFromParamPack<&FTimespan::FromMilliseconds, Value...>();
	}
	
	template <char... Value>
	constexpr FTimespan operator ""_uSec ()
	{
		return Detail::CreateFromParamPack<&FTimespan::FromMicroseconds, Value...>();
	}
}
