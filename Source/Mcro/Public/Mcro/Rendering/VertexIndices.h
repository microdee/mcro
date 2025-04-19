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
#include "RHIResourceUtils.h"

#include "Mcro/Concepts.h"

namespace Mcro::Rendering::VertexIndices
{
	using namespace Mcro::Concepts;

	enum class EGridMajor { Row, Column };

	struct FGridIndexBufferTopology
	{
		bool bForwardDiagonal = false;
		EGridMajor Majority = EGridMajor::Row;
	};

	/**
	 *	@brief
	 *	Creates an index buffer for a square grid walking through cells in Z / Morton order. This gives better vertex
	 *	reuse, roughly 75% rate vs 66% of naive scanline approach.
	 *	
	 *	This was stolen from
	 *	`Experimental/VirtualHeightfieldMesh/Source/VirtualHeightfieldMesh/Private/VirtualHeightfieldMeshVertexFactory.cpp`
	 *	and ported to ISPC
	 *	
	 *	@tparam        T  Numeric type of output indices
	 *	@param   indices  Output array
	 *	@param      size
	 *	@param  topology  Provides row/column majority and the direction of the diagonal edge in the cells
	 */
	MCRO_API void SquareGridMortonOrderIndexBuffer(TArray<uint16>& indices, uint32 size, FGridIndexBufferTopology const& topology);

	/** @copydoc SquareGridMortonOrderIndexBuffer */
	MCRO_API void SquareGridMortonOrderIndexBuffer(TArray<uint32>& indices, uint32 size, FGridIndexBufferTopology const& topology);

	/**
	 *	@brief
	 *	Creates an index buffer for arbitrary size grid going row-by-row by default.
	 *	
	 *	This was stolen from
	 *	`Experimental/VirtualHeightfieldMesh/Source/VirtualHeightfieldMesh/Private/VirtualHeightfieldMeshVertexFactory.cpp`
	 *	
	 *	@tparam        T  Numeric type of output indices
	 *	@param   indices  Output array
	 *	@param     width
	 *	@param    height
	 *	@param  topology  Provides row/column majority and the direction of the diagonal edge in the cells
	 *	@return 
	 */
	MCRO_API void GridScanlineOrderIndexBuffer(TArray<uint16>& indices, uint32 width, uint32 height, FGridIndexBufferTopology const& topology);
	
	/** @copydoc GridScanlineOrderIndexBuffer */
	MCRO_API void GridScanlineOrderIndexBuffer(TArray<uint32>& indices, uint32 width, uint32 height, FGridIndexBufferTopology const& topology);

	MCRO_API void GridIndexBuffer(TArray<uint32>& indices, uint32 width, uint32 height, FGridIndexBufferTopology const& topology);
	MCRO_API void GridIndexBuffer(TArray<uint16>& indices, uint32 width, uint32 height, FGridIndexBufferTopology const& topology);

	/**
	 *	@brief
	 *	Creates a static index buffer from input indices 
	 *	
	 *	@tparam            T  Numeric type of output indices
	 *	@param       indices  Input array
	 *	@param    rhiCmdList
	 *	@param  resourceName  A name to identify the resource while profiling
	 */
	template <CIntegral T>
	FBufferRHIRef CreateStaticIndexBufferRHI(TArray<T> const& indices, FRHICommandListBase& rhiCmdList, const TCHAR* resourceName)
	{
		return UE::RHIResourceUtils::CreateIndexBufferFromArray(
			rhiCmdList,
			resourceName,
			EBufferUsageFlags::Static,
			MakeConstArrayView(indices)
		);
	}
}