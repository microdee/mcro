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
#include "Mcro/Common.h"

using namespace Mcro::Common::With::Literals;

DEFINE_SPEC(
	FMcroTimespan_Spec,
	TEXT_"Mcro.Timespan",
	EAutomationTestFlags_ApplicationContextMask
	| EAutomationTestFlags::CriticalPriority
	| EAutomationTestFlags::ProductFilter
);

void FMcroTimespan_Spec::Define()
{
	Describe(TEXT_"Timespan literals", [this]
	{
		It(TEXT_"should produce the same values as Timespan::From functions.", [this]
		{
			TestEqual(TEXT_"Days",         30_Day,   FTimespan::FromDays(30));
			TestEqual(TEXT_"Hours",        30_Hour,  FTimespan::FromHours(30));
			TestEqual(TEXT_"Minutes",      30_Min,   FTimespan::FromMinutes(30));
			TestEqual(TEXT_"Seconds",      30_Sec,   FTimespan::FromSeconds(30));
			TestEqual(TEXT_"Milliseconds", 30_mSec,  FTimespan::FromMilliseconds(30));
			TestEqual(TEXT_"Microseconds", 30_uSec,  FTimespan::FromMicroseconds(30));
		});
	});
}
