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
#include "Widgets/SCompoundWidget.h"
#include "Mcro/Error/SErrorDisplay.h"

namespace Mcro::Error
{
	/** @brief Displaying error components which just provide a block of text (like a stack-trace) */
	class MCRO_API SPlainTextDisplay : public SErrorDisplay
	{
	public:
		SLATE_BEGIN_ARGS(SPlainTextDisplay) {}
			SLATE_ARGUMENT(IErrorRef, Error);
		SLATE_END_ARGS()

		void Construct(const FArguments& inArgs);
	};
}
