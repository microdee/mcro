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
#include "Mcro/Error.h"

namespace Mcro::Error
{
	/** @brief An error component which displays only its Message, used as simple modular plain text storage */
	class MCRO_API IPlainTextComponent : public IError
	{
	protected:
		virtual void AddError(const FString& name, const TSharedRef<IError>& error, const FString& typeOverride = {}) override {}
		virtual void SerializeInnerErrors(YAML::Emitter&) const override {}

		virtual void SerializeYaml(YAML::Emitter& emitter) const override;
		virtual TSharedRef<SErrorDisplay> CreateErrorWidget() override;
	};
	
}
