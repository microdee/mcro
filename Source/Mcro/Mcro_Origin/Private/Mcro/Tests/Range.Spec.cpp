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
#include "Mcro/Range.h"
#include "Containers/Deque.h"
#include "Containers/LruCache.h"
#include "Containers/PagedArray.h"
#include "Containers/RingBuffer.h"

using namespace Mcro::Range;

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

	views::ints(0, unreachable)
		| views::transform([](int a) { return a * a; })
		| views::take(10);
	
	auto vi = views::for_each(
		views::ints(1, 6),
		[](int i) { return yield_from(views::repeat_n(i, i)); }
	) | to<std::vector>();

	TBitArray<> bitArrayA;
	TBitArray<> bitArrayB;
	[](auto&&){} (ranges::begin(bitArrayA));
	[](auto&&){} (ranges::end(bitArrayA));
	[](auto&&){} (views::concat(bitArrayA, bitArrayB) | views::take(10));

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
	auto d = ::end(sortedMapA) - ::begin(sortedMapA);
	
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