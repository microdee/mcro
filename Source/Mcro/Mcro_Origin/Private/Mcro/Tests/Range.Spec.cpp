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

	TBitArray<> bitArrayA;
	TBitArray<> bitArrayB;
	[](auto&&){} (ranges::begin(bitArrayA));
	[](auto&&){} (ranges::end(bitArrayA));
	[](auto&&){} (views::concat(bitArrayA, bitArrayB) | views::take(10));

	TChunkedArray<int32> acr;
	TChunkedArray<int32> bcr;

	TSparseArray<int32> aspr;
	TSparseArray<int32> bspr;

	TStaticArray<int32, 1> asr {0};
	TStaticArray<int32, 1> bsr {0};

	TIndirectArray<int32> air;
	TIndirectArray<int32> bir;
	
	TMap<int32, int32> am;
	TMap<int32, int32> bm;
	
	// non-comparable
	TSortedMap<int32, int32> a_sm;
	TSortedMap<int32, int32> b_sm;
	
	TQueue<int32> aq;
	TQueue<int32> bq;
	
	TRingBuffer<int32> arb;
	TRingBuffer<int32> brb;

	// non-comparable
	TDeque<int32> adq;
	TDeque<int32> bdq;

	// non-comparable
	// TIntrusiveDoubleLinkedList<int32> aidll;
	// TIntrusiveDoubleLinkedList<int32> bidll;

	// non-comparable
	TLinkedList<int32> all;
	TLinkedList<int32> bll;

	// non-comparable
	TDoubleLinkedList<int32> adll;
	TDoubleLinkedList<int32> bdll;
	
	// non-comparable
	TLruCache<int32, int32> alruc;
	TLruCache<int32, int32> blruc;

	// non-comparable
	TPagedArray<int32> apr;
	TPagedArray<int32> bpr;

	// no iterators (needs custom iterators)
	// TCircularBuffer
	// FBinaryHeap
	// TCircularQueue
	// TConsumeAllMpmcQueue
	// TDiscardableKeyValueCache
	// TStaticHashTable
	// FHashTable
	// TStaticBitArray
	// TStridedView
	// TTripleBuffer

}