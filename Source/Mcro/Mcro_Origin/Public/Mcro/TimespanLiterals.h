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
			static TStaticArray<char, sizeof...(ValueText) + 1> string { ValueText..., '\0' };
			double value = TCString<char>::Atod(string.GetData());
			return Function(value);
		}
	}
	
	template <char... Value>
	constexpr FTimespan operator ""_D ()
	{
		return Detail::CreateFromParamPack<&FTimespan::FromDays, Value...>();
	}
	
	template <char... Value>
	constexpr FTimespan operator ""_h ()
	{
		return Detail::CreateFromParamPack<&FTimespan::FromHours, Value...>();
	}
	
	template <char... Value>
	constexpr FTimespan operator ""_m ()
	{
		return Detail::CreateFromParamPack<&FTimespan::FromMinutes, Value...>();
	}
	
	template <char... Value>
	constexpr FTimespan operator ""_s ()
	{
		return Detail::CreateFromParamPack<&FTimespan::FromSeconds, Value...>();
	}
	
	template <char... Value>
	constexpr FTimespan operator ""_ms ()
	{
		return Detail::CreateFromParamPack<&FTimespan::FromMilliseconds, Value...>();
	}
	
	template <char... Value>
	constexpr FTimespan operator ""_us ()
	{
		return Detail::CreateFromParamPack<&FTimespan::FromMicroseconds, Value...>();
	}
}
