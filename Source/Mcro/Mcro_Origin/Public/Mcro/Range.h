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

/**
 *	@file
 *	@brief
 *	Bring modern declarative range operations like views and actions to the Unreal C++ arsenal. This header is
 *	responsible for bridging compatibility between Unreal containers and range-v3/std::ranges.
 *
 *	The official documentation is quite barebones and doesn't go to great lengths explaining how individual features in
 *	the range-v3 library behave exactly. For a more explanatory documentation I found this website
 *	https://lukasfro.github.io/range-v3-doc/chapter_01/01_introduction.html by **PhD Lukas Fröhlich** which lists
 *	views with examples and precise descriptions. Although, as the original documentation, that is also "incomplete".
 *
 *	Please consider the [potentials and pitfals](https://lukasfro.github.io/range-v3-doc/chapter_03/03_02_pitfalls.html)
 *	and the [tips & tricks](https://lukasfro.github.io/range-v3-doc/chapter_03/03_01_tips_tricks.html) when using
 *	range-v3 in your program.
 */

#include "CoreMinimal.h"

#include "Containers/BasicArray.h"
#include "Containers/Deque.h"
#include "Containers/LruCache.h"
#include "Containers/MRUArray.h"
#include "Containers/PagedArray.h"
#include "Containers/RingBuffer.h"

#include "Mcro/Range/Iterators.h"

#include "Mcro/LibraryIncludes/Start.h"
#include "range/v3/all.hpp"
#include "Mcro/LibraryIncludes/End.h"

/** @brief Bring modern declarative range operations like views and actions to the Unreal C++ arsenal */

template <Mcro::Concepts::CRangeMember Container, Mcro::Range::FExtendedIteratorPolicy Policy = {}>
using TIteratorExtension = Mcro::Range::TExtendedIterator<decltype(DeclVal<Container>().begin()), Policy>;

// TArray (pointer)
template <typename T, typename A> auto begin(TArray<T, A>&       r) ->       T* { return r.GetData(); }
template <typename T, typename A> auto begin(TArray<T, A> const& r) -> const T* { return r.GetData(); }
template <typename T, typename A> auto end  (TArray<T, A>&       r) ->       T* { return r.GetData() + r.Num(); }
template <typename T, typename A> auto end  (TArray<T, A> const& r) -> const T* { return r.GetData() + r.Num(); }
template <typename T, typename A> size_t size(TArray<T, A> const& r) { return static_cast<size_t>(r.Num()); }

// TArrayView (pointer)
template <typename T, typename A> auto begin(TArrayView<T, A>&       r) ->       T* { return r.GetData(); }
template <typename T, typename A> auto begin(TArrayView<T, A> const& r) -> const T* { return r.GetData(); }
template <typename T, typename A> auto end  (TArrayView<T, A>&       r) ->       T* { return r.GetData() + r.Num(); }
template <typename T, typename A> auto end  (TArrayView<T, A> const& r) -> const T* { return r.GetData() + r.Num(); }
template <typename T, typename A> size_t size(TArrayView<T, A> const& r) { return static_cast<size_t>(r.Num()); }

// TStaticArray (pointer)
template <typename T, uint32 N, uint32 A> auto begin(TStaticArray<T, N, A>&       r) ->       T* { return r.GetData(); }
template <typename T, uint32 N, uint32 A> auto begin(TStaticArray<T, N, A> const& r) -> const T* { return r.GetData(); }
template <typename T, uint32 N, uint32 A> auto end  (TStaticArray<T, N, A>&       r) ->       T* { return r.GetData() + r.Num(); }
template <typename T, uint32 N, uint32 A> auto end  (TStaticArray<T, N, A> const& r) -> const T* { return r.GetData() + r.Num(); }
template <typename T, uint32 N, uint32 A> size_t size(TStaticArray<T, N, A> const& r) { return static_cast<size_t>(r.Num()); }

// TIndirectArray (pointer)
template <typename T>
using TExtendedIndirectArrayIterator = Mcro::Range::TExtendedIterator<T, Mcro::Range::FExtendedIteratorPolicy{.DereferencePointerToPointer = true}>; 

template <typename T, typename A> auto begin(TIndirectArray<T, A>&       r) -> TExtendedIndirectArrayIterator<      T**> { return r.GetData(); }
template <typename T, typename A> auto begin(TIndirectArray<T, A> const& r) -> TExtendedIndirectArrayIterator<const T**> { return r.GetData(); }
template <typename T, typename A> auto end  (TIndirectArray<T, A>&       r) -> TExtendedIndirectArrayIterator<      T**> { return r.GetData() + r.Num(); }
template <typename T, typename A> auto end  (TIndirectArray<T, A> const& r) -> TExtendedIndirectArrayIterator<const T**> { return r.GetData() + r.Num(); }
template <typename T, typename A> size_t size(TIndirectArray<T, A> const& r) { return static_cast<size_t>(r.Num()); }

