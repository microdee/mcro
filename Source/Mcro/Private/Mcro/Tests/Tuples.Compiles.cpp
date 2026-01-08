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

// TTupleSkip should produce a tuple without the first N arguments of input tuple
static_assert(CSameAs<
	TTupleSkip<2, TTuple<AActor*, bool, char, FVector, FQuat>>,
	TTuple<char, FVector, FQuat>
>);

// TTupleTake should produce a tuple only from the first N arguments of input tuple
static_assert(CSameAs<
	TTupleTake<2, TTuple<AActor*, bool, char, FVector, FQuat>>,
	TTuple<AActor*, bool>
>);

// TTupleTrimEnd should produce a tuple without the last N arguments of input tuple
static_assert(CSameAs<
	TTupleTrimEnd<2, TTuple<AActor*, bool, char, FVector, FQuat>>,
	TTuple<AActor*, bool, char>
>);
