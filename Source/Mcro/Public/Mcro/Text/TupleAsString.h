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

#include "Mcro/Text.h"
#include "Mcro/Tuples.h"

namespace Mcro::Text
{
	using namespace Mcro::Tuples;

	/** @brief Convert Unreal/STL/Range-V3 tuples to string the following way: `(Item0, Item1, Item2, ...)`*/
	template <CTuple Operand>
	struct TAsFormatArgument<Operand>
	{
		template <typename Tuple, size_t... Indices>
		static FString Render(Tuple const& tuple, std::index_sequence<Indices...>&&)
		{
			auto body = Join(TEXT_", ", AsString(GetItem<Indices>(tuple))...);
			return TEXT_"(" + body + TEXT_")";
		}
		
		template <CConvertibleToDecayed<Operand> Arg>
		FString operator () (Arg&& left) const
		{
			return Render(left, TIndexSequenceForTuple<Operand>());
		}
	};
}
