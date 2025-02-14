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
#include "Algo/Count.h"
#include "Mcro/Common.h"

using namespace Mcro::Common::With::Literals;

namespace Mcro::Test
{
	class FTestSimpleError : public IError {};
}

using namespace Mcro::Test;

auto CommonTestInnerError() -> TSharedRef<FTestSimpleError>
{
	return IError::Make(new FTestSimpleError())
		->WithMessage(TEXT_"This is a test inner error")
		->WithDetails(TEXT_
			"Did you forget to do something you have definitely read in the documentation?"
		)
		->WithCodeContext(TEXT_"!SomethingNullable")
		->WithAppendix(TEXT_"Some number", TEXT_"123")
		->WithAppendix(TEXT_"Some other notes", TEXT_"My condolences")
		->WithLocation()
		->AsFatal()
		->WithCppStackTrace()
	;
}

auto CommonTestError() -> TSharedRef<FTestSimpleError>
{
	return IError::Make(new FTestSimpleError())
		->WithMessage(TEXT_"This is one test error")
		->WithDetails(TEXT_
			"Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Parturient maximus donec penatibus lectus non"
			"\nconubia amet condimentum. Tincidunt et iaculis efficitur integer, pulvinar phasellus. Mauris nisl"
			"\nparturient pharetra potenti aptent phasellus pharetra pellentesque. Leo aliquam vulputate pellentesque"
			"\nsapien gravida aptent facilisis tempus nec. Dolor aenean auctor penatibus iaculis dui justo integer"
			"\nporta. Sed vivamus porta sagittis nulla; sollicitudin class convallis mattis. Egestas lobortis nullam"
			"\nsed interdum ultricies donec."
		)
		->WithCodeContext(TEXT_"D = A + B + C")
		->WithAppendix(TEXT_"Foo", TEXT_"Lorem ipsum")
		->WithAppendix(TEXT_"Bar", TEXT_"dolor sit amet consectetur")
		->WithLocation()
		->AsFatal()
		->WithCppStackTrace()
	;
}

DEFINE_SPEC(
	FMcroError_Spec,
	TEXT_"Mcro.Error",
	EAutomationTestFlags_ApplicationContextMask
	| EAutomationTestFlags::CriticalPriority
	| EAutomationTestFlags::ProductFilter
);

void FMcroError_Spec::Define()
{
	Describe(TEXT_"IError", [this]
	{
		It(TEXT_"should do its basics", [this]
		{
			auto error = CommonTestError()
				->AsRecoverable();
			
			TestEqual(TEXT_"Error Type", error->GetType(), NAME_"Mcro::Test::FTestSimpleError");
			TestEqual(TEXT_"Error Severity", error->GetSeverityString(), TEXTVIEW_"Recoverable");
			TestEqual(TEXT_"Error Message", error->GetMessage(), STRING_"This is one test error");
			TestEqual(TEXT_"Error Details", error->GetDetails(), STRING_
				"Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Parturient maximus donec penatibus lectus non"
				"\nconubia amet condimentum. Tincidunt et iaculis efficitur integer, pulvinar phasellus. Mauris nisl"
				"\nparturient pharetra potenti aptent phasellus pharetra pellentesque. Leo aliquam vulputate pellentesque"
				"\nsapien gravida aptent facilisis tempus nec. Dolor aenean auctor penatibus iaculis dui justo integer"
				"\nporta. Sed vivamus porta sagittis nulla; sollicitudin class convallis mattis. Egestas lobortis nullam"
				"\nsed interdum ultricies donec."
			);
			TestEqual(TEXT_"Error Code context", error->GetCodeContext(), STRING_"D = A + B + C");
			ERROR_LOG(LogTemp, Display, error);
		});
	});
}

DEFINE_SPEC(
	FMcroErrorUI_Spec,
	TEXT_"Mcro.Error.UI",
	EAutomationTestFlags::EditorContext
	| EAutomationTestFlags::ClientContext
	| EAutomationTestFlags::CriticalPriority
	| EAutomationTestFlags::ProductFilter
);

void FMcroErrorUI_Spec::Define()
{
	xDescribe(TEXT_"FErrorManager", [this]
	{
		LatentIt(TEXT_"should display async error", 1_Hour, [this](FDoneDelegate const& done)
		{
			auto error = CommonTestError()
				->WithLocation()
				->WithError(
					TEXT_"Inner error",
					CommonTestInnerError()
						->WithLocation()
				);
			FErrorManager::Get().DisplayError(
				error,
				{.bAsync = true, .bImportantToRead = true, .bBreakDebugger = false, .bLogError = false}
			).Next(
				[this, done](FErrorManager::EDisplayErrorResult result)
				{
					TestEqual(TEXT_"Display result", result, FErrorManager::Displayed);
					(void) done.ExecuteIfBound();
				}
			);
		});
		
		LatentIt(TEXT_"should display modal error", 1_Hour, [this](FDoneDelegate const& done)
		{
			auto error = CommonTestError()
				->WithLocation()
				->WithError(
					TEXT_"Inner error",
					CommonTestInnerError()
						->WithLocation()
				);
			FErrorManager::Get().DisplayError(
				error,
				{.bImportantToRead = true, .bBreakDebugger = false, .bLogError = false}
			).Next(
				[this, done](FErrorManager::EDisplayErrorResult result)
				{
					TestEqual(TEXT_"Display result", result, FErrorManager::Displayed);
					(void) done.ExecuteIfBound();
				}
			);
		});
	});

	xDescribe(TEXT_"Assertions", [this]
	{
		It(TEXT_"should crash app on assertion failure", [this]
		{
			ASSERT_CRASH(false);
		});
		It(TEXT_"should crash app on invalid code path", [this]
		{
			FORCE_CRASH();
		});
	});
}