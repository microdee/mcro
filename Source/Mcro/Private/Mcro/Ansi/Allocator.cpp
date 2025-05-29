#include "Mcro/Ansi/Allocator.h"
#include "Mcro/AssertMacros.h"

namespace Mcro::Ansi
{
	void Detail::OnInvalidAnsiAllocatorNum(int32 newNum, SIZE_T numBytesPerElement)
	{
		FORCE_CRASH(
			->WithMessageF(
				TEXT_"Trying to resize FAnsiAllocator to an invalid size of {0} with element size {1}",
				newNum, numBytesPerElement
			)
		)
		for (;;);
	}

	void FAllocator::ForAnyElementType::ResizeAllocation(SizeType currentNum, SizeType newMax, SIZE_T numBytesPerElement)
	{
		// Avoid calling FMemory::Realloc( nullptr, 0 ) as ANSI C mandates returning a valid pointer which is not what we want.
		if (newMax)
		{
			static_assert(sizeof(int32) <= sizeof(SIZE_T), "SIZE_T is expected to be larger than int32");

			// Check for under/overflow
			if (UNLIKELY(newMax < 0 || numBytesPerElement < 1 || numBytesPerElement > (SIZE_T)MAX_int32))
			{
				Detail::OnInvalidAnsiAllocatorNum(newMax, numBytesPerElement);
			}

			void* NewRealloc = ::realloc(Data, newMax*numBytesPerElement);
			Data = (FScriptContainerElement*)NewRealloc;
		}
		else
		{
			::free(Data);
			Data = nullptr;
		}
	}
}
