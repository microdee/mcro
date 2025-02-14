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

#pragma once

#include "CoreMinimal.h"
#include "Mcro/Common.h"

#include "DummyDynamicDelegates.generated.h"

using FTestDelegateWithArray = TDelegate<int32(TArray<FString>&)>;

DECLARE_DYNAMIC_DELEGATE_OneParam(FTestDynamicDelegate, int32, Argument);
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(int32, FTestDynamicDelegateRetVal, int32, Argument);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTestDynamicMulticastDelegate);

UCLASS()
class UDynamicDelegateTestClass : public UObject
{
	GENERATED_BODY()

public:
	void Initialize();

	FTestDelegateWithArray GetTestMember() const;
	FTestDelegateWithArray GetTestLambda() const;
	int32 MemberFuncTest(TArray<FString>& results, const TCHAR* append) const;

	TArray<FString> TestResult;

	UPROPERTY()
	FTestDynamicDelegate DynamicDelegate;
	TDelegate<void(int32)> ResultDynamicDelegate;

	UPROPERTY()
	FTestDynamicDelegateRetVal DynamicDelegateRetVal;
	TDelegate<int32(int32)> ResultDynamicDelegateRetVal;

	UPROPERTY()
	FTestDynamicMulticastDelegate Event;

	UFUNCTION()
	void DynamicDelegateBinding();
};