// TSet (GetIndex-able)
template <typename T, typename K, typename A> auto begin(TSet<T, K, A>&       r) -> TIteratorExtension<TSet<T, K, A>> { return r.begin(); }
template <typename T, typename K, typename A> auto begin(TSet<T, K, A> const& r) -> TIteratorExtension<TSet<T, K, A>> { return r.begin(); }
template <typename T, typename K, typename A> auto end  (TSet<T, K, A>&       r) -> TIteratorExtension<TSet<T, K, A>> { return r.end(); }
template <typename T, typename K, typename A> auto end  (TSet<T, K, A> const& r) -> TIteratorExtension<TSet<T, K, A>> { return r.end(); }
template <typename T, typename K, typename A> size_t size(TSet<T, K, A> const& r) { return static_cast<size_t>(r.Num()); }

// TMapBase
// TMap and variants doesn't allow zero-copy view on its pairs with a capable enough iterator, however it has the
// internal TSet stored as a protected member Pairs, and so we can access it via exploiting the fact that TMapBase
// friends its template specializations. So we can use a dummy TMapBase to access that internal TSet.
struct FMapPairsAccessTag {};

template<> class TMapBase<FMapPairsAccessTag, FMapPairsAccessTag, FMapPairsAccessTag, FMapPairsAccessTag>
{
public:
	
	template <typename K, typename V, typename A, typename F>
	static auto GetPairs(TMapBase<K, V, A, F>& map) -> TSet<TPair<K, V>, F, A>&
	{
		return map.Pairs;
	}
	
	template <typename K, typename V, typename A, typename F>
	static auto GetPairs(TMapBase<K, V, A, F> const& map) -> TSet<TPair<K, V>, F, A> const&
	{
		return map.Pairs;
	}
};

using FMapPairsAccess = TMapBase<FMapPairsAccessTag, FMapPairsAccessTag, FMapPairsAccessTag, FMapPairsAccessTag>;

