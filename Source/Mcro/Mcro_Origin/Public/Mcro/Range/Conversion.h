﻿/** @noop License Comment
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

#include <ranges>

#include "CoreMinimal.h"

#include "Mcro/Concepts.h"
#include "Mcro/Range/Iterators.h"
#include "Mcro/Text/TupleAsString.h"
#include "Mcro/Templates.h"
#include "Mcro/Text.h"

#include "Mcro/LibraryIncludes/Start.h"
#include "range/v3/all.hpp"
#include "Mcro/LibraryIncludes/End.h"

namespace Mcro::Range
{
	using namespace Mcro::Templates;
	
	namespace Detail
	{
		template <CRangeMember Range>
		struct TExplicitSeparator
		{
			TExplicitSeparator(Range const& range, const TCHAR* separator)
				: Storage(range)
				, Separator(separator)
			{}
			
			auto begin() const { return Storage.begin(); }
			auto end() const { return Storage.end(); }

			FString GetSeparator() const { return Separator; }
			
		private:
			Range const& Storage;
			FString Separator;
		};
	}
	
	/**
	 *	@brief
	 *	Specialize this template to specify a default item separator sequence for a range type when converting it to a
	 *	string.
	 */
	template <typename T>
	struct TContainerAsStringSeparator
	{
		static FStringView Get(T&&)
		{
			static auto separator = TEXTVIEW_", ";
			return separator;
		}
	};

	template <CRangeMember Range>
	struct TContainerAsStringSeparator<Detail::TExplicitSeparator<Range>>
	{
		static FString Get(Detail::TExplicitSeparator<Range>&& from)
		{
			return from.GetSeparator();
		}
	};

	/** @brief  Specify a separator sequence for a range when converting it to a string */
	template <typename = void>
	auto SeparatedBy(const TCHAR* separator)
	{
		return ranges::make_pipeable([separator](auto&& range)
		{
			return Detail::TExplicitSeparator(range, separator);
		});
	}
	
	/**
	 *	@brief  Render a range as the given container.
	 *
	 *	This functor will iterate over the entire input range and copy its values to the newly created container
	 *	one-by-one with its `Add` function. If you want a more optimised way to do that use `OutputTo` where you can
	 *	supply your own container as an l-value.
	 *
	 *	usage:
	 *	@code
	 *	using namespace ranges;
	 *	auto result = views::ints(0)
	 *		| views::stride(2)
	 *		| views::take(5)
	 *		| RenderAs<TArray>();
	 *		
	 *	// -> TArray<int32> {0, 2, 4, 6, 8}
	 *	@endcode
	 *	
	 *	@tparam Target
	 *	An Unreal container template which has a public function member `Add`, the element-type of which will be deduced
	 *	from the input left side range.
	 */
	template <template <typename> typename Target>
	class RenderAs
	{
		template <CRangeMember From, typename Value = TRangeElementType<From>>
		requires CUnrealRange<Target<Value>>
		Target<Value> Convert(From&& range) const
		{
			Target<Value> result;
			for (Value const& value : range)
				result.Add(value);
			return result;
		}
		
	public:
		RenderAs() {};

		template <CRangeMember From>
		friend auto operator | (From&& range, RenderAs&& functor)
		{
			return functor.Convert(range);
		}
		
		template <CRangeMember From>
		auto Render(From&& range) const
		{
			return Convert(range);
		}
	};

	/**
	 *	@brief  Render a range to an already existing container.
	 *
	 *	This functor will iterate over the entire input range and copy its values to the given container one-by-one.
	 *	Target container must expose iterators which allows modifying its content. If the input range has more items
	 *	than the target container current size, then start using its `Add` function. 
	 *
	 *	usage:
	 *	@code
	 *	using namespace ranges;
	 *	TArray<int32> Storage;
	 *	Storage.SetNumUninitialized(5);
	 *	
	 *	auto result = views::ints(0, 10)
	 *		| views::stride(2)
	 *		| OutputTo(Storage);
	 *		
	 *	// -> TArray<int32> {0, 2, 4, 6, 8}
	 *	@endcode
	 *	
	 *	@tparam Target
	 *	An Unreal container which has a public function member `Add`, the element-type of which will be deduced from
	 *	the target output container.
	 */
	template <CUnrealRange Target>
	class OutputTo
	{
		using ElementType = TRangeElementType<Target>;
		Target& Storage;

		template <CRangeMember From, CConvertibleToDecayed<ElementType> Value = TRangeElementType<From>>
		void Convert(From&& range)
		{
			auto it = Storage.begin();
			auto endIt = Storage.end(); 
			for (Value const& value : range)
			{
				if (IteratorEquals(it, endIt))
					Storage.Add(value);
				else
				{
					*it = value;
					++it;
				}
			}
		}

	public:
		OutputTo(Target& target) : Storage(target) {}

		template <CRangeMember From>
		friend Target& operator | (From&& range, OutputTo&& functor)
		{
			functor.Convert(range);
			return functor.Storage;
		}
	};

	/**
	 *	@brief
	 *	Render a range of tuples or range of ranges with at least 2 elements as a TMap.
	 *
	 *	This functor will iterate over the entire input range and copy its values to the newly created container
	 *	one-by-one with its `Add` function.
	 *
	 *	When working with range-of-ranges then ranges which doesn't have at least two elements will be silently ignored.
	 *
	 *	usage (from tuples):
	 *	@code
	 *	using namespace ranges;
	 *	TArray<int32> MyKeyArray {1, 2, 3, 4, 5};
	 *	TArray<FString> MyValueArray {TEXT_"foo", TEXT_"bar"};
	 *	
	 *	auto result = views::zip(MyKeyArray, views::cycle(MyValueArray))
	 *		| RenderAsMap();
	 *		
	 *	// -> TMap<int32, FString> {{1, "foo"}, {2, "bar"}, {3, "foo"}, {4, "bar"}, {5, "foo"}}
	 *	@endcode
	 *	
	 *	usage (from inner-ranges):
	 *	@code
	 *	using namespace ranges;
	 *	auto result = views::ints(0, 9)
	 *		| views::chunk(2)
	 *		| RenderAsMap();
	 *		
	 *	// -> TMap<int32, int32> {{0, 1}, {2, 3}, {4, 5}, {6, 7}}
	 *	// notice how 8 is discarded from the end, as that range didn't have 2 items 
	 *	@endcode
	 */
	class RenderAsMap
	{
		template <
			CRangeMember From,
			typename Value = TRangeElementType<From>,
			typename MapType = TMap<TTupleElement<0, Value>, TTupleElement<1, Value>>
		>
		requires (TIsTuple_V<Value> && TTupleArity<Value>::Value >= 2)
		static void Convert(From&& range, MapType& result)
		{
			for (Value const& value : range)
				result.Add(value.template Get<0>(), value.template Get<1>());
		}
		
		template <
			CRangeMember From,
			CStdPairLike Value = TRangeElementType<From>,
			typename MapType = TMap<std::tuple_element_t<0, Value>, std::tuple_element_t<1, Value>>
		>
		static void Convert(From&& range, MapType& result)
		{
			for (Value const& value : range)
				result.Add(std::get<0>(value), std::get<1>(value));
		}
		
		template <
			CRangeMember From,
			CRangeV3PairLike Value = TRangeElementType<From>,
			typename MapType = TMap<std::tuple_element_t<0, Value>, std::tuple_element_t<1, Value>>
		>
		static void Convert(From&& range, MapType& result)
		{
			for (Value const& value : range)
				result.Add(ranges::get<0>(value), ranges::get<1>(value));
		}
		
		template <
			CRangeMember From,
			CRangeMember InnerRange = TRangeElementType<From>,
			typename Value = TRangeElementType<InnerRange>,
			typename MapType = TMap<Value, Value>
		>
		static void Convert(From&& range, MapType& result)
		{
			for (InnerRange const& innerRange : range)
			{
				// TODO: support TMultiMap
				auto it = innerRange.begin();
				if (IteratorEquals(it, innerRange.end())) continue;
				Value const& key = *it;
				++it;
				if (IteratorEquals(it, innerRange.end())) continue;
				Value const& value = *it;
				result.Add(key, value);
			}
		}
		
		template <
			CRangeMember From,
			typename Value = TRangeElementType<From>,
			typename MapType = TMap<TTupleElement<0, Value>, TTupleElement<1, Value>>
		>
		requires (TIsTuple_V<Value> && TTupleArity<Value>::Value >= 2)
		MapType Convert(From&& range) const
		{
			MapType result;
			Convert(range, result);
			return result;
		}
		
		template <
			CRangeMember From,
			CRangeMember InnerRange = TRangeElementType<From>,
			typename Value = TRangeElementType<InnerRange>,
			typename MapType = TMap<Value, Value>
		>
		MapType Convert(From&& range) const
		{
			MapType result;
			Convert(range, result);
			return result;
		}

	public:
		template <CIsTemplate<TMap> Target>
		friend class OutputToMap;
		
		RenderAsMap() {};

		template <CRangeMember From>
		friend auto operator | (From&& range, RenderAsMap&& functor)
		{
			return functor.Convert(range);
		}
		
		template <CRangeMember From>
		auto Render(From&& range) const
		{
			return Convert(range);
		}
	};

	/**
	 *	@brief
	 *	Output a range of tuples or range of ranges with at least 2 elements to an already existing TMap.
	 *
	 *	This functor will iterate over the entire input range and copy its values to the existing TMap one-by-one with
	 *	its `Add` function.
	 *
	 *	When working with range-of-ranges then ranges which doesn't have at least two elements will be silently ignored.
	 *
	 *	usage (from tuples):
	 *	@code
	 *	using namespace ranges;
	 *	TArray<int32> MyKeyArray {1, 2, 3, 4, 5};
	 *	TArray<FString> MyValueArray {TEXT_"foo", TEXT_"bar"};
	 *	
	 *	auto result = views::zip(MyKeyArray, views::cycle(MyValueArray))
	 *		| RenderAsMap();
	 *		
	 *	// -> TMap<int32, FString> {{1, "foo"}, {2, "bar"}, {3, "foo"}, {4, "bar"}, {5, "foo"}}
	 *	@endcode
	 *	
	 *	usage (from inner-ranges):
	 *	@code
	 *	using namespace ranges;
	 *	auto result = views::ints(0, 9)
	 *		| views::chunk(2)
	 *		| RenderAsMap();
	 *		
	 *	// -> TMap<int32, int32> {{0, 1}, {2, 3}, {4, 5}, {6, 7}}
	 *	// notice how 8 is discarded from the end, as that range didn't have 2 items 
	 *	@endcode
	 *	
	 *	@tparam Target  A TMap. Its key-value types will be deduced from the target output map.
	 */
	template <CIsTemplate<TMap> Target>
	class OutputToMap
	{
		using KeyType = typename Target::KeyType;
		using ValueType = typename Target::ValueType;
		Target& Storage;

	public:
		OutputToMap(Target& target) : Storage(target) {};

		template <
			CRangeMember From,
			CTupleConvertsToArgs<KeyType, ValueType> = TRangeElementType<From>
		>
		friend Target& operator | (From&& range, OutputToMap&& functor)
		{
			RenderAsMap::Convert(range, functor.Storage);
			return functor.Storage;
		}
	};

	
}
