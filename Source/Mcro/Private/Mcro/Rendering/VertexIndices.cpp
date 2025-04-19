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

#include "Mcro/Rendering/VertexIndices.h"
#if INTEL_ISPC
#include "VertexIndices.ispc.generated.h"
#endif

namespace Mcro::Rendering::VertexIndices
{
	void SquareGridMortonOrderIndexBuffer(TArray<uint16>& indices, uint32 size, FGridIndexBufferTopology const& topology)
	{
		indices.SetNumUninitialized(size * size * 6);
#if INTEL_ISPC
		ispc::SquareGridMortonOrderIndexBuffer_Uint16(indices.GetData(), size, topology.bForwardDiagonal);
#else
#error "Intel ISPC is required"
#endif
	}

	void SquareGridMortonOrderIndexBuffer(TArray<uint32>& indices, uint32 size, FGridIndexBufferTopology const& topology)
	{
		indices.SetNumUninitialized(size * size * 6);
#if INTEL_ISPC
		ispc::SquareGridMortonOrderIndexBuffer_Uint32(indices.GetData(), size, topology.bForwardDiagonal);
#else
#error "Intel ISPC is required"
#endif
	}

	void GridScanlineOrderIndexBuffer(TArray<uint16>& indices, uint32 width, uint32 height, FGridIndexBufferTopology const& topology)
	{
		indices.SetNumUninitialized(width * height * 6);
#if INTEL_ISPC
		ispc::GridScanlineOrderIndexBuffer_Uint16(indices.GetData(), width, height, topology.Majority == EGridMajor::Row, topology.bForwardDiagonal);
#else
#error "Intel ISPC is required"
#endif
	}

	void GridScanlineOrderIndexBuffer(TArray<uint32>& indices, uint32 width, uint32 height, FGridIndexBufferTopology const& topology)
	{
		indices.SetNumUninitialized(width * height * 6);
#if INTEL_ISPC
		ispc::GridScanlineOrderIndexBuffer_Uint32(indices.GetData(), width, height, topology.Majority == EGridMajor::Row, topology.bForwardDiagonal);
#else
#error "Intel ISPC is required"
#endif
	}

	void GridIndexBuffer(TArray<uint32>& indices, uint32 width, uint32 height, FGridIndexBufferTopology const& topology)
	{
		if (width == height) SquareGridMortonOrderIndexBuffer(indices, width, topology);
		else GridScanlineOrderIndexBuffer(indices, width, height, topology);
	}

	void GridIndexBuffer(TArray<uint16>& indices, uint32 width, uint32 height, FGridIndexBufferTopology const& topology)
	{
		if (width == height) SquareGridMortonOrderIndexBuffer(indices, width, topology);
		else GridScanlineOrderIndexBuffer(indices, width, height, topology);
	}
}
