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
#include "Containers/IntrusiveDoubleLinkedList.h"
#include "Containers/LruCache.h"
#include "Containers/PagedArray.h"
#include "Containers/RingBuffer.h"

using namespace Mcro::Range;

void Test()
{
	TSet<int32> as;
	TSet<int32> bs;

	TArray<int32> ar;
	const TArray<int32> br;

	TBitArray<> abr;
	TBitArray<> bbr;

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
	

	auto mi = begin(am);
	auto i = as.begin();
	
	static_assert(concepts::type<ranges::iter_difference_t<TExtendedIterator<decltype(i)>>>);
	static_assert(ranges::weakly_incrementable_concept_<TExtendedIterator<decltype(i)>>);
	static_assert(ranges::input_iterator<TExtendedIterator<decltype(i)>>);
	static_assert(ranges::readable_concept_<TExtendedIterator<decltype(i)>>);

	// bool compare = as.begin() < as.end();

	using namespace ranges;

	auto rb = ranges::begin(bs);
	auto re = ranges::end(bs);
	auto r = views::concat(ar, br) | views::take(10);

}