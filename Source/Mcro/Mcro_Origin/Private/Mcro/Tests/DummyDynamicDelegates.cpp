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

#include "DummyDynamicDelegates.h"

using namespace Mcro::Common::With::InferDelegate;

void UDynamicDelegateTestClass::Initialize()
{
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
	
}

int32 UDynamicDelegateTestClass::OnMultiplexed(FString const& argument)
{
	return argument.Len();
}
