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
#include "Mcro/AutoModularFeature.h"
#include "Mcro/TimespanLiterals.h"

using namespace Mcro::AutoModularFeature;
using namespace Mcro::Timespan::Literals;

class ITestFeature : public TAutoModularFeature<ITestFeature> { };

class FTestFeatureImplementation : public ITestFeature, public IFeatureImplementation
{
public:
	FTestFeatureImplementation() { Register(); }
};

class ITestBelatedFeature : public TAutoModularFeature<ITestBelatedFeature> { };

class FTestBelatedFeatureImplementation : public ITestBelatedFeature, public IFeatureImplementation
{
public:
	FTestBelatedFeatureImplementation() { Register(); }
};

namespace TestImplementation
{
	class FTestImplementationInNamespace : public ITestFeature, public IFeatureImplementation
	{
	public:
		FTestImplementationInNamespace() { Register(); }
	};
}

DEFINE_SPEC(
	FMcroAutoModularFeatures_Spec,
	TEXT_"Mcro.AutoModularFeatures",
	EAutomationTestFlags_ApplicationContextMask
	| EAutomationTestFlags::HighPriority
	| EAutomationTestFlags::ProductFilter
);

void FMcroAutoModularFeatures_Spec::Define()
{
	Describe(TEXT_"AutoModularFeatures", [this]
	{
		It(TEXT_"API should work", [this]
		{
			{
				// Keep an implementation only during this scope
				FTestFeatureImplementation implementation {};
			
				TestEqual(TEXT_"Feature name", ITestFeature::FeatureName().ToString(), TEXT_"ITestFeature");
				TestEqual(TEXT_"Feature name (derived)", FTestFeatureImplementation::FeatureName().ToString(), TEXT_"ITestFeature");

				TestTrue(TEXT_"Is implemented?", ITestFeature::ImplementationCount() > 0);
				TestNotNull(TEXT_"Get implementation", ITestFeature::TryGet(0));
			}
			
			TestTrue(TEXT_"Confirm unregistering", ITestFeature::ImplementationCount() == 0);
			
			{
				// Keep an implementation only during this scope
				using namespace TestImplementation;
				FTestImplementationInNamespace implementation {};

				TestTrue(
					TEXT_"Implementation name in namespace",
					ITestFeature::ImplementationCount() > 0
				);
			}
		});

		LatentIt(TEXT_"should be available via TFuture", 30_mSec, [this](FDoneDelegate const& done)
		{
			ITestBelatedFeature::GetBelated().Next([this, &done](ITestBelatedFeature*)
			{
				(void) done.ExecuteIfBound();
			});
			
			FTestBelatedFeatureImplementation implementation {};
		});
	});
}
