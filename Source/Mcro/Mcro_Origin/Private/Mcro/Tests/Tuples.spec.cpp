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
#include "Mcro/Tuples.h"
#include "Mcro/Concepts.h"

using namespace Mcro::Tuples;
using namespace Mcro::Concepts;

DEFINE_SPEC(
	FMcroTuples_Spec,
	TEXT("Mcro.Tuples"),
	EAutomationTestFlags_ApplicationContextMask
	| EAutomationTestFlags::CriticalPriority
	| EAutomationTestFlags::ProductFilter
);

void FMcroTuples_Spec::Define()
{
	Describe(TEXT("Tuple metaprogramming"), [this]
	{
		It(TEXT("should correctly manipulate tuples"), [this]
		{
			static_assert(CSameAs<
				TSkip<2, TTuple<AActor*, bool, char, FVector, FQuat>>,
				TTuple<char, FVector, FQuat>
			>);
			static_assert(CSameAs<
				TTake<2, TTuple<AActor*, bool, char, FVector, FQuat>>,
				TTuple<AActor*, bool>
			>);
			static_assert(CSameAs<
				TTrimEnd<2, TTuple<AActor*, bool, char, FVector, FQuat>>,
				TTuple<AActor*, bool, char>
			>);
			TestTrue(TEXT("This test can be compile time really."), true);
		});
	});
}
