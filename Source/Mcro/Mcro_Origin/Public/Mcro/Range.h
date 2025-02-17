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
 *	@brief  This header is responsible for bridging compatibility between range-v3 and Unreal collections.
 */

#include "CoreMinimal.h"
#include "Mcro/Concepts.h"
#include "Mcro/TextMacros.h"
#include "Mcro/Void.h"

#include "Mcro/LibraryIncludes/Start.h"
#include "range/v3/all.hpp"
#include "Mcro/LibraryIncludes/End.h"

/** @brief Bring modern declarative range operations like views and actions to the Unreal C++ arsenal */
namespace Mcro::Range
{
	using namespace Concepts;

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

	/** @brief The most basic minimal iterator which can only proceed forward one element at a time */
	template <typename T>
	concept CBasicForwardIterator = CPointer<T> || requires(T& i, T& other)
	{
		++i;
		*i;
		i.operator->();
		i == other;
	};

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

	/** @brief return the iterator's associated content type when they're dereferenced. It preserves qualifiers. */
	template <CBasicForwardIterator T>
	using TIteratorElementType = std::decay_t<decltype(*DeclVal<T>())>;

	template <typename T>
	struct TIteratorJumpForward_Struct {};

	template <CIsIteratorStep<EIteratorStep::Single> T>
	struct TIteratorJumpForward_Struct<T>
	{
		T& operator () (T& iterator, size_t steps)
		{
			UE_STATIC_ASSERT_WARN(false,
				"Given iterator can only be incremented in single steps, and therefore can only be incremented in O(N) time!"
			);
			for (size_t i=0; i<steps; ++i)
				++iterator;
			return iterator;
		}
	};
	
	template <CIsIteratorStep<EIteratorStep::Jump> T>
	struct TIteratorJumpForward_Struct<T>
	{
		T& operator () (T& iterator, size_t steps)
		{
			return iterator += steps;
		}
	};

	template <typename T>
	constexpr TIteratorJumpForward_Struct<T> TIteratorJumpForward;

	template <typename T>
	struct TIteratorJumpBackward_Struct {};

	template <CIteratorFeature<EIteratorDirection::Bidirectional, EIteratorStep::Single> T>
	struct TIteratorJumpBackward_Struct<T>
	{
		T& operator () (T& iterator, size_t steps)
		{
			UE_STATIC_ASSERT_WARN(false,
				"Given iterator can only be decremented in single steps, and therefore can only be decremented in O(N) time!"
			);
			for (size_t i=0; i<steps; ++i)
				--iterator;
			return iterator;
		}
	};
	
	template <CIteratorFeature<EIteratorDirection::Bidirectional, EIteratorStep::Jump> T>
	struct TIteratorJumpBackward_Struct<T>
	{
		T& operator () (T& iterator, size_t steps)
		{
			return iterator -= steps;
		}
	};

	template <typename T>
	constexpr TIteratorJumpBackward_Struct<T> TIteratorJumpBackward;

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
	struct TIteratorDifference_Struct
	{
		using Type = void;
	};

	template <CBasicForwardIterator T>
	struct TIteratorDifference_Struct<T>
	{
		using Type = int64;
	};

	/*
	template <CPointer T>
	struct TIteratorDifference_Struct<T>
	{
		using Type = std::ptrdiff_t;
	};

	template <CHasGetIndex T>
	struct TIteratorDifference_Struct<T>
	{
		using Type = decltype(DeclVal<T>().GetIndex());
	};

	template <CHasElementIndex T>
	struct TIteratorDifference_Struct<T>
	{
		using Type = decltype(DeclVal<T>().ElementIndex);
	};
	*/

	/** @brief return a difference type for given iterator. */
	template <typename T>
	using TIteratorDifference = typename TIteratorDifference_Struct<T>::Type;

	template <typename T>
	struct TIteratorCompare_Struct {};

	template <CBasicForwardIterator T>
	requires CTotallyOrdered<T>
	struct TIteratorCompare_Struct<T>
	{
		auto operator () (T const& l, T const& r)
		{
			return l <=> r;
		}
	};
	
	template <CBasicForwardIterator T>
	requires CHasElementIndex<T>
	struct TIteratorCompare_Struct<T>
	{
		auto operator () (T const& l, T const& r)
		{
			return l.ElementIndex <=> r.ElementIndex;
		}
	};
	
