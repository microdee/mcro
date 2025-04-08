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
#include "Mcro/Rendering/Textures.h"
#include "Mcro/TextMacros.h"

#include "Engine/Texture2DDynamic.h"

namespace Mcro::Rendering::Textures
{
	FRHITexture* GetRhiTexture2D(UTexture* target)
	{
		FTextureResource* targetResource = target ? target->GetResource() : nullptr;
		return targetResource ? targetResource->GetTexture2DRHI() : nullptr;
	}

	FUnrealTextureSize GetTextureSize(UTexture* texture)
	{
		if (!IsValid(texture)) return {};
		
		auto width = texture->GetSurfaceWidth();
		auto height = texture->GetSurfaceHeight();
		EPixelFormat format = PF_Unknown;
		
		if (auto typedTexture = Cast<UTexture2D>(texture))
			format = typedTexture->GetPixelFormat();
		if (auto typedTexture = Cast<UTexture2DDynamic>(texture))
			format = typedTexture->Format;
		if (auto typedTexture = Cast<UTextureRenderTarget>(texture))
			format = typedTexture->GetFormat();
		
		ensureMsgf(format != PF_Unknown, TEXT_"Couldn't get pixel format of %s", *texture->GetClass()->GetName());
		return {width, height, format};
	}
}
