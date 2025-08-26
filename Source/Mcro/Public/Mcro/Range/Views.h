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
#include "Mcro/Range/Iterators.h"

#include "Mcro/LibraryIncludes/Start.h"
#include "range/v3/all.hpp"
#include "Mcro/LibraryIncludes/End.h"

namespace Mcro::Range
{
	using namespace Mcro::Concepts;

	/** @brief Make an initializer list compatible with range API's */
	template <typename T>
	decltype(auto) Literal(std::initializer_list<T>&& input) { return FWD(input); }

	/** @brief pipeable version of `ranges::views::zip` */
	template <CRangeMember... Ranges>
	auto Zip(Ranges&&...right)
	{
		return ranges::make_pipeable([&](auto&& left)
		{
			return ranges::views::zip(left, FWD(right)...);
		});
	}
	
	/** @brief pipeable version of `ranges::views::concat` */
	template <CRangeMember... Ranges>
	auto Concat(Ranges&&...right)
	{
		return ranges::make_pipeable([&](auto&& left)
		{
			return ranges::views::concat(left, FWD(right)...);
		});
	}

	/** @brief Check if range is empty */
	template <CRangeMember Input>
	bool IsEmpty(Input&& range)
	{
		return IteratorEquals(range.begin(), range.end());
	}

	/** @brief Check if range is empty */
	FORCEINLINE auto IsEmpty()
	{
		return ranges::make_pipeable([](auto&& left){ return IsEmpty(left); });
	}

	/** @brief Get's the first element of a range or return a provided default value. Same as `*r.begin()` but safer. */
	template <
		CRangeMember Input,
		typename Value = TRangeElementType<Input>
	>
	decltype(auto) First(Input&& range, Value&& def)
	{
		return IsEmpty(range) ? def : *range.begin();
	}

	/** @brief Get's the first element of a range or return a provided default value. Same as `*r.begin()` but safer. */
	FORCEINLINE auto First(auto&& def)
	{
		return ranges::make_pipeable([&](auto&& left){ return First(left, def); });
	}

	/**
	 *	@brief
	 *	Get's the first element of a range or return the default value for the element type. Same as `*r.begin()` but safer.
	 */
	template <
		CRangeMember Input,
		typename Value = TRangeElementType<Input>
	>
	decltype(auto) FirstOrDefault(Input&& range)
	{
		return IsEmpty(range) ? Value{} : *range.begin();
	}

	/**
	 *	@brief
	 *	Get's the first element of a range or return the default value for the element type. Same as `*r.begin()` but safer.
	 */
	FORCEINLINE auto FirstOrDefault()
	{
		return ranges::make_pipeable([&](auto&& left){ return FirstOrDefault(left); });
	}

	/**
	 *	@brief
	 *	Return true if input ranges match their values and their order.
	 *
	 *	This runs in O(N) time when result is true, unless both input ranges are `CCountableRange`, `matchOnlyBeginning`
	 *	is false and the two input ranges have different size.
	 *
	 *	@param  left  Input range
	 *	@param right  Range to compare with
	 *	
	 *	@param matchOnlyBeginning
	 *	By default MatchOrdered returns false for ranges with different lengths
	 */
	template <CRangeMember Left, CRangeMember Right>
	requires CCoreHalfEqualityComparable<TRangeElementType<Left>, TRangeElementType<Right>>
	bool MatchOrdered(Left&& left, Right&& right, bool matchOnlyBeginning = false)
	{
		if (IsEmpty(left) && IsEmpty(right)) return true;
		
		if constexpr (CCountableRange<Left> && CCountableRange<Right>)
			if (!matchOnlyBeginning && size(left) != size(right))
				return false;
		
		auto leftIt = left.begin();
		auto rightIt = right.begin();
		for (;;)
		{
			if (IteratorEquals(leftIt, left.end()) || IteratorEquals(rightIt, right.end()))
				return matchOnlyBeginning || (
					IteratorEquals(leftIt, left.end()) && IteratorEquals(rightIt, right.end())
				);

			if (*leftIt != *rightIt) return false;
			++leftIt;
			++rightIt;
		}
	}

