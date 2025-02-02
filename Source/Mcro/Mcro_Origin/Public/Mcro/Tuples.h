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

/** @brief Templating utilities for manipulating `TTuple`s */
namespace Mcro::Tuples
{
	/** @brief Compose one tuple out of the elements of another tuple based on the input index parameter pack */
	template <typename Tuple, size_t... Indices>
	using TComposeFrom = TTuple<typename TTupleElement<Indices, Tuple>::Type...>;

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

	/** @brief Skip the first `Count` elements of the input tuple */
	template <size_t Count, typename Tuple>
	using TSkip = typename TSkip_Struct<Count, Tuple>::Type;

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

	/** @brief Disregard the last `Count` elements of the input tuple */
	template <size_t Count, typename Tuple>
	using TTrimEnd = typename TTrimEnd_Struct<Count, Tuple>::Type;

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

	/** @brief Take only the first `Count` elements of the input tuple */
	template <size_t Count, typename Tuple>
	using TTake = typename TTake_Struct<Count, Tuple>::Type;

	namespace Detail
	{
		template <typename T, typename RestTuple, size_t... Indices>
		auto Prepend_Impl(T&& left, RestTuple const& right, std::index_sequence<Indices...>&&)
		{
			return TTuple<T, typename TTupleElement<Indices, RestTuple>::Type...>(
				Forward<T>(left), right.template Get<Indices>()...
			);
		}
		
		template <typename T, typename RestTuple, size_t... Indices>
		auto Append_Impl(T&& right, RestTuple const& left, std::index_sequence<Indices...>&&)
		{
			return TTuple<typename TTupleElement<Indices, RestTuple>::Type..., T>(
				left.template Get<Indices>()..., Forward<T>(right)
			);
		}
	}

	/** @brief Prepend a value to a tuple */
	template <typename T, typename... Rest>
	TTuple<T, Rest...> operator >> (T&& left, TTuple<Rest...> const& right)
	{
		return Detail::Prepend_Impl(left, right, std::make_index_sequence<sizeof...(Rest)>{});
	}

	/** @brief Append a value to a tuple */
	template <typename T, typename... Rest>
	TTuple<Rest..., T> operator << (TTuple<Rest...> const& left, T&& right)
	{
		return Detail::Append_Impl(right, left, std::make_index_sequence<sizeof...(Rest)>{});
	}
}
