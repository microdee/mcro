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

#include "Mcro/LibraryIncludes/Start.h"
#include "range/v3/all.hpp"
#include "Mcro/LibraryIncludes/End.h"

namespace Mcro::Range
{
	using namespace Mcro::Concepts;

	enum class EIteratorDirection
	{
		NotIterator,
		Forward,
		Bidirectional,
	};
	enum class EIteratorStep
	{
		NotIterator,
		Single,
		Jump,
		JumpBinaryOperators,
	};

	template <typename L, typename R>
	concept CHasEquals = requires (L& a, R& b) { a == b; };

	template <typename L, typename R>
	concept CHasNotEquals = requires (L& a, R& b) { a != b; };

	template <typename L, typename R>
	bool IteratorEquals(L const& l, R const& r)
	{
		if constexpr (CHasEquals<L, R>)
			return l == r;
		else return !(l != r);
	}

	/** @brief The most basic minimal iterator which can only proceed forward one element at a time */
	template <typename T>
	concept CBasicForwardIteratorBase = CPointer<T> || requires(T& i) { ++i; *i; };

	template <typename T>
	concept CBasicForwardIterator = CBasicForwardIteratorBase<T>
		&& (CHasEquals<T, T> || CHasNotEquals<T, T>)
	;

	template <typename T>
	concept CHasMemberAccessOperator = requires(T& i) { i.operator->(); };

	/** @brief Basic minimal iterator which can only proceed forward or backward one element at a time */
	template <typename T>
	concept CBasicBidirectionalIterator = CPointer<T> || (CBasicForwardIterator<T> && requires(T& i) { --i; });

	/** @brief An iterator type which can natively proceed forward in arbitrarily large steps */
	template <typename T>
	concept CJumpForwardIterator = CPointer<T> || (CBasicForwardIterator<T> && requires(T& i) { i += 2; });

	/** @brief An iterator type which can natively proceed forward in arbitrarily large steps and exposes a + operator */
	template <typename T>
	concept CJumpForwardPlusIterator = CPointer<T> || (CJumpForwardIterator<T> && requires(T& i) { { i + 2 } -> CConvertibleToDecayed<T>; });

	/** @brief An iterator type which can natively seek its associated content in arbitrarily large steps */
	template <typename T>
	concept CRandomAccessIterator = CPointer<T> || (CJumpForwardIterator<T> && requires(T& i) { i -= 2; });

	/**
	 *	@brief
	 *	An iterator type which can natively seek its associated content in arbitrarily large steps and exposes +- operators
	 */
	template <typename T>
	concept CRandomAccessPlusMinusIterator = CPointer<T> ||
	(
		CRandomAccessIterator<T>
		&& CJumpForwardPlusIterator<T>
		&& requires(T& i)
		{
			{ i - 2 } -> CConvertibleToDecayed<T>;
		}
	);

	/** @brief Get the direction given iterator is capable of */
	template <typename T>
	constexpr EIteratorDirection TIteratorDirection =
		  CBasicBidirectionalIterator<T>
		? EIteratorDirection::Bidirectional
		: CBasicForwardIterator<T>
		? EIteratorDirection::Forward
		: EIteratorDirection::NotIterator
	;

	/** @brief Get the maximum steps the given iterator can be seeking with */
	template <typename T>
	constexpr EIteratorStep TIteratorStep =
		  CJumpForwardPlusIterator<T>
		? EIteratorStep::JumpBinaryOperators
		: CJumpForwardIterator<T>
		? EIteratorStep::Jump
		: CBasicForwardIterator<T>
		? EIteratorStep::Single
		: EIteratorStep::NotIterator
	;

	/** @brief Assume an STL iterator category from input iterator */
	template <typename T>
	using TIteratorCategory = std::conditional_t<
		CRandomAccessIterator<T>,
		std::random_access_iterator_tag,
		std::conditional_t<
			CBasicBidirectionalIterator<T>,
			std::bidirectional_iterator_tag,
			std::conditional_t<
				CBasicForwardIterator<T>,
				std::forward_iterator_tag,
				std::input_iterator_tag
			>
		>
	>;

	/**
	 *	@brief
	 *	Constraint given iterator to its best stepping capability.
	 *
	 *	To constrain for "at least" method of capability use `CBasicForwardIterator` and refining concepts.
	 */
	template <typename T, EIteratorStep Step>
	concept CIsIteratorStep = TIteratorStep<T> == Step;

	/**
	 *	@brief
	 *	Constraint given iterator to its best directional capability.
	 *
	 *	To constrain for "at least" method of capability use `CBasicForwardIterator` and refining concepts.
	 */
	template <typename T, EIteratorDirection Direction>
	concept CIsIteratorDirection = TIteratorDirection<T> == Direction;

	/** @brief Constraint given iterator to its best capabilies both in stepping and in direction */
	template <typename T, EIteratorDirection Direction, EIteratorStep Step>
	concept CIteratorFeature = CIsIteratorDirection<T, Direction> && CIsIteratorStep<T, Step>;

	/** @brief return the iterator's associated content type when they're dereferenced. */
	template <CBasicForwardIterator T>
	using TIteratorElementType = std::decay_t<decltype(*DeclVal<T>())>;

	/** @brief return a range's associated content type determined by dereferencing their iterator. */
	template <CRangeMember T>
	using TRangeElementType = TIteratorElementType<decltype(DeclVal<T>().begin())>;
	
	template <typename T>
	concept CUnrealRange = CRangeMember<T>
		&& requires(T& container, TRangeElementType<T> const& item)
		{
			container.Add(item);
		};

	template <typename T>
	concept CStdDistanceCompatible = requires(T& l, T& r) { std::distance(l, r); };

	template <typename T>
	concept CHasGetIndex = requires(T& i) { i.GetIndex(); };

	template <typename T>
	concept CHasElementIndex = requires(T& i) { i.ElementIndex; };

	template <typename T>
	concept CIteratorComparable =
		CTotallyOrdered<T>
		|| CHasGetIndex<T>
		|| CHasElementIndex<T>
	;

	template <typename T>
	concept CCountableRange = requires(T&& t) { size(t); };
}
