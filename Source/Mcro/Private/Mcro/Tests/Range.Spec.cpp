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

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Mcro/Common.h"
#include "Containers/Deque.h"
#include "Containers/LruCache.h"
#include "Containers/PagedArray.h"
#include "Containers/RingBuffer.h"
#include "Mcro/Tests/TestCompatibility.h"

using namespace Mcro::Common;

DEFINE_SPEC(
	FMcroRange_Spec,
	TEXT_"Mcro.Range",
	EAutomationTestFlags_ApplicationContextMask
	| EAutomationTestFlags::CriticalPriority
	| EAutomationTestFlags::ProductFilter
);

void FMcroRange_Spec::Define()
{
	using namespace ranges;
	Describe(TEXT_"Range-V3 should support Unreal container", [this]
	{
		It(TEXT_"TArray", [this]
		{
			TArray containerA {0, 1, 2, 3, 4};
			TArray containerB {5, 6, 7, 8, 9};

			TestTrue(
				TEXT_"Matches concat",
				containerA | Concat(containerB) | MatchOrdered(views::ints(0, 10))
			);
		});
		
		It(TEXT_"TSet", [this]
		{
			TSet containerA {0, 1, 2, 3, 4};
			TSet containerB {5, 6, 7, 8, 9};
			auto result = containerA | Concat(containerB) | RenderAs<TSet>();
			TestTrue(
				TEXT_"Matches concat",
				views::ints(0, 10) | AllOf([&](int32 i)
				{
					return result.Contains(i);
				})
			);
		});
		
		It(TEXT_"TMap", [this]
		{
			TArray containerA {0, 1, 2, 3, 4};
			TArray containerB {5, 6, 7, 8, 9};
			auto result = containerA | Zip(containerB) | RenderAsMap();
			for (int i = 0; i < 5; ++i)
				TestEqual(TEXT_"Map got zipped", result[i], i + 5);
		});
	});

	Describe(TEXT_"Serialize ranges to string", [this]
	{
		It(TEXT_"Simple array", [this]
		{
			TArray payload { NAME_"Foo", NAME_"Bar", NAME_"Asd" };
			TestEqualSensitive(
				TEXT_"Default decorated",
				payload | RenderAsString(),
				TEXT_"[Foo, Bar, Asd]"
			);
			
			TestEqualSensitive(
				TEXT_"Decorated with nothing",
				payload | NoDecorators() | RenderAsString(),
				TEXT_"FooBarAsd"
			);
			
			TestEqualSensitive(
				TEXT_"Combining Decorators",
				payload
					| Separator(TEXT_" and ")
					| Enclosure(TEXT_"!", TEXT_"?")
					| RenderAsString(),
				TEXT_"!Foo and Bar and Asd?"
			);
			
			TestEqualSensitive(
				TEXT_"Via FMT macro",
				TEXT_"stuff: {0}" _FMT(payload | NoDecorators()),
				TEXT_"stuff: FooBarAsd"
			);
		});
		It(TEXT_"TMap", [this]
		{
			TMap<EPixelFormat, FName> payload
			{
				{PF_R8G8, NAME_"Ech"},
				{PF_DXT1, NAME_"OK"},
				{PF_DXT3, NAME_"Nice"},
				{PF_DXT5, NAME_"Great"}
			};

			TestEqualSensitive(
				TEXT_"Correctly handle tuples",
				payload | NoEnclosure() | RenderAsString(),
				TEXT_"(PF_R8G8, Ech), (PF_DXT1, OK), (PF_DXT3, Nice), (PF_DXT5, Great)"
			);
		});
	});

	Describe(TEXT_"Functional deconstruction of tuples", [this]
	{
		It(TEXT_"should transform properly", [this]
		{
			TMap<EPixelFormat, FName> map
			{
				{PF_R8G8, NAME_"Ech"},
				{PF_DXT1, NAME_"OK"},
				{PF_DXT3, NAME_"Nice"},
				{PF_DXT5, NAME_"Great"}
			};

			TestTrue(
				TEXT_"Tuples from maps",
				map
					| TransformTuple([](EPixelFormat key, FName const& value)
					{
						return value;
					})
					| MatchOrdered({NAME_"Ech", NAME_"OK", NAME_"Nice", NAME_"Great"})
			);
		});
	});
}


