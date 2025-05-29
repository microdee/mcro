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
#include "TestHelpers.h"

#include "Mcro/CommonCore.h"

namespace Mcro::Test
{
	using namespace Mcro::Common;
	
	struct FAnyTestBase { int A = 1; };
	struct FAnyTest : FAnyTestBase { int B = 2; };

	struct IBaseA {};
	struct IBaseB {};
	struct IBaseC {};

	struct FIntrusiveInherit : TInherit<IBaseA, IBaseB, IBaseC> {};
}

DEFINE_SPEC(
	FMcroAny_Spec,
	TEXT_"Mcro.Any",
	EAutomationTestFlags_ApplicationContextMask
	| EAutomationTestFlags::CriticalPriority
	| EAutomationTestFlags::ProductFilter
);

void FMcroAny_Spec::Define()
{
	using namespace Mcro::Test;

	Describe(TEXT_"FAny", [this]
	{
		It(TEXT_"should respect type safety.", [this]
		{
			auto payload = FAny(new FAnyTest()).WithAlias<FAnyTestBase>();
			
			TestNotNull(TEXT_"Fetch with exact type", payload.TryGet<FAnyTest>());
			TestNotNull(TEXT_"Fetch with alias type", payload.TryGet<FAnyTestBase>());
			TestNull(TEXT_"Shouldn't allow unrelated types", payload.TryGet<FVector>());
		});
		
		It(TEXT_"should use intrusive inheritance.", [this]
		{
			auto payload = FAny(new FIntrusiveInherit());
			
			TestNotNull(TEXT_"Fetch with exact type", payload.TryGet<FIntrusiveInherit>());
			TestNotNull(TEXT_"Fetch with alias type A", payload.TryGet<IBaseA>());
			TestNotNull(TEXT_"Fetch with alias type B", payload.TryGet<IBaseB>());
			TestNotNull(TEXT_"Fetch with alias type C", payload.TryGet<IBaseC>());
		});
		
		It(TEXT_"should be copyable/movable", [this]
		{
			auto payload = FAny(new FCopyConstructCounter());
			check(payload.TryGet<FCopyConstructCounter>());
			TestEqual(TEXT_"No initial copy", payload.TryGet<FCopyConstructCounter>()->CopyCount, 0);
			TestEqual(TEXT_"No initial move", payload.TryGet<FCopyConstructCounter>()->MoveCount, 0);
			FAny copy = payload;
			check(copy.TryGet<FCopyConstructCounter>());
			TestEqual(TEXT_"Copy once", copy.TryGet<FCopyConstructCounter>()->CopyCount, 1);
			FAny movedCopy { MoveTemp(copy) };
			check(movedCopy.TryGet<FCopyConstructCounter>());
			TestFalse(TEXT_"Source should be invalid", copy.IsValid());
			TestEqual(TEXT_"Copy shouldn't change", movedCopy.TryGet<FCopyConstructCounter>()->CopyCount, 1);
			TestEqual(TEXT_"Contents shouldn't actually move", movedCopy.TryGet<FCopyConstructCounter>()->MoveCount, 0);
		});
		
		It(TEXT_"should support object lifespan customization", [this]
		{
			TMap<int, FAnyTest> stupidPool { {1, FAnyTest{.B = 1}} };

			{
				TAnyTypeFacilities<FAnyTest> customLifespan
				{
					.Destruct = [&stupidPool](FAnyTest* i)
					{
						check(i);
						stupidPool.Remove(i->B);
					},
					.CopyConstruct = [&stupidPool](FAnyTest const& i)
					{
						decltype(auto) result = stupidPool.Add(i.B + 1, FAnyTest{.B = i.B + 1});
						return &result;
					}
				};
				
				auto payload = FAny(stupidPool.Find(1), customLifespan);
				check(payload.TryGet<FAnyTest>());
				TestEqual(TEXT_"No initial copy", stupidPool.Num(), 1);
				{
					FAny copy = payload;
					check(copy.TryGet<FAnyTest>());
					TestEqual(TEXT_"Copy once", stupidPool.Num(), 2);
				}
				TestEqual(TEXT_"Copy gone out of scope", stupidPool.Num(), 1);
			}
			TestEqual(TEXT_"Original gone out of scope", stupidPool.Num(), 0);
		});
	});
}
