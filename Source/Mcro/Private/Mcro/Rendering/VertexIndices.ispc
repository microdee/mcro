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

inline uint32 MortonCode2(uint32 x)
{
    x &= 0x0000ffff;
    x = (x ^ (x << 8)) & 0x00ff00ff;
    x = (x ^ (x << 4)) & 0x0f0f0f0f;
    x = (x ^ (x << 2)) & 0x33333333;
    x = (x ^ (x << 1)) & 0x55555555;
    return x;
}

inline uint64 MortonCode2_64(uint64 x)
{
    x &= 0x00000000ffffffff;
    x = (x ^ (x << 16)) & 0x0000ffff0000ffff;
    x = (x ^ (x << 8)) & 0x00ff00ff00ff00ff;
    x = (x ^ (x << 4)) & 0x0f0f0f0f0f0f0f0f;
    x = (x ^ (x << 2)) & 0x3333333333333333;
    x = (x ^ (x << 1)) & 0x5555555555555555;
    return x;
}

inline uint32 ReverseMortonCode2(uint32 x)
{
    x &= 0x55555555;
    x = (x ^ (x >> 1)) & 0x33333333;
    x = (x ^ (x >> 2)) & 0x0f0f0f0f;
    x = (x ^ (x >> 4)) & 0x00ff00ff;
    x = (x ^ (x >> 8)) & 0x0000ffff;
    return x;
}

inline uint64 ReverseMortonCode2_64(uint64 x)
{
    x &= 0x5555555555555555;
    x = (x ^ (x >> 1)) & 0x3333333333333333;
    x = (x ^ (x >> 2)) & 0x0f0f0f0f0f0f0f0f;
    x = (x ^ (x >> 4)) & 0x00ff00ff00ff00ff;
    x = (x ^ (x >> 8)) & 0x0000ffff0000ffff;
    x = (x ^ (x >> 16)) & 0x00000000ffffffff;
    return x;
}

inline uint32 MortonCode3(uint32 x)
{
    x &= 0x000003ff;
    x = (x ^ (x << 16)) & 0xff0000ff;
    x = (x ^ (x <<  8)) & 0x0300f00f;
    x = (x ^ (x <<  4)) & 0x030c30c3;
    x = (x ^ (x <<  2)) & 0x09249249;
    return x;
}

inline uint32 ReverseMortonCode3(uint32 x)
{
    x &= 0x09249249;
    x = (x ^ (x >>  2)) & 0x030c30c3;
    x = (x ^ (x >>  4)) & 0x0300f00f;
    x = (x ^ (x >>  8)) & 0xff0000ff;
    x = (x ^ (x >> 16)) & 0x000003ff;
    return x;
}
 
template <typename T>
void SquareGridMortonOrderIndexBuffer(
    uniform T[] indices,
    uniform uint32 size,
    uniform bool forwardDiagonal
) {
    foreach (morton = 0 ... (size * size))
    {
        varying int i = morton * 6;
        varying uint32 squareX = ReverseMortonCode2(morton);
        varying uint32 squareY = ReverseMortonCode2(morton >> 1);

        varying uint32 index0 = squareX + squareY * (size + 1);
        varying uint32 index1 = index0 + 1;
        varying uint32 index2 = index0 + (size + 1);
        varying uint32 index3 = index2 + 1;

        cif (forwardDiagonal)
        {
            indices[i + 0] = index3;
            indices[i + 1] = index1;
            indices[i + 2] = index2;
            indices[i + 3] = index0;
            indices[i + 4] = index2;
            indices[i + 5] = index1;
        }
        else
        {
            indices[i + 0] = index3;
            indices[i + 1] = index1;
            indices[i + 2] = index0;
            indices[i + 3] = index0;
            indices[i + 4] = index2;
            indices[i + 5] = index3;
        }
    }
}

template <typename T>
void GridScanlineOrderIndexBuffer(
    uniform T[] indices,
    uniform uint32 width,
    uniform uint32 height,
    uniform bool rowMajor,
    uniform bool forwardDiagonal
) {
    uniform uint32 minor = rowMajor ? height : width;
    uniform uint32 major = rowMajor ? width : height;
    foreach (mi = 0 ... minor, ma = 0 ... major)
    {
        varying uint32 i = (major * mi + ma) * 6;
        cif (forwardDiagonal)
        {
            indices[i + 0] = major * (mi + 1) + ma + 0;
            indices[i + 1] = major * (mi + 0) + ma + 0;
            indices[i + 2] = major * (mi + 0) + ma + 1;
            
            indices[i + 3] = major * (mi + 1) + ma + 0;
            indices[i + 4] = major * (mi + 0) + ma + 1;
            indices[i + 5] = major * (mi + 1) + ma + 1;
        }
        else
        {
            indices[i + 0] = major * (mi + 0) + ma + 0;
            indices[i + 1] = major * (mi + 0) + ma + 1;
            indices[i + 2] = major * (mi + 1) + ma + 1;
            
            indices[i + 3] = major * (mi + 0) + ma + 0;
            indices[i + 4] = major * (mi + 1) + ma + 1;
            indices[i + 5] = major * (mi + 1) + ma + 0;
        }
    }
}

export void SquareGridMortonOrderIndexBuffer_Uint32(
    uniform uint32[] indices,
    uniform uint32 size,
    uniform bool forwardDiagonal
) {
    SquareGridMortonOrderIndexBuffer<uint32>(indices, size, forwardDiagonal);
}

export void SquareGridMortonOrderIndexBuffer_Uint16(
    uniform uint16[] indices,
    uniform uint32 size,
    uniform bool forwardDiagonal
) {
    SquareGridMortonOrderIndexBuffer<uint16>(indices, size, forwardDiagonal);
}

export void GridScanlineOrderIndexBuffer_Uint32(
    uniform uint32[] indices,
    uniform uint32 width,
    uniform uint32 height,
    uniform bool rowMajor,
    uniform bool forwardDiagonal
) {
    GridScanlineOrderIndexBuffer<uint32>(indices, width, height, rowMajor, forwardDiagonal);
}

export void GridScanlineOrderIndexBuffer_Uint16(
    uniform uint16[] indices,
    uniform uint32 width,
    uniform uint32 height,
    uniform bool rowMajor,
    uniform bool forwardDiagonal
) {
    GridScanlineOrderIndexBuffer<uint16>(indices, width, height, rowMajor, forwardDiagonal);
}