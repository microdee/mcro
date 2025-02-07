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
#include "Mcro/Common.h"
#include "DummyDynamicDelegates.h"

using namespace Mcro::Common::With::InferDelegate;
using namespace Mcro::Common::With::Literals;

// CDynamicDelegate should accept dynamic dynamic delegates
static_assert(CDynamicDelegate<FTestDynamicDelegate>);

// CDynamicDelegate shouldn't accept dynamic multicast delegates
static_assert(!CDynamicDelegate<FTestDynamicMulticastDelegate>);

// CDynamicMulticastDelegate should accept dynamic multicast delegates
static_assert(CDynamicMulticastDelegate<FTestDynamicMulticastDelegate>);

// CDynamicMulticastDelegate shouldn't accept dynamic delegates
static_assert(!CDynamicMulticastDelegate<FTestDynamicDelegate>);

// TDynamicSignature should correctly extract function signature from dynamic delegate
static_assert(CConvertibleToDecayed<
	TDynamicSignature<FTestDynamicDelegate>,
	void(int32)
>);

// TNative should correctly convert a dynamic delegate type to a compatible native delegate type
static_assert(CSameAsDecayed<
	TNative<FTestDynamicDelegate>,
	TDelegate<void(int32)>
>);

// Delegate type should be correctly inferred from function pointer type
static_assert(CSameAsDecayed<
	TInferredDelegate<void(UDynamicDelegateTestClass::*)(int32)>,
	TDelegate<void(int32)>
>);

// Delegate type should be correctly inferred from function pointer type and a list of captures
static_assert(CSameAsDecayed<
	TInferredDelegate<void(UDynamicDelegateTestClass::*)(int32, bool, float), bool, float>,
	TDelegate<void(int32)>
>);

class FTestSharedFromThisBase : public TSharedFromThis<FTestSharedFromThisBase> {};

template <typename BaseType>
class TTestNativeObject : public BaseType
{
public:
	int32 MemberFuncTest(TArray<FString>& results, FStringView append) const
	{
		results.Add(TEXT_"From member function: {0}" _FMT(append));
		return results.Num();
	}
	
	FTestDelegateWithArray GetTestMember() const
	{
		return From(this, &TTestNativeObject::MemberFuncTest, TEXTVIEW_"Capture");
	}
	
	FTestDelegateWithArray GetTestLambda() const
	{
		return From(this, [this](TArray<FString>& input)
		{
			input.Add(TEXT_"From lambda function");
			return input.Num();
		});
	}
};

struct FTestDelegateArgs
{
	int32 TestElementAt = 0;
	int32 ExpectedNum = 1;
	int32 ExpectedReturn = 1;
	FString ExpectedValue {};
};

BEGIN_DEFINE_SPEC(
	FMcroDelegates_Spec,
	TEXT_"Mcro.Delegates",
	EAutomationTestFlags_ApplicationContextMask
	| EAutomationTestFlags::CriticalPriority
	| EAutomationTestFlags::ProductFilter
)
	static int32 StaticFuncTest(TArray<FString>& results, FStringView append);

	bool TestDelegateResult(FTestDelegateWithArray const& delegate, FTestDelegateArgs const& args);
	bool TestDelegateResult(TArray<FString>& testArray, FTestDelegateWithArray const& delegate, FTestDelegateArgs const& args);
	bool TestDelegateResultArray(TArray<FString>& testArray, FTestDelegateArgs const& args);

END_DEFINE_SPEC(FMcroDelegates_Spec);

int32 FMcroDelegates_Spec::StaticFuncTest(TArray<FString>& results, FStringView append)
{
	results.Add(TEXT_"From static function: {0}" _FMT(append));
	return results.Num();
}

bool FMcroDelegates_Spec::TestDelegateResult(FTestDelegateWithArray const& delegate, FTestDelegateArgs const& args)
{
	TArray<FString> result;
	return TestDelegateResult(result, delegate, args);
}

bool FMcroDelegates_Spec::TestDelegateResult(TArray<FString>& testArray, FTestDelegateWithArray const& delegate, FTestDelegateArgs const& args)
{
	int32 testReturn = delegate.Execute(testArray);
	return TestEqual(TEXT_"Correct return value.", testReturn, args.ExpectedReturn)
		&& TestDelegateResultArray(testArray, args);
}