void Test()
{
	using namespace ranges;
	
	TSet<int32> setA;
	TSet<int32> setB;
	[](auto&&){} (ranges::begin(setA));
	[](auto&&){} (ranges::end(setA));
	[](auto&&){} (views::concat(setA, setB) | views::take(10));

	TArray<int32> arrayA;	
	TArray<int32> arrayB;
	[](auto&&){} (ranges::begin(arrayA));
	[](auto&&){} (ranges::end(arrayA));
	[](auto&&){} (views::concat(arrayA, arrayB) | views::take(10));
	[](auto&&){} (views::concat(arrayA, arrayB)
		| views::take(10)
	);

	auto stuff = views::ints(0, unreachable)
		| views::transform([](int a) { return a * a; })
		| views::take(10)
		| RenderAs<TArray>()
		| OutputTo(arrayB)
	;

	auto stuff2 = views::ints(0, unreachable)
		| views::transform([](int a) { return a * a; })
		| views::take(10)
		| Separator(TEXT_"\n")
		| RenderAsString()
	;
	
	auto vi = views::for_each(
		views::ints(1, 6),
		[](int i) { return yield_from(views::repeat_n(i, i)); }
	);

	static_assert(CRangeMember<decltype(vi)>);

#if UE_VERSION_NEWER_THAN(5, 5, -1)
	TBitArray<> bitArrayA;
	TBitArray<> bitArrayB;
	[](auto&&){} (ranges::begin(bitArrayA));
	[](auto&&){} (ranges::end(bitArrayA));
	[](auto&&){} (views::concat(bitArrayA, bitArrayB) | views::take(10));
#endif

	TChunkedArray<int32> chunkedArrayA;
	TChunkedArray<int32> chunkedArrayB;
	[](auto&&){} (ranges::begin(chunkedArrayA));
	[](auto&&){} (ranges::end(chunkedArrayA));
	[](auto&&){} (views::concat(chunkedArrayA, chunkedArrayB) | views::take(10));

	TSparseArray<int32> sparseArrayA;
	TSparseArray<int32> sparseArrayB;
	[](auto&&){} (ranges::begin(sparseArrayA));
	[](auto&&){} (ranges::end(sparseArrayA));
	[](auto&&){} (views::concat(sparseArrayA, sparseArrayB) | views::take(10));

	TStaticArray<int32, 1> staticArrayA {0};
	TStaticArray<int32, 1> staticArrayB {0};
	[](auto&&){} (ranges::begin(staticArrayA));
	[](auto&&){} (ranges::end(staticArrayA));
	[](auto&&){} (views::concat(staticArrayA, staticArrayB) | views::take(10));

	TIndirectArray<int32> indirectArrayA;
	TIndirectArray<int32> indirectArrayB;
	[](auto&&){} (ranges::begin(indirectArrayA));
	[](auto&&){} (ranges::end(indirectArrayA));
	[](auto&&){} (views::concat(indirectArrayA, indirectArrayB) | views::take(10));
	
	TMap<int32, int32> mapA;
	TMap<int32, int32> mapB;
	[](auto&&){} (ranges::begin(mapA));
	[](auto&&){} (ranges::end(mapA));
	[](auto&&){} (views::concat(mapA, mapB) | views::take(10));
	
	// non-comparable
	TSortedMap<int32, int32> sortedMapA;
	TSortedMap<int32, int32> sortedMapB;
	[](auto&&){} (ranges::begin(sortedMapA));
	[](auto&&){} (ranges::end(sortedMapA));
	[](auto&&){} (views::concat(sortedMapA, sortedMapB) | views::take(10));
	
	TRingBuffer<int32> ringBufferA;
	TRingBuffer<int32> ringBufferB;
	[](auto&&){} (ranges::begin(ringBufferA));
	[](auto&&){} (ranges::end(ringBufferA));
	[](auto&&){} (views::concat(ringBufferA, ringBufferB) | views::take(10));

	// non-comparable
	TDeque<int32> dequeA;
	TDeque<int32> dequeB;
	[](auto&&){} (ranges::begin(dequeA));
	[](auto&&){} (ranges::end(dequeA));
	[](auto&&){} (views::concat(dequeA, dequeB) | views::take(10));

	FString stringA;
	FString stringB;
	[](auto&&){} (ranges::begin(stringA));
	[](auto&&){} (ranges::end(stringA));
	[](auto&&){} (views::concat(stringA, stringB) | views::take(10));

	// non-comparable
	// TIntrusiveDoubleLinkedList<int32> aidll;
	// TIntrusiveDoubleLinkedList<int32> bidll;

	// T[Double]LinkedList is not supported as it defines begin/end functions as friends, and so we cannot overload that
	// TLinkedList<int32> linkedListA;
	// TLinkedList<int32> linkedListB;
	// [](auto&&){} (ranges::begin(linkedListA));
	// [](auto&&){} (ranges::end(linkedListA));
	// [](auto&&){} (views::concat(linkedListA, linkedListB) | views::take(10));
	
	// non-comparable
	TLruCache<int32, int32> lruCacheA;
	TLruCache<int32, int32> lruCacheB;
	[](auto&&){} (ranges::begin(lruCacheA));
	[](auto&&){} (ranges::end(lruCacheA));
	[](auto&&){} (views::concat(lruCacheA, lruCacheB) | views::take(10));

	// non-comparable
	TPagedArray<int32> pagedArrayA;
	TPagedArray<int32> pagedArrayB;
	[](auto&&){} (ranges::begin(pagedArrayA));
	[](auto&&){} (ranges::end(pagedArrayA));
	[](auto&&){} (views::concat(pagedArrayA, pagedArrayB) | views::take(10));

	// no iterators (needs custom iterators)
	// TQueue
	// FBinaryHeap
	// TCircularBuffer
	// TCircularQueue
	// TConsumeAllMpmcQueue
	// TDiscardableKeyValueCache
	// TStaticHashTable
	// FHashTable
	// TStaticBitArray
	// TStridedView
	// TTripleBuffer

}