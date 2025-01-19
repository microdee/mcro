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
#include "Mcro/Concepts.h"
#include "Mcro/AssertMacros.h"
#include "Engine/TextureRenderTarget2D.h"

/** @brief This namespace contain utilities regarding high-level cross-platform and cross-rhi texture objects */
namespace Mcro::Rendering::Textures
{
	using namespace Mcro::Concepts;
	
	/** @brief Get the lower level RHI texture from a high level UTexture2D object if it's possible, nullptr otherwise */
	MCRO_API FRHITexture* GetRhiTexture2D(UTexture* target);

	/** @brief Describing a struct which can give texture size and layout information */
	template <typename T>
	concept CTextureSize = requires(T t, typename T::FormatType)
	{
		t.Width;
		t.Height;
		t.Format;
	};

	/** @brief Specialize this function template to define the unknown format for an RHI texture format */
	template <CEnum FormatType>
	constexpr auto GetUnknownFormat() { return static_cast<FormatType>(0); }

	template <> constexpr auto GetUnknownFormat<EPixelFormat>() { return PF_Unknown; }
	template <> constexpr auto GetUnknownFormat<ETextureRenderTargetFormat>() { return RTF_R8; }

	/** @brief Specialize this function template to define format conversion of equivalent formats between RHIs */
	template <CEnum ToFormatType, CEnum FromFormatType>
	requires (!CSameAs<ToFormatType, FromFormatType>)
	ToFormatType ConvertFormat(FromFormatType from)
	{
		return static_cast<ToFormatType>(from);
	}

	/** @brief Special case when ToFormatType and FromFormatType are the same there's no need for conversion */
	template <CEnum ToFormatType, CEnum FromFormatType>
	requires CSameAs<ToFormatType, FromFormatType>
	ToFormatType ConvertFormat(FromFormatType from)
	{
		return from;
	}

	template <>
	FORCEINLINE_DEBUGGABLE ETextureRenderTargetFormat ConvertFormat<ETextureRenderTargetFormat, EPixelFormat>(EPixelFormat from)
	{
		switch (from)
		{
		case PF_G8: return RTF_R8;
		case PF_R8G8: return RTF_RG8;
		case PF_B8G8R8A8: return RTF_RGBA8;

		case PF_R16F: return RTF_R16f;
		case PF_G16R16F: return RTF_RG16f;
		case PF_FloatRGBA: return RTF_RGBA16f;

		case PF_R32_FLOAT: return RTF_R32f;
		case PF_G32R32F: return RTF_RG32f;
		case PF_A32B32G32R32F: return RTF_RGBA32f;
		case PF_A2B10G10R10: return RTF_RGB10A2;
		default:
			{
				ASSERT_QUIT(false, RTF_R8,
					->WithMessageF(TEXT("Unhandled ETextureRenderTargetFormat entry %s"), GetPixelFormatString(from))
				)
				return RTF_R8;
			};
		}
	}
	
	template <>
	FORCEINLINE_DEBUGGABLE EPixelFormat ConvertFormat<EPixelFormat, ETextureRenderTargetFormat>(ETextureRenderTargetFormat from)
	{
		// I know about GetPixelFormatFromRenderTargetFormat but I also have better error handling
		switch (from)
		{
		case RTF_R8: return PF_G8;
		case RTF_RG8: return PF_R8G8;
		case RTF_RGBA8: return PF_B8G8R8A8;
		case RTF_RGBA8_SRGB: return PF_B8G8R8A8;

		case RTF_R16f: return PF_R16F;
		case RTF_RG16f: return PF_G16R16F;
		case RTF_RGBA16f: return PF_FloatRGBA;

		case RTF_R32f: return PF_R32_FLOAT;
		case RTF_RG32f: return PF_G32R32F;
		case RTF_RGBA32f: return PF_A32B32G32R32F;
		case RTF_RGB10A2: return PF_A2B10G10R10;
		default:
			{
				auto enumName = UEnum::GetValueAsString(from);
				ASSERT_QUIT(false, PF_Unknown,
					->WithMessageF(TEXT("Unhandled ETextureRenderTargetFormat entry %s"), *enumName)
				);
				return PF_Unknown;
			}
		}
	}

	/**
	 *	@brief  A simple texture size description which can be used for checking the need to recreate a texture resource.
	 *	@tparam SizeType      the integral type for texture size
	 *	@tparam InFormatType  the type of the format enum
	 *	@todo   This may get generalized to other type of resources in the future.
	 */
	template <CScalar SizeType, CEnum InFormatType>
	struct TTextureSize
	{
		using FormatType = InFormatType;
		
		TTextureSize() {};
		
		template <CScalar SizeTypeArg, CEnum FormatTypeArg>
		TTextureSize(SizeTypeArg width, SizeTypeArg height, FormatTypeArg format)
			: Width(static_cast<SizeType>(width))
			, Height(static_cast<SizeType>(height))
			, Format(ConvertFormat<InFormatType>(format))
		{}

		template <CTextureSize OtherTextureSize>
		TTextureSize(OtherTextureSize const& other)
			: Width(static_cast<SizeType>(other.Width))
			, Height(static_cast<SizeType>(other.Height))
			, Format(ConvertFormat<InFormatType>(other.Format))
		{}

		template <CTextureSize OtherTextureSize>
		TTextureSize(OtherTextureSize&& other)
			: Width(static_cast<SizeType>(other.Width))
			, Height(static_cast<SizeType>(other.Height))
			, Format(ConvertFormat<InFormatType>(other.Format))
		{}
		
		SizeType Width = 0;
		SizeType Height = 0;
		InFormatType Format = GetUnknownFormat<InFormatType>();
		
		operator bool() const
		{
			return Width > 0 && Height > 0 && Format != GetUnknownFormat<InFormatType>();
		}

		template <CTextureSize OtherTextureSize>
		operator OtherTextureSize() const
		{
			return OtherTextureSize(
				Width, Height,
				ConvertFormat<typename OtherTextureSize::FormatType>(Format)
			);
		}
	};

	template <CTextureSize Left, CTextureSize Right>
	bool operator == (Left const& l, Right const& r)
	{
		return l.Width == r.Width
			&& l.Height == r.Height
			&& l.Format == ConvertFormat<typename Left::FormatType>(r.Format)
		;
	}
	
	template <CTextureSize Left, CTextureSize Right>
	bool operator != (Left const& l, Right const& r) { return !(l == r); }

	using FUnrealTextureSize = TTextureSize<uint32, EPixelFormat>;

	MCRO_API FUnrealTextureSize GetTextureSize(UTexture* texture);
}