bool FMcroDelegates_Spec::TestDelegateResultArray(TArray<FString>& testArray, FTestDelegateArgs const& args)
{
	int result = FPlatformString::Strncmp(*testArray[args.TestElementAt], *args.ExpectedValue, testArray[args.TestElementAt].Len());
	UE_LOG(LogTemp, Display, TEXT_"Compare result = %d", result);
	return TestEqual(TEXT_"Input array modified.", testArray.Num(), 1)
		&& TestEqualSensitive(
			TEXT_"Input array has expected data.",
			testArray[args.TestElementAt],
			args.ExpectedValue
		);
}

void FMcroDelegates_Spec::Define()
{
	Describe(TEXT_"Infer delegates From", [this]
	{
		It(TEXT_"should work with functors", [this]
		{
			TestDelegateResult(
				From([this](TArray<FString>& input)
				{
					input.Add(TEXT_"From lambda function");
					return input.Num();
				}),
				{ .ExpectedValue = TEXT_"From lambda function" }
			);
		});
		It(TEXT_"should work with TSharedRef bound functors", [this]
		{
			auto object = MakeShared<TTestNativeObject<FTestSharedFromThisBase>>();
			TestDelegateResult(
				From(object, [this](TArray<FString>& input)
				{
					input.Add(TEXT_"From lambda function");
					return input.Num();
				}),
				{ .ExpectedValue = TEXT_"From lambda function" }
			);
		});
		It(TEXT_"should work with TSharedFromThis bound functors", [this]
		{
			auto object = MakeShared<TTestNativeObject<FTestSharedFromThisBase>>();
			TestDelegateResult(
				object->GetTestLambda(),
				{ .ExpectedValue = TEXT_"From lambda function" }
			);
		});
		It(TEXT_"should work with static functions", [this]
		{
			TestDelegateResult(
				From(&FMcroDelegates_Spec::StaticFuncTest, TEXTVIEW_"Capture"),
				{ .ExpectedValue = TEXT_"From static function: Capture" }
			);
		});
		It(TEXT_"should work with TSharedFromThis bound function pointers", [this]
		{
			auto object = MakeShared<TTestNativeObject<FTestSharedFromThisBase>>();
			TestDelegateResult(
				object->GetTestMember(),
				{ .ExpectedValue = TEXT_"From member function: Capture" }
			);
		});
		It(TEXT_"should work with Raw object bound function pointers", [this]
		{
			TTestNativeObject<FVoid> object;
			TestDelegateResult(
				object.GetTestMember(),
				{ .ExpectedValue = TEXT_"From member function: Capture" }
			);
		});
		It(TEXT_"should work with UObject bound functor", [this]
		{
			TScopeObject<UDynamicDelegateTestClass> object({});
			TestDelegateResult(
				object->GetTestLambda(),
				{ .ExpectedValue = TEXT_"From lambda function" }
			);
		});
		It(TEXT_"should work with UObject bound function pointers", [this]
		{
			TScopeObject<UDynamicDelegateTestClass> object({});
			TestDelegateResult(
				object->GetTestMember(),
				{ .ExpectedValue = TEXT_"From member function: Capture" }
			);
		});
		It(TEXT_"should propagate native multicast delegates", [this]
		{
			TArray<FString> result;
			
			TMulticastDelegate<void(TArray<FString>&)> someEvent;
			someEvent.AddLambda([](TArray<FString>& results)
			{
				results.Add(TEXT_"From lambda function");
			});

			auto delegate = From(someEvent);
			delegate.Execute(result);
			TestDelegateResultArray(result, { .ExpectedValue = TEXT_"From lambda function" });
		});
		It(TEXT_"should propagate dynamic multicast delegates", [this]
		{
			TScopeObject<UDynamicDelegateTestClass> object({});
			auto delegate = From(object->Event);
			delegate.Execute();
			TestDelegateResultArray(object->TestResult, { .ExpectedValue = TEXT_"From UFunction" });
		});
	});
}

