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

/**
 * @file
 * This header is there to ensure AutomationTest compatibility between different engine versions
 */

#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"

#if UE_VERSION_OLDER_THAN(5,5,0)

/**
 * @brief
 * TestEqualSensitive is introduced in Unreal 5.5, until then we simply replace it with TestEqual
 * which should do the same thing.
 */
#define TestEqualSensitive(...) TestEqual(__VA_ARGS__)

/**
 * @brief
 * EAutomationTestFlags has been refactored in newer engine versions. Emulating refactored values here:
 */
constexpr EAutomationTestFlags::Type EAutomationTestFlags_ApplicationContextMask = EAutomationTestFlags::ApplicationContextMask;
#endif