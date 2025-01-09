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

#include "CoreMinimal.h"
#include "Mcro/Error/PlainTextComponent.h"

namespace Mcro::Error
{
	/**
	 * An Error component which stores a BP stack trace in its message upon construction
	 */
	class MCRO_API FBlueprintStackTrace : public IPlainTextComponent
	{
	public:
		FBlueprintStackTrace();
	};
}
