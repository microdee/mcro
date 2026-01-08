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
#include "Mcro/Templates.h"

#include "Mcro/LibraryIncludes/Start.h"
#include "range/v3/all.hpp"
#include "Mcro/LibraryIncludes/End.h"

/** @brief Templating utilities for manipulating `TTuple`s */
namespace Mcro::Tuples
{
	using namespace Mcro::Concepts;
	using namespace Mcro::Templates;

	template <typename>
	constexpr bool TIsStdArray = false;

	template <typename T, size_t S>
	constexpr bool TIsStdArray<std::array<T, S>> = true;

	template <typename>
	constexpr bool TIsStdSubRange = false;

	template <class I, class S, std::ranges::subrange_kind K>
	constexpr bool TIsStdSubRange<std::ranges::subrange<I, S, K>> = true;

	template <typename>
	constexpr bool TIsRangeV3SubRange = false;

	template <class I, class S, ranges::subrange_kind K>
	constexpr bool TIsRangeV3SubRange<ranges::subrange<I, S, K>> = true;

	template <typename>
	constexpr bool TIsStdTuple = false;

	template <typename... Args>
	constexpr bool TIsStdTuple<std::tuple<Args...>> = true;

	template <typename>
	constexpr bool TIsStdPair = false;

	template <typename... Args>
	constexpr bool TIsStdPair<std::pair<Args...>> = true;

	template <typename>
	constexpr bool TIsRangeV3Tuple = false;

	template <typename... Args>
	constexpr bool TIsRangeV3Tuple<ranges::common_tuple<Args...>> = true;

	template <typename>
	constexpr bool TIsRangeV3Pair = false;

	template <typename... Args>
	constexpr bool TIsRangeV3Pair<ranges::common_pair<Args...>> = true;

	template <typename T>
	concept CStdTupleLike =
		TIsStdTuple<std::decay_t<T>>
		|| TIsStdPair<std::decay_t<T>>
		|| TIsStdArray<std::decay_t<T>>
		|| TIsStdSubRange<std::decay_t<T>>;

	template <typename T>
	concept CRangeV3TupleLike =
		TIsRangeV3Tuple<std::decay_t<T>>
		|| TIsRangeV3Pair<std::decay_t<T>>
		|| TIsRangeV3SubRange<std::decay_t<T>>;

	template <typename T>
	concept CStdPairLike = CStdTupleLike<T> && std::tuple_size_v<std::remove_cvref_t<T>> == 2;

	template <typename T>
	concept CRangeV3PairLike = CRangeV3TupleLike<T> && std::tuple_size_v<std::remove_cvref_t<T>> == 2;

	template <typename T, typename... Args>
	concept CTupleConvertsToArgs =
		CConvertibleToDecayed<T, TTuple<Args...>>
		|| CConvertibleToDecayed<T, std::tuple<Args...>>
		|| CConvertibleToDecayed<T, ranges::common_tuple<Args...>>
	;

	template <typename T>
	concept CUnrealTuple = TIsTuple_V<std::decay_t<T>>;

	template <typename T>
	concept CStdOrRangeV3Tuple = CStdTupleLike<T> || CRangeV3TupleLike<T>;

	template <typename T>
	concept CTuple = CStdTupleLike<T> || CRangeV3TupleLike<T> || CUnrealTuple<T>;

	template <size_t I, CStdTupleLike T>
	decltype(auto) GetItem(T&& tuple)
	{
		return std::get<I>(FWD(tuple));
	}

	template <size_t I, CRangeV3TupleLike T>
	decltype(auto) GetItem(T&& tuple)
	{
		return ranges::get<I>(FWD(tuple));
	}

	template <size_t I, CUnrealTuple T>
	decltype(auto) GetItem(T&& tuple)
	{
		return FWD(tuple).template Get<I>();
	}

	template <CStdOrRangeV3Tuple T>
	consteval size_t GetSize()
	{
		return std::tuple_size_v<std::decay_t<T>>;
	}

