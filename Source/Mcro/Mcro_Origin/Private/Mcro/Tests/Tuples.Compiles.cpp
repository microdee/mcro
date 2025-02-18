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
#include "Mcro/CommonCore.h"

using namespace Mcro::Common;

// TSkip should produce a tuple without the first N arguments of input tuple
static_assert(CSameAs<
	TSkip<2, TTuple<AActor*, bool, char, FVector, FQuat>>,
	TTuple<char, FVector, FQuat>
>);

// TTake should produce a tuple only from the first N arguments of input tuple
static_assert(CSameAs<
	TTake<2, TTuple<AActor*, bool, char, FVector, FQuat>>,
	TTuple<AActor*, bool>
>);

// TTrimEnd should produce a tuple without the last N arguments of input tuple
static_assert(CSameAs<
	TTrimEnd<2, TTuple<AActor*, bool, char, FVector, FQuat>>,
	TTuple<AActor*, bool, char>
>);
