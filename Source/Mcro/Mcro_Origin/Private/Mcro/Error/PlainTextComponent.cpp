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

#include "Mcro/Error/PlainTextComponent.h"
#include "Mcro/Error/SPlainTextDisplay.h"

namespace Mcro::Error
{
	using namespace Mcro::Yaml;
	
	void IPlainTextComponent::SerializeYaml(YAML::Emitter& emitter, bool isRoot) const
	{
		emitter << YAML::Literal << Message;
	}

	TSharedRef<SErrorDisplay> IPlainTextComponent::CreateErrorWidget()
	{
		return SNew(SPlainTextDisplay).Error(SharedThis(this));
	}
}
