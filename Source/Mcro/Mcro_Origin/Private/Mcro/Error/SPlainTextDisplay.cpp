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

#include "Mcro/Error/SPlainTextDisplay.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

namespace Mcro::Error
{
	void SPlainTextDisplay::Construct(const FArguments& inArgs)
	{
		using namespace Mcro::Slate;
		ChildSlot
		[
			SNew(SEditableTextBox)
			/ Text(inArgs._Error->GetMessage())
		];
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
