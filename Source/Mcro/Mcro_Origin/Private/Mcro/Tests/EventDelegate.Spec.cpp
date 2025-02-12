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
#include "Algo/Count.h"
#include "Mcro/Common.h"

using namespace Mcro::Common::With::InferDelegate;

struct FCopyForbidden : FNoncopyable {};

struct FCopyConstructCounter
{
	FCopyConstructCounter() {}
	FCopyConstructCounter(FCopyConstructCounter const& other)
		: CopyCount(other.CopyCount + 1)
	{}
	FCopyConstructCounter(FCopyConstructCounter&& other) noexcept
		: MoveCount(other.MoveCount + 1)
	{}

	auto operator=(FCopyConstructCounter const& other) -> FCopyConstructCounter&
	{
		if (this == &other) return *this;

		CopyCount = other.CopyCount + 1;
		return *this;
	}

	auto operator=(FCopyConstructCounter&& other) noexcept -> FCopyConstructCounter&
	{
		if (this == &other) return *this;
		
		MoveCount = other.MoveCount + 1;
		return *this;
	}

	int32 CopyCount = 0;
	int32 MoveCount = 0;
};

DEFINE_SPEC(
	FMcroEventDelegate_Spec,
	TEXT_"Mcro.EventDelegate",
	EAutomationTestFlags_ApplicationContextMask
	| EAutomationTestFlags::CriticalPriority
	| EAutomationTestFlags::ProductFilter
);

void FMcroEventDelegate_Spec::Define()
{
	Describe(TEXT_"TEventDelegate", [this]
	{
		It(TEXT_"should respect individual binding preferences", [this]
		{
			TArray<FString> result;
			TEventDelegate<void(TArray<FString>&)> event;
			event.Add(From([](TArray<FString>& resultArg)
			{
				resultArg.Add(TEXT_"Always called");
			}));
			event.Add(From([](TArray<FString>& resultArg)
			{
				resultArg.Add(TEXT_"Called once");
			}), {.Once = true});
			
			event.Broadcast(result);
			TestEqual(TEXT_"Entries in result should be 2", result.Num(), 2);
			TestTrue(TEXT_"Has the entry from regular binding", result.Contains(STRING_"Always called"));
			TestTrue(TEXT_"Has the entry from only-once binding", result.Contains(STRING_"Called once"));
			
			event.Broadcast(result);
			TestEqual(TEXT_"Entries in result should be 3", result.Num(), 3);
			TestEqual(TEXT_"Has 2 entries from regular binding", Algo::Count(result, STRING_"Always called"), 2UL);
			TestEqual(TEXT_"Has 1 entry from only-once binding", Algo::Count(result, STRING_"Called once"), 1UL);
			
			event.Add(From([](TArray<FString>& resultArg)
			{
				resultArg.Add(TEXT_"Called Belated");
			}), {.Belated = true});
			
			TestEqual(TEXT_"Entries in result should be 4", result.Num(), 4);
			TestEqual(TEXT_"Has 1 entry from belated binding", Algo::Count(result, STRING_"Called Belated"), 1UL);
			TestEqual(TEXT_"Has 2 entries from regular binding", Algo::Count(result, STRING_"Always called"), 2UL);
			TestEqual(TEXT_"Has 1 entry from only-once binding", Algo::Count(result, STRING_"Called once"), 1UL);
			
			event.Broadcast(result);
			TestEqual(TEXT_"Entries in result should be 6", result.Num(), 6);
			TestEqual(TEXT_"Has 2 entry from belated binding", Algo::Count(result, STRING_"Called Belated"), 2UL);
			TestEqual(TEXT_"Has 3 entries from regular binding", Algo::Count(result, STRING_"Always called"), 3UL);
			TestEqual(TEXT_"Has 1 entry from only-once binding", Algo::Count(result, STRING_"Called once"), 1UL);
			
			event.Add(From([](TArray<FString>& resultArg)
			{
				resultArg.Add(TEXT_"Called Once and Belated");
			}), {.Once = true, .Belated = true});

			TestEqual(TEXT_"Entries in result should be 7", result.Num(), 7);
			TestEqual(TEXT_"Has 1 entry from belated and only-once binding", Algo::Count(result, STRING_"Called Once and Belated"), 1UL);
			TestEqual(TEXT_"Has 2 entry from belated binding", Algo::Count(result, STRING_"Called Belated"), 2UL);
			TestEqual(TEXT_"Has 3 entries from regular binding", Algo::Count(result, STRING_"Always called"), 3UL);
			TestEqual(TEXT_"Has 1 entry from only-once binding", Algo::Count(result, STRING_"Called once"), 1UL);
			
			event.Broadcast(result);
			TestEqual(TEXT_"Entries in result should be 9", result.Num(), 9);
			TestEqual(TEXT_"Has 1 entry from belated and only-once binding", Algo::Count(result, STRING_"Called Once and Belated"), 1UL);
			TestEqual(TEXT_"Has 3 entry from belated binding", Algo::Count(result, STRING_"Called Belated"), 3UL);
			TestEqual(TEXT_"Has 4 entries from regular binding", Algo::Count(result, STRING_"Always called"), 4UL);
			TestEqual(TEXT_"Has 1 entry from only-once binding", Algo::Count(result, STRING_"Called once"), 1UL);
		});
		
		It(TEXT_"should only copy broadcast arguments when CopyArguments is active", [this]
		{
			FCopyForbidden cannotCopy;
			FCopyConstructCounter shouldNeverCopy;
			FCopyConstructCounter shouldCopy;
			
			TEventDelegate<void(FCopyConstructCounter const&)> nonCopyEvent;
			TEventDelegate<void(FCopyForbidden const&)> cannotCopyEvent;
			TRetainingEventDelegate<void(FCopyConstructCounter const&)> retainingEvent;

			nonCopyEvent.Add(From([this](FCopyConstructCounter const& payload)
			{
				TestEqual(TEXT_"No copy should be made for initial binding", payload.CopyCount, 0);
				TestEqual(TEXT_"No copy should be made for initial binding", payload.MoveCount, 0);
			}));

			retainingEvent.Add(From([this](FCopyConstructCounter const& payload)
			{
				TestEqual(TEXT_"No copy should be made for initial binding", payload.CopyCount, 0);
				TestEqual(TEXT_"No copy should be made for initial binding", payload.MoveCount, 0);
			}));

			cannotCopyEvent.Add(From([this](FCopyForbidden const& payload)
			{
				UE_LOG(LogTemp, Display, TEXT_"Non-copyable compiles correctly");
			}));

			nonCopyEvent.Broadcast(shouldNeverCopy);
			retainingEvent.Broadcast(shouldCopy);
			cannotCopyEvent.Broadcast(cannotCopy);

			nonCopyEvent.Add(From([this](FCopyConstructCounter const& payload)
			{
				TestEqual(TEXT_"Belated invoke should get a reference", payload.CopyCount, 0);
				TestEqual(TEXT_"Belated invoke should get a reference", payload.MoveCount, 0);
			}), {.Belated = true});

			retainingEvent.Add(From([this](FCopyConstructCounter const& payload)
			{
				TestEqual(TEXT_"Belated invoke should get move constructed object", payload.CopyCount, 0);
				TestEqual(TEXT_"Belated invoke should get move constructed object", payload.MoveCount, 1);
			}), {.Belated = true});
		});
	});
}
