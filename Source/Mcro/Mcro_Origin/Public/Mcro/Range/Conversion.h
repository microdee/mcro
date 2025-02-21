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
	using namespace Mcro::Concepts;

	struct FRangeStringFormatOptions
	{
		FString Start { TEXT_"[" };
		FString End { TEXT_"]" };
		FString Separator { TEXT_", " };
	};

	constexpr FRangeStringFormatOptions NoDecoratorFormatOptions = {{}, {}, {}};
	
	namespace Detail
	{
		template <CRangeMember Range>
		struct TRangeWithStringFormat
		{
			TRangeWithStringFormat(Range const& range, FRangeStringFormatOptions const& options)
				: Storage(range)
				, Options(options)
			{}
			
			auto begin() const { return Storage.begin(); }
			auto end() const { return Storage.end(); }
			
			FRangeStringFormatOptions Options;
		private:
			Range const& Storage;
		};
	}
	
	/**
	 *	@brief
	 *	Specialize this template to specify a default item separator sequence for a range type when converting it to a
	 *	string.
	 */
	template <typename T>
	struct TRangeStringFormatOptions
	{
		static FRangeStringFormatOptions Get(T&&)
		{
			return {};
		}
	};

	template <CRangeMember Range>
	struct TRangeStringFormatOptions<Detail::TRangeWithStringFormat<Range>>
	{
		static FRangeStringFormatOptions Get(Detail::TRangeWithStringFormat<Range>&& from)
		{
			return from.Options;
		}
	};

	/** @brief Specify a separator sequence for a range when converting it to a string */
	FORCEINLINE auto Separator(FString const& separator)
	{
		return ranges::make_pipeable([separator] <CRangeMember Input> (Input&& range)
		{
			if constexpr (CIsTemplate<Input, Detail::TRangeWithStringFormat>)
			{
				range.Options.Separator = separator;
				return range;
			}
			else return Detail::TRangeWithStringFormat(range, {.Separator = separator});
		});
	}

	/** @brief Specify a start and an end sequence enclosing this range when converting it to a string */
	FORCEINLINE auto Enclosure(FString const& start, FString const& end)
	{
		return ranges::make_pipeable([=] <CRangeMember Input> (Input&& range)
		{
			if constexpr (CIsTemplate<Input, Detail::TRangeWithStringFormat>)
			{
				range.Options.Start = start;
				range.Options.End = end;
				return range;
			}
			else return Detail::TRangeWithStringFormat(range, {.Start = start, .End = end});
		});
	};

	/** @brief Don't use a separator when this range is rendered to a string */
	FORCEINLINE auto NoSeparator()
	{
		return ranges::make_pipeable([] <CRangeMember Input> (Input&& range)
		{
			if constexpr (CIsTemplate<Input, Detail::TRangeWithStringFormat>)
			{
				range.Options.Separator = {};
				return range;
			}
			else return Detail::TRangeWithStringFormat(range, {.Separator = {}});
		});
	}

	/** @brief Don't enclose this range in anything when it's rendered to a string */
	FORCEINLINE auto NoEnclosure()
	{
		return ranges::make_pipeable([] <CRangeMember Input> (Input&& range)
		{
			if constexpr (CIsTemplate<Input, Detail::TRangeWithStringFormat>)
			{
				range.Options.Start = {};
				range.Options.End = {};
				return range;
			}
			else return Detail::TRangeWithStringFormat(range, {.Start = {}, .End = {}});
		});
	}

	/**
	 *	@brief
	 *	Don't insert anything else than the contents of the input range when that is rendered as a string just append
	 *	each item one after the other.
	 */
	FORCEINLINE auto NoDecorators()
	{
		return ranges::make_pipeable([] <CRangeMember Input> (Input&& range)
		{
			if constexpr (CIsTemplate<Input, Detail::TRangeWithStringFormat>)
			{
				range.Options = NoDecoratorFormatOptions;
				return range;
			}
			else return Detail::TRangeWithStringFormat(range, NoDecoratorFormatOptions);
		});
	}

	namespace Detail
	{
		template <typename CharType>
		void CopyCharactersToBuffer(CharType const& value, int32 chunks, int32& position, TArray<CharType>& buffer)
		{
			if (buffer.Num() == position) buffer.AddZeroed(chunks);
			buffer[position] = value;
			++position;
		}

		template <CStringOrView String>
		void CopyStringToBufferUnsafe(String const& value, int32& position, TArray<TCHAR>& buffer)
		{
			FMemory::Memcpy(buffer.GetData() + position, GetData(value), value.Len() * sizeof(TCHAR));
			position += value.Len();
		}

		template <CStringOrView String>
		void CopyStringItemToBuffer(String const& value, FString const& separator, int32 chunks, int32& position, TArray<TCHAR>& buffer)
		{
			int nextLength = value.Len() + separator.Len();
				
			if (buffer.Num() <= position + nextLength) buffer.AddZeroed(chunks);
			if (!separator.IsEmpty())
				CopyStringToBufferUnsafe(separator, position, buffer);
			CopyStringToBufferUnsafe(value, position, buffer);
		}
	}

	/**
	 *	@brief  Render an input range as a string.
	 *
	 *	For ranges of any char type, the output is an uninterrupted string of them. Other char types than TCHAR will
	 *	be converted to the encoding of the current TCHAR.
	 *
	 *	For ranges of strings and string-views individual items will be directly copy-appended to the output separated
	 *	by `, ` (unless another separator sequence is set via `Separator`)
	 *
	 *	For anything else, `Mcro::Text::AsString` is used. In fact this function serves as the basis for `AsString` for
	 *	any range type. Like for strings, any other type is separated by `, ` (unless another separator sequence is set
	 *	via `Separator`). For convenience a piped version is also provided of this function.
	 */
	template <CRangeMember Range>
	FString RenderAsString(Range&& range)
	{
		using ElementType = TRangeElementType<Range>;
		
		if (IteratorEquals(range.begin(), range.end()))
			return {};

		constexpr int chunks = 16384;
		int32 position = 0;
		FRangeStringFormatOptions rangeFormatOptions = TRangeStringFormatOptions<Range>::Get(range);

		if constexpr (CChar<ElementType>)
		{
			TArray<ElementType> buffer;
			for (ElementType const& character : range)
				Detail::CopyCharactersToBuffer(character, chunks, position, buffer);
			
			if constexpr (CCurrentChar<ElementType>)
				return FString::ConstructFromPtrSize(buffer.GetData(), position);
			else
			{
				TStdStringView<ElementType> stringView(buffer.GetData(), position);
				return UnrealConvert(stringView);
			}
		}
		else if constexpr (CStringOrView<ElementType>)
		{
			TArray<TCHAR> buffer;
			for (auto it = range.begin(); !IteratorEquals(it, range.end()); ++it)
			{
				ElementType const& value = *it;
				bool isFirst = IteratorEquals(it, range.begin());
				Detail::CopyStringItemToBuffer(
					value,
					isFirst ? FString() : rangeFormatOptions.Separator,
					chunks,
					position, buffer
				);
			}
			FString output = FString::ConstructFromPtrSize(buffer.GetData(), position);
			return rangeFormatOptions.Start + output + rangeFormatOptions.End;
		}
		else
		{
			TArray<TCHAR> buffer;
			for (auto it = range.begin(); !IteratorEquals(it, range.end()); ++it)
			{
				ElementType const& value = *it;
				FString valueString = AsString(value);

				bool isFirst = IteratorEquals(it, range.begin());
				Detail::CopyStringItemToBuffer(
					valueString,
					isFirst ? FString() : rangeFormatOptions.Separator,
					chunks,
					position, buffer
				);
			}
			FString output = FString::ConstructFromPtrSize(buffer.GetData(), position);
			return rangeFormatOptions.Start + output + rangeFormatOptions.End;
		}
	}

	FORCEINLINE auto RenderAsString()
	{
		return ranges::make_pipeable([](auto&& range) { return RenderAsString(range); });
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
			CTuple Value = TRangeElementType<From>,
			typename MapType = TMap<TTypeAtDecayed<0, Value>, TTypeAtDecayed<1, Value>>
		>
		requires (GetSize<Value>() >= 2)
		static void Convert(From&& range, MapType& result)
		{
			for (Value const& value : range)
				result.Add(GetItem<0>(value), GetItem<1>(value));
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
			CTuple Value = TRangeElementType<From>,
			typename MapType = TMap<TTypeAtDecayed<0, Value>, TTypeAtDecayed<1, Value>>
		>
		requires (GetSize<Value>() >= 2)
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

namespace Mcro::Text
{
	using namespace Mcro::Range;
	
	template <CRangeMember Operand>
	requires (
		!CDirectStringFormatArgument<Operand>
		&& !CHasToString<Operand>
	)
	struct TAsFormatArgument<Operand>
	{
		template <CConvertibleToDecayed<Operand> Arg>
		FString operator () (Arg&& left) const { return RenderAsString(left); }
	};
}
