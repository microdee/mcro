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

/**
 *	@brief
 *	Epic Games may not agree with standards because they know better, but sometimes we have to bare consequences of
 *	such wisdom. This namespace contains utilities which politely circumvent the obviously superior decisions Epic Games
 *	has made.
 */
namespace Mcro::Ansi
{
	namespace Detail
	{
		MCRO_API void OnInvalidAnsiAllocatorNum(int32 newNum, SIZE_T numBytesPerElement);
	}

	/**
	 *	@brief  Allocator that allocates memory using standard library functions.
	 *
	 *	This is a copy of `FAnsiAllocator` in `Runtime/Core/Private/HAL/Allocators/AnsiAllocator.h` but since it's
	 *	private, we have the MCRO version of it here.
	 */
	class MCRO_API FAllocator
	{
	public:
		using SizeType = int32;

		enum { NeedsElementType = false };
		enum { RequireRangeCheck = true };

		typedef FAllocator ElementAllocator;
		typedef FAllocator BitArrayAllocator;

		class MCRO_API ForAnyElementType
		{
		public:
			ForAnyElementType() : Data(nullptr) {}

			/**
			 *	@brief  Moves the state of another allocator into this one.
			 *	
			 *	Assumes that the allocator is currently empty, i.e. memory may be allocated but any existing elements
			 *	have already been destructed (if necessary).
			 *	
			 *	@param other  The allocator to move the state from.  This allocator should be left in a valid empty state.
			 */
			FORCEINLINE void MoveToEmpty(ForAnyElementType& other)
			{
				check(this != &other);

				if (Data) ::free(Data);

				Data = other.Data;
				other.Data = nullptr;
			}

			/** Destructor. */
			FORCEINLINE ~ForAnyElementType()
			{
				if (Data) ::free(Data);
			}

			// FContainerAllocatorInterface
			FORCEINLINE FScriptContainerElement* GetAllocation() const
			{
				return Data;
			}
			void ResizeAllocation(SizeType currentNum, SizeType newMax, SIZE_T numBytesPerElement);
			SizeType CalculateSlackReserve(SizeType newMax, SIZE_T numBytesPerElement) const
			{
				return DefaultCalculateSlackReserve(newMax, numBytesPerElement, false);
			}
			SizeType CalculateSlackShrink(SizeType newMax, SizeType currentMax, SIZE_T numBytesPerElement) const
			{
				return DefaultCalculateSlackShrink(newMax, currentMax, numBytesPerElement, false);
			}
			SizeType CalculateSlackGrow(SizeType newMax, SizeType currentMax, SIZE_T numBytesPerElement) const
			{
				return DefaultCalculateSlackGrow(newMax, currentMax, numBytesPerElement, false);
			}

			SIZE_T GetAllocatedSize(SizeType currentMax, SIZE_T numBytesPerElement) const
			{
				return currentMax * numBytesPerElement;
			}

			bool HasAllocation() const
			{
				return !!Data;
			}

			SizeType GetInitialCapacity() const
			{
				return 0;
			}

		private:
			ForAnyElementType(const ForAnyElementType&) = delete;
			ForAnyElementType& operator=(const ForAnyElementType&) = delete;

			/** A pointer to the container's elements. */
			FScriptContainerElement* Data;
		};

		template<typename ElementType>
		class MCRO_API ForElementType : public ForAnyElementType
		{
		public:

			/** Default constructor. */
			ForElementType()
			{}

			FORCEINLINE ElementType* GetAllocation() const
			{
				return (ElementType*)ForAnyElementType::GetAllocation();
			}
		};
	};

	/** @brief Allocator for sets to follow standard allocation behavior */
	class FSetAllocator : public TSetAllocator<FAllocator, TInlineAllocator<1, FAllocator>> {};
}

/** @brief TArray alias which enforces standard memory allocations */
template <typename T>
using TAnsiArray = TArray<T, Mcro::Ansi::FAllocator>;

/** @brief TSet alias which enforces standard memory allocations */
template <typename T, typename KeyFuncs = DefaultKeyFuncs<T>>
using TAnsiSet = TSet<T, KeyFuncs, Mcro::Ansi::FSetAllocator>;

/** @brief TMap alias which enforces standard memory allocations */
template <typename K, typename V, typename KeyFuncs = TDefaultMapHashableKeyFuncs<K, V, false>>
using TAnsiMap = TMap<K, V, Mcro::Ansi::FSetAllocator, KeyFuncs>;

template <>
struct TAllocatorTraits<Mcro::Ansi::FAllocator> : TAllocatorTraitsBase<Mcro::Ansi::FAllocator>
{
	enum { IsZeroConstruct = true };
};
