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

	struct FComposableSimple : IComposable
	{
		TFunction<void()> Confirmation;
	};

	struct FComposableOther : IComposable {};

	struct FSharedComposable : IComposable, TSharedFromThis<FSharedComposable> {};
	
	struct IComponentInterface
	{
		int A = 0;
	};

	struct FComponentBase: IComponentInterface
	{
		int B = 1;
	};

	struct FComponentA: FComponentBase
	{
		int C = 2;
	};

	struct FComponentB: FComponentBase
	{
		int C = 3;
	};

	struct FComponentC: FComponentBase
	{
		int C = 4;
	};

	struct FSimpleComponent
	{
		int D = 3;
	};

	struct FChillComponent : IComponent 
	{
		void OnComponentRegistered(FComposableSimple& to) const
		{
			to.Confirmation();
		}
	};

	struct FStrictComponent : IStrictComponent 
	{
		void OnComponentRegistered(FComposableSimple& to) const
		{
			to.Confirmation();
		}
	};
}

DEFINE_SPEC(
	FMcroComposition_Spec,
	TEXT_"Mcro.Composition",
	EAutomationTestFlags_ApplicationContextMask
	| EAutomationTestFlags::CriticalPriority
	| EAutomationTestFlags::ProductFilter
);

void FMcroComposition_Spec::Define()
{
	using namespace Mcro::Common;
	using namespace Mcro::Test;

	Describe(TEXT_"IComposable", [this]
	{
		It(TEXT_"should respect type safety.", [this]
		{
			auto payload = FComposableSimple()
				.WithComponent<FSimpleComponent>()
				.WithComponent<FComponentA>().With(TAlias<FComponentBase, IComponentInterface>())
				.WithComponent<FComponentB>().With(TAlias<FComponentBase, IComponentInterface>())
				.WithComponent<FComponentC>().With(TAlias<FComponentBase, IComponentInterface>())
			;
			TestNotNull(TEXT_"FSimpleComponent", payload.TryGetComponent<FSimpleComponent>());
			TestNotNull(TEXT_"FComponentA", payload.TryGetComponent<FComponentA>());
			TestNotNull(TEXT_"FComponentB", payload.TryGetComponent<FComponentB>());
			TestNotNull(TEXT_"FComponentC", payload.TryGetComponent<FComponentC>());
			TestNull(TEXT_"Should return null for non-component", payload.TryGetComponent<FVector>());

			auto components = payload.GetComponents<IComponentInterface>() | RenderAs<TArray>();
			TestEqual(TEXT_"Getting multiple Components",  components.Num(), 3);
		});
		
		It(TEXT_"should call OnComponentRegistered with supported components", [this]
		{
			namespace rv = ranges::views;
			
			int counter = 0;
			auto payload = FComposableSimple { .Confirmation = [&counter]{ ++counter; } }
				.WithComponent<FChillComponent>()
				.WithComponent<FStrictComponent>()
			;
			TestEqual(TEXT_"OnComponentRegistered confirmation", counter, 2);

			auto other = FComposableOther()
				.WithComponent<FChillComponent>()
				// .WithComponent<FStrictComponent>() // <- shouldn't compile
			;
		});
		
		It(TEXT_"should respect shared objects.", [this]
		{
			auto payload = MakeShared<FSharedComposable>()
				->WithComponent<FSimpleComponent>()
				->WithComponent<FComponentA>()->With(TAlias<FComponentBase, IComponentInterface>())
				->WithComponent<FComponentB>()->With(TAlias<FComponentBase, IComponentInterface>())
				->WithComponent<FComponentC>()->With(TAlias<FComponentBase, IComponentInterface>())
			;
			TestNotNull(TEXT_"FSimpleComponent", payload->TryGetComponent<FSimpleComponent>());
			TestNotNull(TEXT_"FComponentA", payload->TryGetComponent<FComponentA>());
			TestNotNull(TEXT_"FComponentB", payload->TryGetComponent<FComponentB>());
			TestNotNull(TEXT_"FComponentC", payload->TryGetComponent<FComponentC>());
			TestNull(TEXT_"Should return null for non-component", payload->TryGetComponent<FVector>());

			auto components = payload->GetComponents<IComponentInterface>() | RenderAs<TArray>();
			TestEqual(TEXT_"Getting multiple Components",  components.Num(), 3);
		});
	});
}
