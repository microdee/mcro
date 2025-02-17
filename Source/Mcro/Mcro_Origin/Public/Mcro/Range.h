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
 *	responsible for bridging compatibility between Unreal collections and range-v3/std::ranges.
 */

#include "CoreMinimal.h"
#include "Mcro/Range/Iterators.h"

#include "Mcro/LibraryIncludes/Start.h"
#include "range/v3/all.hpp"
#include "Mcro/LibraryIncludes/End.h"

/** @brief Bring modern declarative range operations like views and actions to the Unreal C++ arsenal */

template <Mcro::Concepts::CRange Container, Mcro::Range::FExtendedIteratorPolicy Policy = {}>
using TIteratorExtension = Mcro::Range::TExtendedIterator<decltype(DeclVal<Container>().begin()), Policy>;

// TArray
template <typename T, typename A> auto begin(TArray<T, A>&       r) { return r.GetData(); }
template <typename T, typename A> auto begin(TArray<T, A> const& r) { return r.GetData(); }
template <typename T, typename A> auto end  (TArray<T, A>&       r) { return r.GetData() + r.Num(); }
template <typename T, typename A> auto end  (TArray<T, A> const& r) { return r.GetData() + r.Num(); }
template <typename T, typename A> size_t size(TArray<T, A> const& r) { return static_cast<size_t>(r.Num()); }

// TArrayView
template <typename T, typename A> auto begin(TArrayView<T, A>&       r) { return r.GetData(); }
template <typename T, typename A> auto begin(TArrayView<T, A> const& r) { return r.GetData(); }
template <typename T, typename A> auto end  (TArrayView<T, A>&       r) { return r.GetData() + r.Num(); }
template <typename T, typename A> auto end  (TArrayView<T, A> const& r) { return r.GetData() + r.Num(); }
template <typename T, typename A> size_t size(TArrayView<T, A> const& r) { return static_cast<size_t>(r.Num()); }

// TStaticArray
template <typename T, uint32 N, uint32 A> auto begin(TStaticArray<T, N, A>&       r) { return r.GetData(); }
template <typename T, uint32 N, uint32 A> auto begin(TStaticArray<T, N, A> const& r) { return r.GetData(); }
template <typename T, uint32 N, uint32 A> auto end  (TStaticArray<T, N, A>&       r) { return r.GetData() + r.Num(); }
template <typename T, uint32 N, uint32 A> auto end  (TStaticArray<T, N, A> const& r) { return r.GetData() + r.Num(); }
template <typename T, uint32 N, uint32 A> size_t size(TStaticArray<T, N, A> const& r) { return static_cast<size_t>(r.Num()); }

// TIndirectArray
template <typename T>
using TExtendedIndirectArrayIterator = Mcro::Range::TExtendedIterator<T, Mcro::Range::FExtendedIteratorPolicy{.DereferencePointerToPointer = true}>; 

template <typename T, typename A> auto begin(TIndirectArray<T, A>&       r) -> TExtendedIndirectArrayIterator<      T*> { return r.GetData(); }
template <typename T, typename A> auto begin(TIndirectArray<T, A> const& r) -> TExtendedIndirectArrayIterator<const T*> { return r.GetData(); }
template <typename T, typename A> auto end  (TIndirectArray<T, A>&       r) -> TExtendedIndirectArrayIterator<      T*> { return r.GetData() + r.Num(); }
template <typename T, typename A> auto end  (TIndirectArray<T, A> const& r) -> TExtendedIndirectArrayIterator<const T*> { return r.GetData() + r.Num(); }

// TSet
template <typename T, typename K, typename A> auto begin(TSet<T, K, A>&       r) -> TIteratorExtension<TSet<T, K, A>> { return r.begin(); }
template <typename T, typename K, typename A> auto begin(TSet<T, K, A> const& r) -> TIteratorExtension<TSet<T, K, A>> { return r.begin(); }
template <typename T, typename K, typename A> auto end  (TSet<T, K, A>&       r) -> TIteratorExtension<TSet<T, K, A>> { return r.end(); }
template <typename T, typename K, typename A> auto end  (TSet<T, K, A> const& r) -> TIteratorExtension<TSet<T, K, A>> { return r.end(); }

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

// TMap
template<typename K, typename V, typename A, typename F> auto begin(TMap<K, V, A, F>&       map) { return begin(FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> auto begin(TMap<K, V, A, F> const& map) { return begin(FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> auto end  (TMap<K, V, A, F>&       map) { return end  (FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> auto end  (TMap<K, V, A, F> const& map) { return end  (FMapPairsAccess::GetPairs(map)); }
// TMultiMap
template<typename K, typename V, typename A, typename F> auto begin(TMultiMap<K, V, A, F>&       map) { return begin(FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> auto begin(TMultiMap<K, V, A, F> const& map) { return begin(FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> auto end  (TMultiMap<K, V, A, F>&       map) { return end  (FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> auto end  (TMultiMap<K, V, A, F> const& map) { return end  (FMapPairsAccess::GetPairs(map)); }

// Strings
template <typename CharType> auto begin(TStringView<CharType> const& string) -> const CharType* { return string.GetData(); }
template <typename CharType> auto end  (TStringView<CharType> const& string) -> const CharType* { return string.GetData() + string.Len(); }
FORCEINLINE auto begin(FString const& string) -> const TCHAR* { return *string; }
FORCEINLINE auto end  (FString const& string) -> const TCHAR* { return *string + string.Len(); }
FORCEINLINE auto begin(FString&&      string) -> Mcro::Range::FTempStringIterator { return {Forward<FString>(string), false}; }
FORCEINLINE auto end  (FString&&      string) -> Mcro::Range::FTempStringIterator { return {Forward<FString>(string), true}; }

// TBitArray
template <typename A> auto begin(TBitArray<A>&       r) -> TIteratorExtension<TBitArray<A>> { return r.begin(); }
template <typename A> auto begin(TBitArray<A> const& r) -> TIteratorExtension<TBitArray<A>> { return r.begin(); }
template <typename A> auto end  (TBitArray<A>&       r) -> TIteratorExtension<TBitArray<A>> { return r.end(); }
template <typename A> auto end  (TBitArray<A> const& r) -> TIteratorExtension<TBitArray<A>> { return r.end(); }

// TBitArray
template <typename A> auto begin(TBitArray<A>&       r) -> TIteratorExtension<TBitArray<A>> { return r.begin(); }
template <typename A> auto begin(TBitArray<A> const& r) -> TIteratorExtension<TBitArray<A>> { return r.begin(); }
template <typename A> auto end  (TBitArray<A>&       r) -> TIteratorExtension<TBitArray<A>> { return r.end(); }
template <typename A> auto end  (TBitArray<A> const& r) -> TIteratorExtension<TBitArray<A>> { return r.end(); }