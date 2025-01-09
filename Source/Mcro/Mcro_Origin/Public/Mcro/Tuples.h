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

/** Templating utilities for manipulating `TTuple`s */
namespace Mcro::Tuples
{
	/** Compose one tuple out of the elements of another tuple based on the input index parameter pack */
	template <typename Tuple, size_t... Indices>
	using TComposeFrom = TTuple<typename TTupleElement<Indices, Tuple>::Type...>;

	/** @copydoc TSkip */
	template <size_t Count, typename Tuple>
	requires (TTupleArity<Tuple>::Value >= Count)
	struct TSkip_Struct
	{
		template <size_t... Indices>
		static consteval TComposeFrom<Tuple, (Indices + Count)...> Compose(std::index_sequence<Indices...>&&);

		using Type = decltype(
			Compose(std::make_index_sequence<TTupleArity<Tuple>::Value - Count>{})
		);
	};

	/** Skip the first `Count` elements of the input tuple */
	template <size_t Count, typename Tuple>
	using TSkip = typename TSkip_Struct<Count, Tuple>::Type;

	/** @copydoc TTrimEnd */
	template <size_t Count, typename Tuple>
	requires (TTupleArity<Tuple>::Value >= Count)
	struct TTrimEnd_Struct
	{
		template <size_t... Indices>
		static consteval TComposeFrom<Tuple, Indices...> Compose(std::index_sequence<Indices...>&&);

		using Type = decltype(
			Compose(std::make_index_sequence<TTupleArity<Tuple>::Value - Count>{})
		);
	};

	/** Disregard the last `Count` elements of the input tuple */
	template <size_t Count, typename Tuple>
	using TTrimEnd = typename TTrimEnd_Struct<Count, Tuple>::Type;

	/** @copydoc TTake */
	template <size_t Count, typename Tuple>
	requires (TTupleArity<Tuple>::Value >= Count)
	struct TTake_Struct
	{
		template <size_t... Indices>
		static consteval TComposeFrom<Tuple, Indices...> Compose(std::index_sequence<Indices...>&&);

		using Type = decltype(
			Compose(std::make_index_sequence<Count>{})
		);
	};

	/** Take only the first `Count` elements of the input tuple */
	template <size_t Count, typename Tuple>
	using TTake = typename TTake_Struct<Count, Tuple>::Type;
}
