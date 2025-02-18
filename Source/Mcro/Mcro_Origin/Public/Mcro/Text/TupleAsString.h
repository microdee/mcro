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
	
	/** @brief Convert Unreal tuples to string the following way: `(Item0, Item1, Item2, ...)`*/
	template <typename Operand>
	requires TIsTuple_V<Operand>
	struct TAsFormatArgument<Operand>
	{
		template <typename Tuple, size_t... Indices>
		FString Render(Tuple const& tuple, std::index_sequence<Indices...>&&)
		{
			auto body = Join(TEXT_", ", AsString(tuple.template Get<Indices>())...);
			return TEXT_"(" + body + TEXT_")";
		}
		
		template <CConvertibleToDecayed<Operand> Arg>
		FString operator () (Arg&& left) const
		{
			return Render(left, std::make_index_sequence<TTupleArity<std::decay_t<Operand>>::Value>());
		}
	};

	/** @brief Convert STL tuples to string the following way: `(Item0, Item1, Item2, ...)`*/
	template <CStdTupleLike Operand>
	struct TAsFormatArgument<Operand>
	{
		template <typename Tuple, size_t... Indices>
		FString Render(Tuple const& tuple, std::index_sequence<Indices...>&&)
		{
			auto body = Join(TEXT_", ", AsString(std::get<Indices>(tuple))...);
			return TEXT_"(" + body + TEXT_")";
		}
		
		template <CConvertibleToDecayed<Operand> Arg>
		FString operator () (Arg&& left) const
		{
			return Render(left, std::make_index_sequence<std::tuple_size_v<std::decay_t<Operand>>>());
		}
	};

	/** @brief Convert Range-V3 tuples to string the following way: `(Item0, Item1, Item2, ...)`*/
	template <CRangeV3TupleLike Operand>
	struct TAsFormatArgument<Operand>
	{
		template <typename Tuple, size_t... Indices>
		FString Render(Tuple const& tuple, std::index_sequence<Indices...>&&)
		{
			auto body = Join(TEXT_", ", AsString(ranges::get<Indices>(tuple))...);
			return TEXT_"(" + body + TEXT_")";
		}
		
		template <CConvertibleToDecayed<Operand> Arg>
		FString operator () (Arg&& left) const
		{
			return Render(left, std::make_index_sequence<std::tuple_size_v<std::decay_t<Operand>>>());
		}
	};
}