	template <CBasicForwardIterator T>
	requires CHasGetIndex<T>
	struct TIteratorCompare_Struct<T>
	{
		auto operator () (T const& l, T const& r)
		{
			return l.GetIndex() <=> r.GetIndex();
		}
	};

	template <typename T>
	constexpr TIteratorCompare_Struct<T> TIteratorCompare;

	template <typename T>
	struct TIteratorComputeDistance_Struct {};
	
	template <CBasicForwardIterator T>
	struct TIteratorComputeDistance_Struct<T>
	{
		size_t operator () (T const& l, T const& r)
		{
			UE_STATIC_ASSERT_WARN(false,
				"Given iterator doesn't expose public state about its logical position within the range. Computing"
				" the distance between two may take O(N) time, where N is the singular steps between the two positions."
			);
			UE_STATIC_ASSERT_WARN(CTotallyOrdered<T>,
				"Given iterator wasn't relationally comparable. It is assumed that the right iterator is bigger than"
				" the left one. The program may freeze otherwise!"
			);

			T left = l;
			T right = r;
			if constexpr (CTotallyOrdered<T>) if (l > r)
			{
				left = r;
				right = l;
			}
			
			size_t i = 0;
			for (;;)
			{
				if (left != right) return i;
				ensureAlwaysMsgf(i < 1000000, TEXT_
					"Computing distance between two minimal iterators took longer than a million steps."
					" Maybe use a different collection type which can provide iterator distance in O(1) time, or heed"
					" the above warnings and make sure the right iterator is bigger than the left one."
				);
				++i;
			}
			return 0;
		}
	};
	
	template <CStdDistanceCompatible T>
	struct TIteratorComputeDistance_Struct<T>
	{
		auto operator () (T const& l, T const& r)
		{
			return std::distance(l, r);
		}
	};

	template <CHasElementIndex T>
	struct TIteratorComputeDistance_Struct<T>
	{
		auto operator () (T const& l, T const& r)
		{
			return r.ElementIndex - l.ElementIndex;
		}
	};

	template <CHasGetIndex T>
	struct TIteratorComputeDistance_Struct<T>
	{
		auto operator () (T const& l, T const& r)
		{
			return r.GetIndex() - l.GetIndex();
		}
	};

	template <typename T>
	constexpr TIteratorComputeDistance_Struct<T> TIteratorComputeDistance;

	/** @brief Extra settings for `TExtendedIterator` wrapper */
	struct FExtendedIteratorPolicy
	{
		/**
		 *	@brief
		 *	This is originally meant for `TIndirectArray` where the given operator is extremely minimal but it still
		 *	holds a contiguous memory block of pointers. `TExtendedIterator` can then dereference those pointers
		 *	instead.
		 */
		bool DereferencePointerToPointer = false;
	};