// TMap (map to TSet)
template<typename K, typename V, typename A, typename F> auto begin(TMap<K, V, A, F>&       map) { return begin(FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> auto begin(TMap<K, V, A, F> const& map) { return begin(FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> auto end  (TMap<K, V, A, F>&       map) { return end  (FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> auto end  (TMap<K, V, A, F> const& map) { return end  (FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> size_t size(TMap<K, V, A, F> const& map) { return static_cast<size_t>(map.Num()); }
// TMultiMap (map to TSet)
template<typename K, typename V, typename A, typename F> auto begin(TMultiMap<K, V, A, F>&       map) { return begin(FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> auto begin(TMultiMap<K, V, A, F> const& map) { return begin(FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> auto end  (TMultiMap<K, V, A, F>&       map) { return end  (FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> auto end  (TMultiMap<K, V, A, F> const& map) { return end  (FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> size_t size(TMultiMap<K, V, A, F> const& map) { return static_cast<size_t>(map.Num()); }

// Strings (pointer)
template <typename CharType> auto begin(TStringView<CharType> const& string) -> const CharType* { return string.GetData(); }
template <typename CharType> auto end  (TStringView<CharType> const& string) -> const CharType* { return string.GetData() + string.Len(); }
template <typename CharType> size_t size(TStringView<CharType> const& string) { return static_cast<size_t>(string.Len()); }

FORCEINLINE auto begin(FString&       string) -> const TCHAR* { return *string; }
FORCEINLINE auto begin(FString const& string) -> const TCHAR* { return *string; }
FORCEINLINE auto end  (FString&       string) -> const TCHAR* { return *string + string.Len(); }
FORCEINLINE auto end  (FString const& string) -> const TCHAR* { return *string + string.Len(); }
FORCEINLINE auto begin(FString&&      string) -> Mcro::Range::FTempStringIterator { return {Forward<FString>(string), false}; }
FORCEINLINE auto end  (FString&&      string) -> Mcro::Range::FTempStringIterator { return {Forward<FString>(string), true}; }
FORCEINLINE size_t size(FString const& string) { return static_cast<size_t>(string.Len()); }

// TBasicArray (pointer via begin/end)
template <typename T> auto begin(TBasicArray<T>&       r) ->       T* { return r.begin(); }
template <typename T> auto begin(TBasicArray<T> const& r) -> const T* { return r.begin(); }
template <typename T> auto end  (TBasicArray<T>&       r) ->       T* { return r.end(); }
template <typename T> auto end  (TBasicArray<T> const& r) -> const T* { return r.end(); }
template <typename T> size_t size(TBasicArray<T> const& r) { return static_cast<size_t>(r.Num()); }

// TBitArray (GetIndex-able)
template <typename A> auto begin(TBitArray<A>&       r) -> TIteratorExtension<TBitArray<A>> { return r.begin(); }
template <typename A> auto begin(TBitArray<A> const& r) -> TIteratorExtension<TBitArray<A>> { return r.begin(); }
template <typename A> auto end  (TBitArray<A>&       r) -> TIteratorExtension<TBitArray<A>> { return r.end(); }
template <typename A> auto end  (TBitArray<A> const& r) -> TIteratorExtension<TBitArray<A>> { return r.end(); }
template <typename A> size_t size(TBitArray<A> const& r) { return static_cast<size_t>(r.Num()); }

// TChunkedArray (GetIndex-able)
template <typename T, uint32 C, typename A> auto begin(TChunkedArray<T, C, A>&       r) -> TIteratorExtension<TChunkedArray<T, C, A>> { return r.begin(); }
template <typename T, uint32 C, typename A> auto begin(TChunkedArray<T, C, A> const& r) -> TIteratorExtension<TChunkedArray<T, C, A>> { return r.begin(); }
template <typename T, uint32 C, typename A> auto end  (TChunkedArray<T, C, A>&       r) -> TIteratorExtension<TChunkedArray<T, C, A>> { return r.end(); }
template <typename T, uint32 C, typename A> auto end  (TChunkedArray<T, C, A> const& r) -> TIteratorExtension<TChunkedArray<T, C, A>> { return r.end(); }
template <typename T, uint32 C, typename A> size_t size(TChunkedArray<T, C, A> const& r) { return static_cast<size_t>(r.Num()); }

// TDeque (warning, non-comparable)
template <typename T, typename A> auto begin(TDeque<T, A>&       r) -> TIteratorExtension<TDeque<T, A>> { return r.begin(); }
template <typename T, typename A> auto begin(TDeque<T, A> const& r) -> TIteratorExtension<TDeque<T, A>> { return r.begin(); }
template <typename T, typename A> auto end  (TDeque<T, A>&       r) -> TIteratorExtension<TDeque<T, A>> { return r.end(); }
template <typename T, typename A> auto end  (TDeque<T, A> const& r) -> TIteratorExtension<TDeque<T, A>> { return r.end(); }
template <typename T, typename A> size_t size(TDeque<T, A> const& r) { return static_cast<size_t>(r.Num()); }

// TResourceArray (pointer, TArray derivative)
template <typename T, uint32 A> auto begin(TResourceArray<T, A>&       r) ->       T* { return r.GetData(); }
template <typename T, uint32 A> auto begin(TResourceArray<T, A> const& r) -> const T* { return r.GetData(); }
template <typename T, uint32 A> auto end  (TResourceArray<T, A>&       r) ->       T* { return r.GetData() + r.Num(); }
template <typename T, uint32 A> auto end  (TResourceArray<T, A> const& r) -> const T* { return r.GetData() + r.Num(); }
template <typename T, uint32 A> size_t size(TResourceArray<T, A> const& r) { return static_cast<size_t>(r.Num()); }

// T[Double]LinkedList is not supported as it defines begin/end functions as friends, and so we cannot overload that
// template <typename T> auto begin(TLinkedList<T>&       r) -> TIteratorExtension<TLinkedList<T>> { return r.begin(); }
// template <typename T> auto begin(TLinkedList<T> const& r) -> TIteratorExtension<TLinkedList<T>> { return r.begin(); }
// template <typename T> auto end  (TLinkedList<T>&       r) -> TIteratorExtension<TLinkedList<T>> { return r.end(); }
// template <typename T> auto end  (TLinkedList<T> const& r) -> TIteratorExtension<TLinkedList<T>> { return r.end(); }
// template <typename T> size_t size(TLinkedList<T> const& r) { return static_cast<size_t>(r.Num()); }

// TLruCache (warning, non-comparable)
template <typename K, typename V, typename C> auto begin(TLruCache<K, V, C>&       r) -> TIteratorExtension<TLruCache<K, V, C>> { return r.begin(); }
template <typename K, typename V, typename C> auto begin(TLruCache<K, V, C> const& r) -> TIteratorExtension<TLruCache<K, V, C>> { return r.begin(); }
template <typename K, typename V, typename C> auto end  (TLruCache<K, V, C>&       r) -> TIteratorExtension<TLruCache<K, V, C>> { return r.end(); }
template <typename K, typename V, typename C> auto end  (TLruCache<K, V, C> const& r) -> TIteratorExtension<TLruCache<K, V, C>> { return r.end(); }
template <typename K, typename V, typename C> size_t size(TLruCache<K, V, C> const& r) { return static_cast<size_t>(r.Num()); }

// TMRUArray (pointer, TArray derivative)
template <typename T, typename A> auto begin(TMRUArray<T, A>&       r) ->       T* { return r.GetData(); }
template <typename T, typename A> auto begin(TMRUArray<T, A> const& r) -> const T* { return r.GetData(); }
template <typename T, typename A> auto end  (TMRUArray<T, A>&       r) ->       T* { return r.GetData() + r.Num(); }
template <typename T, typename A> auto end  (TMRUArray<T, A> const& r) -> const T* { return r.GetData() + r.Num(); }
template <typename T, typename A> size_t size(TMRUArray<T, A> const& r) { return static_cast<size_t>(r.Num()); }

// TPagedArray (warning, non-comparable)
template <typename T, int32 P, typename A> auto begin(TPagedArray<T, P, A>&       r) -> TIteratorExtension<TPagedArray<T, P, A>> { return r.begin(); }
template <typename T, int32 P, typename A> auto begin(TPagedArray<T, P, A> const& r) -> TIteratorExtension<TPagedArray<T, P, A>> { return r.begin(); }
template <typename T, int32 P, typename A> auto end  (TPagedArray<T, P, A>&       r) -> TIteratorExtension<TPagedArray<T, P, A>> { return r.end(); }
template <typename T, int32 P, typename A> auto end  (TPagedArray<T, P, A> const& r) -> TIteratorExtension<TPagedArray<T, P, A>> { return r.end(); }
template <typename T, int32 P, typename A> size_t size(TPagedArray<T, P, A> const& r) { return static_cast<size_t>(r.Num()); }

// TRingBuffer (GetIndex-able)
template <typename T, typename A> auto begin(TRingBuffer<T, A>&       r) -> TIteratorExtension<TRingBuffer<T, A>> { return r.begin(); }
template <typename T, typename A> auto begin(TRingBuffer<T, A> const& r) -> TIteratorExtension<TRingBuffer<T, A>> { return r.begin(); }
template <typename T, typename A> auto end  (TRingBuffer<T, A>&       r) -> TIteratorExtension<TRingBuffer<T, A>> { return r.end(); }
template <typename T, typename A> auto end  (TRingBuffer<T, A> const& r) -> TIteratorExtension<TRingBuffer<T, A>> { return r.end(); }
template <typename T, typename A> size_t size(TRingBuffer<T, A> const& r) { return static_cast<size_t>(r.Num()); }

// TSortedMap (warning, non-comparable)
template <typename K, typename V, typename A, typename S> auto begin(TSortedMap<K, V, A, S>&       r) -> TIteratorExtension<TSortedMap<K, V, A, S>> { return r.begin(); }
template <typename K, typename V, typename A, typename S> auto begin(TSortedMap<K, V, A, S> const& r) -> TIteratorExtension<TSortedMap<K, V, A, S>> { return r.begin(); }
template <typename K, typename V, typename A, typename S> auto end  (TSortedMap<K, V, A, S>&       r) -> TIteratorExtension<TSortedMap<K, V, A, S>> { return r.end(); }
template <typename K, typename V, typename A, typename S> auto end  (TSortedMap<K, V, A, S> const& r) -> TIteratorExtension<TSortedMap<K, V, A, S>> { return r.end(); }
template <typename K, typename V, typename A, typename S> size_t size(TSortedMap<K, V, A, S> const& r) { return static_cast<size_t>(r.Num()); }

// TSparseArray (GetIndex-able)
template <typename T, typename A> auto begin(TSparseArray<T, A>&       r) -> TIteratorExtension<TSparseArray<T, A>> { return r.begin(); }
template <typename T, typename A> auto begin(TSparseArray<T, A> const& r) -> TIteratorExtension<TSparseArray<T, A>> { return r.begin(); }
template <typename T, typename A> auto end  (TSparseArray<T, A>&       r) -> TIteratorExtension<TSparseArray<T, A>> { return r.end(); }
template <typename T, typename A> auto end  (TSparseArray<T, A> const& r) -> TIteratorExtension<TSparseArray<T, A>> { return r.end(); }
template <typename T, typename A> size_t size(TSparseArray<T, A> const& r) { return static_cast<size_t>(r.Num()); }