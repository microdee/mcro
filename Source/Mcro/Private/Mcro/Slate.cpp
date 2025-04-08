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

#include "Mcro/Slate.h"

namespace Mcro::Slate
{
	EVisibility IsVisible(bool visible, EVisibility hiddenState)
	{
		return visible ? EVisibility::Visible : hiddenState;
	}
}
