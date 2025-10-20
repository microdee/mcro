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
#include "Mcro/CommonCore.h"
#include "Mcro/Tests/TestCompatibility.h"

using namespace Mcro::Types;

class FNonExistent;

template<typename T>
struct TTestTemplatedType {};

struct FTestIncompleteScope
{
	static FStringView Test()
	{
		return TTypeName<class FIncomplete>;
	}
};

struct FBaseSomething : IHaveTypeShareable
{
	FBaseSomething() { SetType(); }
};

struct FDerivedSomething : FBaseSomething
{
	FDerivedSomething() { SetType(); }
};

DEFINE_SPEC(
	FMcroTypes_Spec,
	TEXT_"Mcro.Types",
	EAutomationTestFlags_ApplicationContextMask
	| EAutomationTestFlags::CriticalPriority
	| EAutomationTestFlags::ProductFilter
);

void FMcroTypes_Spec::Define()
{
	Describe(TEXT_"TTypeName template", [this]
	{
		It(TEXT_"should correctly match typenames", [this]
		{
			auto name = TTypeName<FMcroTypes_Spec>;
			TestEqual(TEXT_"Name matches", FString(name), TEXT_"FMcroTypes_Spec");
			auto nameString = TTypeString<FMcroTypes_Spec>();
			TestEqual(TEXT_"Name as string matches", nameString, TEXT_"FMcroTypes_Spec");
			name = TTypeName<FNonExistent>;
			TestEqual(TEXT_"Matches incomplete types", FString(name), TEXT_"FNonExistent");
			name = FTestIncompleteScope::Test();
			TestEqual(TEXT_"Confirm strictly scoped incompleteness", FString(name), TEXT_"FTestIncompleteScope::Test::FIncomplete");
			name = TTypeName<TTestTemplatedType<FMcroTypes_Spec>>;
#if MCRO_COMPILER_MSVC
			TestEqual(TEXT_"Matches templated types", FString(name), TEXT_"TTestTemplatedType<class FMcroTypes_Spec>");
#else
			TestEqual(TEXT_"Matches templated types", FString(name), TEXT_"TTestTemplatedType<FMcroTypes_Spec>");
#endif
			name = TTypeName<IHaveType>;
			TestEqual(TEXT_"Matches types in namespace", FString(name), TEXT_"Mcro::Types::IHaveType");
			name = TTypeName<IHaveType const&>;
			TestEqual(TEXT_"Ignores CV ref qualifiers", FString(name), TEXT_"Mcro::Types::IHaveType");
		});
	});
	
	Describe(TEXT_"IHaveType base class", [this]
	{
		It(TEXT_"should correctly preserve type", [this]
		{
			FBaseSomething something;
			TestEqual(TEXT_"Type name is embedded.", something.GetType().ToStringCopy(), TEXT_"FBaseSomething");
			FDerivedSomething derived;
			FBaseSomething const& somethingRef = derived;
			TestEqual(TEXT_"Type name is preserved from base lvalue-ref variable", somethingRef.GetType().ToStringCopy(), TEXT_"FDerivedSomething");
		});
		
		It(TEXT_"should dynamic cast to exact type", [this]
		{
			auto derived = MakeShared<FDerivedSomething>();
			TSharedRef<FBaseSomething> something = derived;
			TestEqual(TEXT_"Type name is preserved from base lvalue-ref variable", something->GetType().ToStringCopy(), TEXT_"FDerivedSomething");

			auto derivedAgain = something->As<FDerivedSomething>();
			TestNotNull(TEXT_"Dynamic cast to exact type", derivedAgain.Get());
		});
	});
}
