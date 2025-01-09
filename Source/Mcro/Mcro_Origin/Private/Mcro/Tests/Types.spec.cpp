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
#include "Mcro/Types.h"

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

struct FBaseSomething : IHaveType
{
	FBaseSomething() { SetType(); }
};

struct FDerivedSomething : FBaseSomething
{
	FDerivedSomething() { SetType(); }
};

DEFINE_SPEC(
	FMcroTypes_Spec,
	TEXT("Mcro.Types"),
	EAutomationTestFlags_ApplicationContextMask
	| EAutomationTestFlags::CriticalPriority
	| EAutomationTestFlags::ProductFilter
);

void FMcroTypes_Spec::Define()
{
	Describe(TEXT("TTypeName template"), [this]
	{
		It(TEXT("should correctly match typenames"), [this]
		{
			auto name = TTypeName<FMcroTypes_Spec>;
			TestEqual(TEXT("Name matches"), name, TEXT("FMcroTypes_Spec"));
			name = TTypeName<FNonExistent>;
			TestEqual(TEXT("Matches incomplete types"), name, TEXT("FNonExistent"));
			name = FTestIncompleteScope::Test();
			TestEqual(TEXT("Confirm strictly scoped incompleteness"), name, TEXT("FTestIncompleteScope::Test::FIncomplete"));
			name = TTypeName<TTestTemplatedType<FMcroTypes_Spec>>;
#if PLATFORM_WINDOWS
			TestEqual(TEXT("Matches templated types"), name, TEXT("TTestTemplatedType<class FMcroTypes_Spec>"));
#else
			TestEqual(TEXT("Matches templated types"), name, TEXT("TTestTemplatedType<FMcroTypes_Spec>"));
#endif
			name = TTypeName<IHaveType>;
			TestEqual(TEXT("Matches types in namespace"), name, TEXT("Mcro::Types::IHaveType"));
			name = TTypeName<IHaveType const&>;
			TestEqual(TEXT("Ignores CV ref qualifiers"), name, TEXT("Mcro::Types::IHaveType"));
		});
	});
	
	Describe(TEXT("IHaveType base class"), [this]
	{
		It(TEXT("should correctly preserve type"), [this]
		{
			FBaseSomething something;
			TestEqual(TEXT("Type name is embedded."), something.GetType().ToString(), TEXT("FBaseSomething"));
			FDerivedSomething derived;
			FBaseSomething const& somethingRef = derived;
			TestEqual(TEXT("Type name is preserved from base lvalue-ref variable"), somethingRef.GetType().ToString(), TEXT("FDerivedSomething"));
		});
		
		It(TEXT("should dynamic cast to exact type"), [this]
		{
			auto derived = MakeShared<FDerivedSomething>();
			TSharedRef<FBaseSomething> something = derived;
			TestEqual(TEXT("Type name is preserved from base lvalue-ref variable"), something->GetType().ToString(), TEXT("FDerivedSomething"));

			auto derivedAgain = something->AsExactly<FDerivedSomething>();
			TestNotNull(TEXT("Dynamic cast to exact type"), derivedAgain.Get());
		});
	});
}
