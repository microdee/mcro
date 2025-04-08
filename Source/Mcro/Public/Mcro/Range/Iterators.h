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

#include "Mcro/Concepts.h"
#include "Mcro/TypeName.h"
#include "Mcro/TextMacros.h"
#include "Mcro/SharedObjects.h"
#include "Mcro/Void.h"
#include "Mcro/Range/Concepts.h"

#ifndef MCRO_RANGE_ALLOW_BASIC_ITERATOR_FEATURE_EMULATION
/**
 *	@brief
 *	Iterators which can only step one item at a time, and/or doesn't expose difference computation can have the jump
 *	and difference features emulated at the cost of expensive O(N) time operations. Turning this feature off ensures
 *	stricter policy about this when working with Unreal containers and range-v3
 *
 *	When this flag is on, a runtime ensure is still triggered.
 */
#define MCRO_RANGE_ALLOW_BASIC_ITERATOR_FEATURE_EMULATION 1
#endif

namespace Mcro::Range
{
	using namespace Mcro::Concepts;
	using namespace Mcro::SharedObjects;
	using namespace Mcro::TypeName;

	template <typename T>
	struct TIteratorJumpForward_Struct {};

	template <CIsIteratorStep<EIteratorStep::Single> T>
	struct TIteratorJumpForward_Struct<T>
	{
#if MCRO_RANGE_ALLOW_BASIC_ITERATOR_FEATURE_EMULATION
		T& operator () (T& iterator, size_t steps) const
		{
			ensureAlwaysMsgf(false, TEXT_
				"Given iterator %s can only be incremented in single steps, and therefore can only be incremented in"
				" O(N) time!",
				TTypeName<T>.GetData()
			);
			for (size_t i=0; i<steps; ++i)
				++iterator;
			return iterator;
		}
#endif
	};
	
	template <CIsIteratorStep<EIteratorStep::Jump> T>
	struct TIteratorJumpForward_Struct<T>
	{
		T& operator () (T& iterator, size_t steps) const
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
#if MCRO_RANGE_ALLOW_BASIC_ITERATOR_FEATURE_EMULATION
		T& operator () (T& iterator, size_t steps) const
		{
			ensureAlwaysMsgf(false, TEXT_
				"Given iterator %s can only be decremented in single steps, and therefore can only be decremented in"
				" O(N) time!",
				TTypeName<T>.GetData()
			);
			for (size_t i=0; i<steps; ++i)
				--iterator;
			return iterator;
		}
#endif
	};
	
	template <CIteratorFeature<EIteratorDirection::Bidirectional, EIteratorStep::Jump> T>
	struct TIteratorJumpBackward_Struct<T>
	{
		T& operator () (T& iterator, size_t steps) const
		{
			return iterator -= steps;
		}
	};