	template <CUnrealTuple T>
	consteval size_t GetSize()
	{
		return TTupleArity<std::decay_t<T>>::Value;
	}

	template <CTuple T>
	using TIndexSequenceForTuple = std::make_index_sequence<GetSize<T>()>;

	template <size_t, typename>
	struct TTypeAt_Struct {};

	template <size_t I, CStdOrRangeV3Tuple T>
	struct TTypeAt_Struct<I, T>
	{
		using Type = std::tuple_element_t<I, T>;
	};

	template <size_t I, CUnrealTuple T>
	struct TTypeAt_Struct<I, T>
	{
		using Type = typename TTupleSafeElement_Struct<I, T>::Type;
	};

	template <size_t I, CTuple T>
	using TTypeAt = typename TTypeAt_Struct<I, T>::Type;

	template <size_t I, CTuple T>
	using TTypeAtDecayed = std::decay_t<typename TTypeAt_Struct<I, T>::Type>;

	// TODO: Make these templates compatible with STL and Range-V3 tuples as well
	
	/** @brief Compose one tuple out of the elements of another tuple based on the input index parameter pack */
	template <typename Tuple, size_t... Indices>
	using TComposeTupleFrom = TTuple<typename TTupleElement<Indices, Tuple>::Type...>;

	template <size_t Count, typename Tuple>
	requires (TTupleArity<Tuple>::Value >= Count)
	struct TTupleSkip_Struct
	{
		template <size_t... Indices>
		static consteval TComposeTupleFrom<Tuple, (Indices + Count)...> Compose(std::index_sequence<Indices...>&&);

		using Type = decltype(
			Compose(std::make_index_sequence<TTupleArity<Tuple>::Value - Count>{})
		);
	};

	/** @brief Skip the first `Count` elements of the input tuple */
	template <size_t Count, typename Tuple>
	using TTupleSkip = typename TTupleSkip_Struct<Count, Tuple>::Type;

	template <size_t Count, typename Tuple>
	requires (TTupleArity<Tuple>::Value >= Count)
	struct TTupleTrimEnd_Struct
	{
		template <size_t... Indices>
		static consteval TComposeTupleFrom<Tuple, Indices...> Compose(std::index_sequence<Indices...>&&);

		using Type = decltype(
			Compose(std::make_index_sequence<TTupleArity<Tuple>::Value - Count>{})
		);
	};

	/** @brief Disregard the last `Count` elements of the input tuple */
	template <size_t Count, typename Tuple>
	using TTupleTrimEnd = typename TTupleTrimEnd_Struct<Count, Tuple>::Type;

	template <size_t Count, typename Tuple>
	requires (TTupleArity<Tuple>::Value >= Count)
	struct TTupleTake_Struct
	{
		template <size_t... Indices>
		static consteval TComposeTupleFrom<Tuple, Indices...> Compose(std::index_sequence<Indices...>&&);

		using Type = decltype(
			Compose(std::make_index_sequence<Count>{})
		);
	};

	/** @brief Take only the first `Count` elements of the input tuple */
	template <size_t Count, typename Tuple>
	using TTupleTake = typename TTupleTake_Struct<Count, Tuple>::Type;

	namespace Detail
	{
		template <typename T, typename RestTuple, size_t... Indices>
		auto Prepend_Impl(T&& left, RestTuple const& right, std::index_sequence<Indices...>&&)
		{
			return TTuple<T, typename TTupleElement<Indices, RestTuple>::Type...>(
				FWD(left), right.template Get<Indices>()...
			);
		}
		
		template <typename T, typename RestTuple, size_t... Indices>
		auto Append_Impl(T&& right, RestTuple const& left, std::index_sequence<Indices...>&&)
		{
			return TTuple<typename TTupleElement<Indices, RestTuple>::Type..., T>(
				left.template Get<Indices>()..., FWD(right)
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