	template <CRangeMember Left, typename Value>
	requires CCoreHalfEqualityComparable<Value, TRangeElementType<Left>>
	bool MatchOrdered(Left&& left, std::initializer_list<Value>&& right, bool matchOnlyBeginning = false)
	{
		return MatchOrdered(left, right, matchOnlyBeginning);
	}

	/**
	 *	@brief
	 *	Return true if input ranges match their values and their order.
	 *
	 *	This runs in O(N) time when result is true, unless both input ranges are `CCountableRange`, `matchOnlyBeginning`
	 *	is false and the two input ranges have different size.
	 *
	 *	@param right  Range to compare with
	 *	
	 *	@param matchOnlyBeginning
	 *	By default MatchOrdered returns false for ranges with different lengths
	 */
	template <CRangeMember Right>
	auto MatchOrdered(Right&& right, bool matchOnlyBeginning = false)
	{
		return ranges::make_pipeable([&](auto&& left){ return MatchOrdered(left, right, matchOnlyBeginning); });
	}

	template <typename Value>
	auto MatchOrdered(std::initializer_list<Value>&& right, bool matchOnlyBeginning = false)
	{
		return ranges::make_pipeable([&](auto&& left){ return MatchOrdered(left, right, matchOnlyBeginning); });
	}

	template <CFunctionLike Predicate>
	auto AllOf(Predicate&& pred)
	{
		return ranges::make_pipeable([&](auto&& left){ return ranges::all_of(left, pred); });
	}

	template <CFunctionLike Predicate>
	auto AnyOf(Predicate&& pred)
	{
		return ranges::make_pipeable([&](auto&& left){ return ranges::any_of(left, pred); });
	}

	FORCEINLINE auto FilterValid()
	{
		return ranges::views::filter([]<CValidable T>(T&& item)
		{
			return TestValid(FWD(item));
		});
	}

	/**
	 *	@brief
	 *	Transform a range of tuples with structured binding function arguments, so range transformations shouldn't
	 *	bother with the actual type of the tuple.
	 *
	 *	For example:
	 *	@code
	 *	int32 size = 4;
	 *	                              //   | highFreq            | lowFreq
	 *	namespace rv = ranges::views; //   V                     V
	 *	auto things = rv::cartesian(rv::indices(0, size), rv::indices(0, size))
	 *		| TransformTuple([size](int32 highFreq, int32 lowFreq)
	 *		{
	 *			return lowFreq * size + highFreq;
	 *		})
	 *	;
	 *	FMT_LOG(LogTemp, Display, "Things: {0}", things);
	 *	//-> Things: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]
	 *	@endcode 
	 *	
	 *	@tparam Transform  A transformation function type with signature compatible with input tuple.
	 *	@tparam      Left  The type of the range of tuples
	 *	@param       left  The range of tuples
	 *	@param         tr  A transformation function with signature compatible with input tuple.
	 *	@return  Range with transformed elements 
	 */
	template <CFunctionLike Transform, CRangeOfTuplesCompatibleWithFunction<Transform> Left>
	auto TransformTuple(Left&& left, Transform&& tr)
	{
		using Tuple = TRangeElementType<Left>;
		return ranges::views::transform(FWD(left), [tr](Tuple const& tuple)
		{
			return InvokeWithTuple(tr, tuple);
		});
	}

	/**
	 *	@brief
	 *	Transform a range of tuples with structured binding function arguments, so range transformations shouldn't
	 *	bother with the actual type of the tuple.
	 *
	 *	For example:
	 *	@code
	 *	int32 size = 4;
	 *	                              //   | highFreq            | lowFreq
	 *	namespace rv = ranges::views; //   V                     V
	 *	auto things = rv::cartesian(rv::indices(0, size), rv::indices(0, size))
	 *		| TransformTuple([size](int32 highFreq, int32 lowFreq)
	 *		{
	 *			return lowFreq * size + highFreq;
	 *		})
	 *	;
	 *	FMT_LOG(LogTemp, Display, "Things: {0}", things);
	 *	//-> Things: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]
	 *	@endcode 
	 *	
	 *	@tparam Transform  A transformation function type with signature compatible with input tuple.
	 *	@param         tr  A transformation function with signature compatible with input tuple.
	 *	@return  Range with transformed elements
	 */
	template <CFunctionLike Transform>
	auto TransformTuple(Transform&& tr)
	{
		return ranges::make_pipeable([tr]<CRangeOfTuplesCompatibleWithFunction<Transform> Left>(Left&& left)
		{
			return TransformTuple(FWD(left), tr);
		});
	}

