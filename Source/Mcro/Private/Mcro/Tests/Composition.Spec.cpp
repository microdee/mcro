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

	struct FSimpleComponent { int D = 3; };
	
	struct IComponentInterface { int A = 0; };
	struct FComponentBase : IComponentInterface { int B = 1; };
	struct FComponentA : FComponentBase { int C = 2; };
	struct FComponentB : FComponentBase { int C = 3; };
	struct FComponentC : FComponentBase { int C = 4; };

	struct IAnotherInterface { int E = 0; };
	struct FAutoComponentA : TInherit<IComponentInterface, IAnotherInterface> { int C = 5; };
	struct FAutoComponentB : TInherit<IComponentInterface, IAnotherInterface> { int C = 6; };
	struct FAutoComponentC : TInherit<IComponentInterface, IAnotherInterface> { int C = 7; };

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
				.With<FSimpleComponent>()
				.With<FComponentA>().With(TTypes<FComponentBase, IComponentInterface>())
				.With<FComponentB>().With(TTypes<FComponentBase, IComponentInterface>())
				.With<FComponentC>().With(TTypes<FComponentBase, IComponentInterface>())
				.With<FAutoComponentA>()
				.With<FAutoComponentB>()
				.With<FAutoComponentC>()
			;
			TestNotNull(TEXT_"FSimpleComponent", payload.TryGet<FSimpleComponent>());
			
			TestNotNull(TEXT_"FComponentA", payload.TryGet<FComponentA>());
			TestNotNull(TEXT_"FComponentB", payload.TryGet<FComponentB>());
			TestNotNull(TEXT_"FComponentC", payload.TryGet<FComponentC>());
			
			TestNotNull(TEXT_"FAutoComponentA", payload.TryGet<FAutoComponentA>());
			TestNotNull(TEXT_"FAutoComponentB", payload.TryGet<FAutoComponentB>());
			TestNotNull(TEXT_"FAutoComponentC", payload.TryGet<FAutoComponentC>());
			
			TestNull(TEXT_"Should return null for non-component", payload.TryGet<FVector>());

			auto components = payload.GetComponents<IComponentInterface>() | RenderAs<TArray>();
			TestEqual(TEXT_"Getting multiple Components",  components.Num(), 6);

			auto anotherComponents = payload.GetComponents<IAnotherInterface>() | RenderAs<TArray>();
			TestEqual(TEXT_"Support TInherit",  anotherComponents.Num(), 3);
		});
		
		It(TEXT_"should call OnComponentRegistered with supported components", [this]
		{
			namespace rv = ranges::views;
			
			int counter = 0;
			auto payload = FComposableSimple { .Confirmation = [&counter]{ ++counter; } }
				.With<FChillComponent>()
				.With<FStrictComponent>()
			;
			TestEqual(TEXT_"OnComponentRegistered confirmation", counter, 2);

			auto other = FComposableOther()
				.With<FChillComponent>()
				// .WithComponent<FStrictComponent>() // <- shouldn't compile
			;
		});
		
		It(TEXT_"should respect shared objects.", [this]
		{
			auto payload = MakeShared<FSharedComposable>()
				->With<FSimpleComponent>()
				->With<FAutoComponentA>()
				->With<FAutoComponentB>()
				->With<FAutoComponentC>()
			;
			TestNotNull(TEXT_"FSimpleComponent", payload->TryGet<FSimpleComponent>());
			TestNotNull(TEXT_"FAutoComponentA", payload->TryGet<FAutoComponentA>());
			TestNotNull(TEXT_"FAutoComponentB", payload->TryGet<FAutoComponentB>());
			TestNotNull(TEXT_"FAutoComponentC", payload->TryGet<FAutoComponentC>());
			TestNull(TEXT_"Should return null for non-component", payload->TryGet<FVector>());

			auto components = payload->GetComponents<IComponentInterface>() | RenderAs<TArray>();
			TestEqual(TEXT_"Getting multiple Components",  components.Num(), 3);
		});
	});
}
