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
#include "Misc/AutomationTest.h"
#include "Algo/Count.h"
#include "Mcro/Common.h"
#include "Mcro/Tests/TestCompatibility.h"

using namespace Mcro::Common;

static_assert(!CClass<bool>, "bool is not class");
static_assert(StatePolicyFor<bool>.StorePrevious, "bool should store previous");

DEFINE_SPEC(
	FMcroObservable_Spec,
	TEXT_"Mcro.Observable",
	EAutomationTestFlags_ApplicationContextMask
	| EAutomationTestFlags::CriticalPriority
	| EAutomationTestFlags::ProductFilter
);

void FMcroObservable_Spec::Define()
{
	Describe(TEXT_"TState", [this]
	{
		It(TEXT_"should do its job", [this]
		{
			TState<int> state(-2);

			auto [value, dummyLock] = state.GetOnAnyThread();

			TestTrue(TEXT_"Lock is dummy type when thread safety is not enabled", dummyLock->IsType<FVoid>());
			
			TestEqual(TEXT_"Initial value", state.Get(), -2);
			TestTrue(TEXT_"Reactive change", state.HasChangedFrom(1));
			TestEqual(TEXT_"Current value 0", state.Get(), 1);
			TestFalse(TEXT_"Reactive change only happens on actual change", state.HasChangedFrom(1));

			int changeCounter = 0, nextCache = 0, previousCache = 0;
			state.OnChange([this, &changeCounter, &nextCache, &previousCache](int next, TOptional<int> const& previous)
			{
				++changeCounter;
				nextCache = next; previousCache = previous.Get(-1);
				UE_LOG(LogTemp, Display, TEXT_"Next value: %d | Previous value: %d", next, previous.Get(-1));
			}, {.Belated = true});
			
			TestEqual(TEXT_"Should call on change", changeCounter, 1);
			TestEqual(TEXT_"Next value", nextCache, 1);
			TestEqual(TEXT_"Previous value", previousCache, -2);

			state = 2;
			
			TestEqual(TEXT_"Should call on change", changeCounter, 2);
			TestEqual(TEXT_"Next value", nextCache, 2);
			TestEqual(TEXT_"Previous value", previousCache, 1);
		});
	});
}