﻿/** @noop License Comment
 *  @file
 *  @copyright
 *  This Source Code is subject to the terms of the Mozilla Public License, v2.0.
 *  If a copy of the MPL was not distributed with this file You can obtain one at
 *  https://mozilla.org/MPL/2.0/
 *  
 *  @author David Mórász
 *  @date 2025
*/

#include "DummyDynamicDelegates.h"
#include "Mcro/FmtMacros.h"

using namespace Mcro::Common::With::InferDelegate;

void UDynamicDelegateTestClass::Initialize()
{
	
}

FTestDelegateWithArray UDynamicDelegateTestClass::GetTestMember() const
{
	return From(this, &UDynamicDelegateTestClass::MemberFuncTest, TEXT_"Capture");
}

FTestDelegateWithArray UDynamicDelegateTestClass::GetTestLambda() const
{
	return From(this, [this](TArray<FString>& input)
	{
		input.Add(TEXT_"From lambda function");
		return input.Num();
	});
}

int32 UDynamicDelegateTestClass::MemberFuncTest(TArray<FString>& results, const TCHAR* append) const
{
	results.Add(PRINTF_(append) "From member function: %s");
	return results.Num();
}

void UDynamicDelegateTestClass::DynamicDelegateBinding(int32 argument)
{
}

int32 UDynamicDelegateTestClass::DynamicDelegateRetValBinding(int32 argument)
{
	return argument + 1;
}

void UDynamicDelegateTestClass::OnEventTriggered(int32 argument)
{
	TestResult.Add(PRINTF_(argument) "From member UFunction with %d");
}

int32 UDynamicDelegateTestClass::OnMultiplexed(FString const& argument)
{
	return argument.Len();
}