	/**
	 *	@brief
	 *	Filter a range of tuples with structured binding function arguments, so filter predicates shouldn't
	 *	bother with the actual type of the tuple.
	 *
	 *	For example:
	 *	@code
	 *	int32 size = 4;
	 *	                              //   | highFreq            | lowFreq
	 *	namespace rv = ranges::views; //   V                     V
	 *	auto things = rv::cartesian(rv::indices(0, size), rv::indices(0, size))
	 *		| FilterTuple([size](int32 highFreq, int32 lowFreq)
	 *		{
	 *			return highFreq == 2;
	 *		})
	 *	;
	 *	FMT_LOG(LogTemp, Display, "Things: {0}", things);
	 *	//-> Things: [(2, 0), (2, 1), (2, 2), (2, 3)]
	 *	@endcode 
	 *	
	 *	@tparam Predicate  A transformation function type with signature compatible with input tuple.
	 *	@tparam      Left  The type of the range of tuples
	 *	@param       left  The range of tuples
	 *	@param  predicate  A transformation function with signature compatible with input tuple.
	 *	@return  Range with transformed elements 
	 */
	template <CFunctionLike Predicate, CRangeOfTuplesCompatibleWithFunction<Predicate> Left>
	auto FilterTuple(Left&& left, Predicate&& predicate)
	{
		using Tuple = TRangeElementType<Left>;
		return ranges::views::transform(FWD(left), [predicate](Tuple const& tuple)
		{
			return InvokeWithTuple(predicate, tuple);
		});
	}

	/**
	 *	@brief
	 *	Filter a range of tuples with structured binding function arguments, so filter predicates shouldn't
	 *	bother with the actual type of the tuple.
	 *
	 *	For example:
	 *	@code
	 *	int32 size = 4;
	 *	                              //   | highFreq            | lowFreq
	 *	namespace rv = ranges::views; //   V                     V
	 *	auto things = rv::cartesian(rv::indices(0, size), rv::indices(0, size))
	 *		| FilterTuple([size](int32 highFreq, int32 lowFreq)
	 *		{
	 *			return highFreq == 2;
	 *		})
	 *	;
	 *	FMT_LOG(LogTemp, Display, "Things: {0}", things);
	 *	//-> Things: [(2, 0), (2, 1), (2, 2), (2, 3)]
	 *	@endcode 
	 *	
	 *	@tparam Predicate  A filter predicate function type with signature compatible with input tuple.
	 *	@param  predicate  A filter predicate function with signature compatible with input tuple.
	 *	@return  Range with transformed elements
	 */
	template <CFunctionLike Predicate>
	auto FilterTuple(Predicate&& predicate)
	{
		return ranges::make_pipeable([predicate]<CRangeOfTuplesCompatibleWithFunction<Predicate> Left>(Left&& left)
		{
			return FilterTuple(FWD(left), predicate);
		});
	}

	template <size_t ItemIndex, CRangeOfTuples Range>
	auto SelectTupleItem(Range&& left)
	{
		using Tuple = TRangeElementType<Range>;
		return ranges::views::transform(FWD(left), [](Tuple const& tuple)
		{
			return GetItem<ItemIndex>(tuple);
		});
	}

	template <size_t ItemIndex>
	auto SelectTupleItem()
	{
		return ranges::make_pipeable([]<CRangeOfTuples Left>(Left&& left)
		{
			return SelectTupleItem<ItemIndex>(FWD(left));
		});
	}

	FORCEINLINE auto GetKeys() { return SelectTupleItem<0>(); }
	FORCEINLINE auto GetValues() { return SelectTupleItem<1>(); }
}
