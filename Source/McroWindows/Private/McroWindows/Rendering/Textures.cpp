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

#include "McroWindows/Rendering/Textures.h"

namespace Mcro::Rendering::Textures
{
	template <> DXGI_FORMAT ConvertFormat<DXGI_FORMAT, EPixelFormat>(EPixelFormat from)
	{
		return static_cast<DXGI_FORMAT>(GPixelFormats[from].PlatformFormat);
	}

	template <> EPixelFormat ConvertFormat<EPixelFormat, DXGI_FORMAT>(DXGI_FORMAT from)
	{
		static TMap<DXGI_FORMAT, EPixelFormat> formatCache;
		if (EPixelFormat* result = formatCache.Find(from))
			return *result;

		for (int32 i = 0; i < PF_MAX; ++i)
		{
			const auto& formatDesc = GPixelFormats[i];
			auto currFormat = static_cast<DXGI_FORMAT>(formatDesc.PlatformFormat);
			if (currFormat == from)
			{
				formatCache.Add(currFormat, formatDesc.UnrealFormat);
				return formatDesc.UnrealFormat;
			}
		}

		return PF_Unknown;
	}

	template <>
	DXGI_FORMAT ConvertFormat<DXGI_FORMAT, ETextureRenderTargetFormat>(ETextureRenderTargetFormat from)
	{
		return ConvertFormat<DXGI_FORMAT>(ConvertFormat<EPixelFormat>(from));
	}

	template <>
	ETextureRenderTargetFormat ConvertFormat<ETextureRenderTargetFormat, DXGI_FORMAT>(DXGI_FORMAT from)
	{
		return ConvertFormat<ETextureRenderTargetFormat>(ConvertFormat<EPixelFormat>(from));
	}
}