	/**
	 *	@brief
	 *	Unreal's own iterators are not STL compliant (they are only compatible with range-for loops) so they cannot be
	 *	used with more advanced STL algorithms or other third-party libraries which may expect the full iterator
	 *	interface compatibility. For that TExtendedIterator fills in the missing components and wraps Unreal iterators
	 *	to be fully STL iterator compliant.
	 *
	 *	TExtendedIterator makes a copy of the Unreal iterator it wraps for internal storage, which should not have
	 *	significant side effects, but emphasis is on SHOULD.
	 */
	template <CBasicForwardIterator Iterator, FExtendedIteratorPolicy Policy = {}>
	struct TExtendedIterator
		: TIteratorCategory<Iterator>
		, std::conditional_t<CConstType<Iterator>, FVoid, std::output_iterator_tag>
	{
		using value_type        = TIteratorElementType<Iterator>;
		using iterator_category = TIteratorCategory<Iterator>;
		
		using difference_type   = TIteratorDifference<Iterator>;
		using pointer           = value_type*;
		using reference         = value_type&;

		static constexpr bool SupportsDifference = CNonVoid<value_type>;

		TExtendedIterator() {}
		
		template <CConvertibleToDecayed<Iterator> InputIterator>
		TExtendedIterator(InputIterator&& input) : BaseIterator(input) {}

		TExtendedIterator(TExtendedIterator const& other)
			: BaseIterator(other.BaseIterator) {}

		TExtendedIterator(TExtendedIterator&& other) noexcept
			: BaseIterator(MoveTemp(other.BaseIterator)) {}

		auto operator = (TExtendedIterator other) -> TExtendedIterator&
		{
			using std::swap;
			swap(*this, other);
			return *this;
		}

		auto operator ++ () -> TExtendedIterator&
		{
			++BaseIterator;
			return *this;
		}

		auto operator ++ (int) -> TExtendedIterator
		{
			TExtendedIterator previous = *this;
			++BaseIterator;
			return previous;
		}

		template <CBasicBidirectionalIterator = Iterator>
		auto operator -- () -> TExtendedIterator&
		{
			--BaseIterator;
			return *this;
		}

		template <CBasicBidirectionalIterator = Iterator>
		auto operator -- (int) -> TExtendedIterator
		{
			TExtendedIterator previous = *this;
			--BaseIterator;
			return previous;
		}

		auto operator -> ()       { return BaseIterator.operator->(); }
		auto operator -> () const { return BaseIterator.operator->(); }

		auto operator * () -> value_type const&
		{
			if constexpr (Policy.DereferencePointerToPointer)
				return *static_cast<value_type*>(*BaseIterator);
			return *BaseIterator;
		}
		
		auto operator * () const -> value_type const&
		{
			if constexpr (Policy.DereferencePointerToPointer)
				return *static_cast<const value_type*>(*BaseIterator);
			return *BaseIterator;
		}

		auto operator += (int steps) -> TExtendedIterator&
		{
			return TIteratorJumpForward<Iterator>(*this, steps);
		}

		auto operator + (int steps) const -> TExtendedIterator
		{
			TExtendedIterator result = *this;
			result += steps;
			return result;
		}

		template <CBasicBidirectionalIterator = Iterator>
		auto operator -= (int steps) -> TExtendedIterator&
		{
			return TIteratorJumpBackward<Iterator>(*this, steps);
		}

		template <CBasicBidirectionalIterator = Iterator>
		auto operator - (int steps) const -> TExtendedIterator
		{
			TExtendedIterator result = *this;
			result -= steps;
			return result;
		}

		friend auto operator - (Iterator const& l, Iterator const& r) -> difference_type
		{
			return TIteratorComputeDistance<Iterator>(l, r);
		}

		friend bool operator == (TExtendedIterator const& l, TExtendedIterator const& r)
		{
			return l.BaseIterator == r.BaseIterator;
		}

		friend bool operator != (TExtendedIterator const& l, TExtendedIterator const& r)
		{
			return l.BaseIterator != r.BaseIterator;
		}

		template <CIteratorComparable = Iterator>
		friend auto operator <=> (TExtendedIterator const& l, TExtendedIterator const& r)
		{
			return TIteratorCompare<Iterator>(l, r);
		}

	private:
		Iterator BaseIterator;
	};
}

template <Mcro::Concepts::CRange Container, Mcro::Range::FExtendedIteratorPolicy Policy = {}>
using TIteratorExtension = Mcro::Range::TExtendedIterator<decltype(DeclVal<Container>().begin()), Policy>;

// TArray
template <typename T, typename A> auto begin( TArray<T, A>&       r) { return r.GetData(); }
template <typename T, typename A> auto begin( TArray<T, A> const& r) { return r.GetData(); }
template <typename T, typename A> auto end(   TArray<T, A>&       r) { return r.GetData() + r.Num(); }
template <typename T, typename A> auto end(   TArray<T, A> const& r) { return r.GetData() + r.Num(); }

template <typename T, typename A> auto rbegin(TArray<T, A>&       r) -> std::reverse_iterator<T*> { return r.GetData() + r.Num() - 1; }
template <typename T, typename A> auto rbegin(TArray<T, A> const& r) -> std::reverse_iterator<T*> { return r.GetData() + r.Num() - 1; }
template <typename T, typename A> auto rend(  TArray<T, A>&       r) -> std::reverse_iterator<T*> { return r.GetData() - 1; }
template <typename T, typename A> auto rend(  TArray<T, A> const& r) -> std::reverse_iterator<T*> { return r.GetData() - 1; }

template <typename T, typename A> size_t size(TArray<T, A> const& r) { return static_cast<size_t>(r.Num()); }

// TArrayView
template <typename T, typename A> auto begin( TArrayView<T, A>&       r) { return r.GetData(); }
template <typename T, typename A> auto begin( TArrayView<T, A> const& r) { return r.GetData(); }
template <typename T, typename A> auto end(   TArrayView<T, A>&       r) { return r.GetData() + r.Num(); }
template <typename T, typename A> auto end(   TArrayView<T, A> const& r) { return r.GetData() + r.Num(); }