	template <typename T>
	constexpr TIteratorJumpBackward_Struct<T> TIteratorJumpBackward;

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
		auto operator () (T const& l, T const& r) const
		{
			return l <=> r;
		}
	};
	
	template <CBasicForwardIterator T>
	requires CHasElementIndex<T>
	struct TIteratorCompare_Struct<T>
	{
		auto operator () (T const& l, T const& r) const
		{
			return l.ElementIndex <=> r.ElementIndex;
		}
	};
	
	template <CBasicForwardIterator T>
	requires CHasGetIndex<T>
	struct TIteratorCompare_Struct<T>
	{
		auto operator () (T const& l, T const& r) const
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
#if MCRO_RANGE_ALLOW_BASIC_ITERATOR_FEATURE_EMULATION
		int64 operator () (T const& l, T const& r) const
		{
			ensureAlwaysMsgf(false, TEXT_
				"Given iterator %s doesn't expose public state about its logical position within the range. Computing"
				" the distance between two may take O(N) time, where N is the singular steps between the two actual"
				" positions.",
				TTypeName<T>.GetData()
			);
			
			ensureAlwaysMsgf(CTotallyOrdered<T>, TEXT_
				"Given iterator %s wasn't relationally comparable. It is assumed that the right iterator is bigger than"
				" the left one. The program may freeze otherwise!",
				TTypeName<T>.GetData()
			);

			T left = l;
			T right = r;
			if constexpr (CTotallyOrdered<T>) if (l > r)
			{
				left = r;
				right = l;
			}
			
			int64 i = 0;
			for (;;)
			{
				if (!IteratorEquals(left, right)) return i;
				ensureAlwaysMsgf(i < 1000000, TEXT_
					"Computing distance between two minimal iterators %s took longer than a million steps."
					" Maybe use a different container type which can provide iterator distance in O(1) time, or heed"
					" the above warnings and make sure the right iterator is bigger than the left one.",
					TTypeName<T>.GetData()
				);
				++i;
			}
		}
#endif
	};
	
	template <CStdDistanceCompatible T>
	struct TIteratorComputeDistance_Struct<T>
	{
		auto operator () (T const& l, T const& r) const
		{
			return std::distance(l, r);
		}
	};

	template <CHasElementIndex T>
	struct TIteratorComputeDistance_Struct<T>
	{
		auto operator () (T const& l, T const& r) const
		{
			return r.ElementIndex - l.ElementIndex;
		}
	};

	template <CHasGetIndex T>
	struct TIteratorComputeDistance_Struct<T>
	{
		auto operator () (T const& l, T const& r) const
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

		template <CHasMemberAccessOperator = Iterator>
		auto operator -> ()       { return BaseIterator.operator->(); }
		
		template <CHasMemberAccessOperator = Iterator>
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

		friend auto operator - (TExtendedIterator const& l, TExtendedIterator const& r) -> difference_type
		{
			return TIteratorComputeDistance<Iterator>(l.BaseIterator, r.BaseIterator);
		}

		friend bool operator == (TExtendedIterator const& l, TExtendedIterator const& r)
		{
			return IteratorEquals(l.BaseIterator, r.BaseIterator);
		}

		friend bool operator != (TExtendedIterator const& l, TExtendedIterator const& r)
		{
			return !IteratorEquals(l.BaseIterator, r.BaseIterator);
		}

		template <CIteratorComparable = Iterator>
		friend auto operator <=> (TExtendedIterator const& l, TExtendedIterator const& r)
		{
			return TIteratorCompare<Iterator>(l.BaseIterator, r.BaseIterator);
		}

	private:
		Iterator BaseIterator;
	};

	/**
	 *	@brief
	 *	Allows range-v3 and std::ranges to iterate over temporary string objects and keep the string alive during view
	 *	and action operators.
	 */
	struct MCRO_API FTempStringIterator : std::random_access_iterator_tag
	{
		using value_type        = TCHAR;
		using iterator_category = std::random_access_iterator_tag;
		using difference_type   = std::ptrdiff_t;
		using pointer           = value_type*;
		using reference         = value_type&;

		FTempStringIterator() : Current(nullptr) {}
		FTempStringIterator(FString&& string, bool end);

		FTempStringIterator(FTempStringIterator const& other)
			: String(other.String)
			, Current(other.Current)
		{}

		FTempStringIterator(FTempStringIterator&& other) noexcept
			: String(MoveTemp(other.String))
			, Current(other.Current)
		{}

		auto operator ++ ()                -> FTempStringIterator&;
		auto operator ++ (int)             -> FTempStringIterator;
		auto operator -- ()                -> FTempStringIterator&;
		auto operator -- (int)             -> FTempStringIterator;
		auto operator *  ()                -> TCHAR const&;
		auto operator *  () const          -> TCHAR const&;
		auto operator += (int steps)       -> FTempStringIterator&;
		auto operator -= (int steps)       -> FTempStringIterator&;
		auto operator +  (int steps) const -> FTempStringIterator;
		auto operator -  (int steps) const -> FTempStringIterator;

		friend MCRO_API auto operator - (FTempStringIterator const& l, FTempStringIterator const& r) -> difference_type;
		friend FORCEINLINE auto operator <=> (FTempStringIterator const& l, FTempStringIterator const& r)
		{
			return l.Current <=> r.Current;
		}

	private:
		TSharedStoragePtr<FString> String {};
		const TCHAR* Current;
	};
}
