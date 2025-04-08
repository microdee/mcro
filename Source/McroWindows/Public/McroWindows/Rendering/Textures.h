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
#include "Mcro/Rendering/Textures.h"

#include "Mcro/LibraryIncludes/Start.h"
#include <dxgiformat.h>
#include "Mcro/LibraryIncludes/End.h"

namespace Mcro::Rendering::Textures
{
	template <> FORCEINLINE constexpr auto GetUnknownFormat<DXGI_FORMAT>() { return DXGI_FORMAT_UNKNOWN; }

	template <> MCROWINDOWS_API DXGI_FORMAT ConvertFormat<DXGI_FORMAT, EPixelFormat>(EPixelFormat from);
	template <> MCROWINDOWS_API EPixelFormat ConvertFormat<EPixelFormat, DXGI_FORMAT>(DXGI_FORMAT from);
	
	template <> MCROWINDOWS_API DXGI_FORMAT ConvertFormat<DXGI_FORMAT, ETextureRenderTargetFormat>(ETextureRenderTargetFormat from);
	template <> MCROWINDOWS_API ETextureRenderTargetFormat ConvertFormat<ETextureRenderTargetFormat, DXGI_FORMAT>(DXGI_FORMAT from);
}

namespace Mcro::Windows::Rendering::Textures
{
	using namespace Mcro::Rendering::Textures;

	using FDXGITextureSize = TTextureSize<uint32, DXGI_FORMAT>;
}
