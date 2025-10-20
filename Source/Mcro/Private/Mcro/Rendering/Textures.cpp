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
#include "TextureResource.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DDynamic.h"
#include "Engine/TextureRenderTarget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Mcro/TextMacros.h"
#include "Mcro/AssertMacros.h"

#include "Misc/EngineVersionComparison.h"
#include "Engine/Texture2DDynamic.h"

namespace Mcro::Rendering::Textures
{
	namespace Detail
	{
		EPixelFormat GetRenderTargetFormat(UTextureRenderTarget* renderTarget)
		{
#if UE_VERSION_OLDER_THAN(5,5,0)
			if (auto rt2d = Cast<UTextureRenderTarget2D>(renderTarget))
			{
				if (rt2d->OverrideFormat == PF_Unknown)
				{
					return GetPixelFormatFromRenderTargetFormat(rt2d->RenderTargetFormat);
				}
				return rt2d->OverrideFormat;
			}
			FORCE_CRASH(
				->WithDetails(TEXT_"Not yet implemented for other rendertarget types.")
				->WithAppendix(TEXT_"RenderTargetType", renderTarget->GetClass()->GetName())
			)
			return PF_Unknown;
#else
			return renderTarget->GetFormat();
#endif
		}
	}
	
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
			format = Detail::GetRenderTargetFormat(typedTexture);
		
		ensureMsgf(format != PF_Unknown, TEXT_"Couldn't get pixel format of %s", *texture->GetClass()->GetName());
		return {width, height, format};
	}
}