template <typename T, typename A> auto rbegin(TArrayView<T, A>&       r) -> std::reverse_iterator<T*> { return r.GetData() + r.Num() - 1; }
template <typename T, typename A> auto rbegin(TArrayView<T, A> const& r) -> std::reverse_iterator<T*> { return r.GetData() + r.Num() - 1; }
template <typename T, typename A> auto rend(  TArrayView<T, A>&       r) -> std::reverse_iterator<T*> { return r.GetData() - 1; }
template <typename T, typename A> auto rend(  TArrayView<T, A> const& r) -> std::reverse_iterator<T*> { return r.GetData() - 1; }

template <typename T, typename A> size_t size(TArrayView<T, A> const& r) { return static_cast<size_t>(r.Num()); }

// TStaticArray
template <typename T, uint32 N, uint32 A> auto begin( TStaticArray<T, N, A>&       r) { return r.GetData(); }
template <typename T, uint32 N, uint32 A> auto begin( TStaticArray<T, N, A> const& r) { return r.GetData(); }
template <typename T, uint32 N, uint32 A> auto end(   TStaticArray<T, N, A>&       r) { return r.GetData() + r.Num(); }
template <typename T, uint32 N, uint32 A> auto end(   TStaticArray<T, N, A> const& r) { return r.GetData() + r.Num(); }

template <typename T, uint32 N, uint32 A> auto rbegin(TStaticArray<T, N, A>&       r) -> std::reverse_iterator<T*> { return r.GetData() + r.Num() - 1; }
template <typename T, uint32 N, uint32 A> auto rbegin(TStaticArray<T, N, A> const& r) -> std::reverse_iterator<T*> { return r.GetData() + r.Num() - 1; }
template <typename T, uint32 N, uint32 A> auto rend(  TStaticArray<T, N, A>&       r) -> std::reverse_iterator<T*> { return r.GetData() - 1; }
template <typename T, uint32 N, uint32 A> auto rend(  TStaticArray<T, N, A> const& r) -> std::reverse_iterator<T*> { return r.GetData() - 1; }

template <typename T, uint32 N, uint32 A> size_t size(TStaticArray<T, N, A> const& r) { return static_cast<size_t>(r.Num()); }

// TIndirectArray
template <typename T>
using TExtendedIndirectArrayIterator = Mcro::Range::TExtendedIterator<T, Mcro::Range::FExtendedIteratorPolicy{.DereferencePointerToPointer = true}>; 

template <typename T, typename A> auto begin(TIndirectArray<T, A>&       r) -> TExtendedIndirectArrayIterator<      T*> { return r.GetData(); }
template <typename T, typename A> auto begin(TIndirectArray<T, A> const& r) -> TExtendedIndirectArrayIterator<const T*> { return r.GetData(); }
template <typename T, typename A> auto end(  TIndirectArray<T, A>&       r) -> TExtendedIndirectArrayIterator<      T*> { return r.GetData() + r.Num(); }
template <typename T, typename A> auto end(  TIndirectArray<T, A> const& r) -> TExtendedIndirectArrayIterator<const T*> { return r.GetData() + r.Num(); }

// TSet
template <typename T, typename K, typename A> auto begin(TSet<T, K, A>&       r) -> TIteratorExtension<TSet<T, K, A>> { return r.begin(); }
template <typename T, typename K, typename A> auto begin(TSet<T, K, A> const& r) -> TIteratorExtension<TSet<T, K, A>> { return r.begin(); }
template <typename T, typename K, typename A> auto end(  TSet<T, K, A>&       r) -> TIteratorExtension<TSet<T, K, A>> { return r.end(); }
template <typename T, typename K, typename A> auto end(  TSet<T, K, A> const& r) -> TIteratorExtension<TSet<T, K, A>> { return r.end(); }

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
template<typename K, typename V, typename A, typename F> auto end(  TMap<K, V, A, F>&       map) { return end(  FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> auto end(  TMap<K, V, A, F> const& map) { return end(  FMapPairsAccess::GetPairs(map)); }
// TMultiMap
template<typename K, typename V, typename A, typename F> auto begin(TMultiMap<K, V, A, F>&       map) { return begin(FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> auto begin(TMultiMap<K, V, A, F> const& map) { return begin(FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> auto end(  TMultiMap<K, V, A, F>&       map) { return end(  FMapPairsAccess::GetPairs(map)); }
template<typename K, typename V, typename A, typename F> auto end(  TMultiMap<K, V, A, F> const& map) { return end(  FMapPairsAccess::GetPairs(map)); }