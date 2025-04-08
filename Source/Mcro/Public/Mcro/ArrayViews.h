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

namespace Mcro::ArrayViews
{

	/**
	 * @brief   Makes a non-owning byte array view of the source typed array.
	 * @tparam  T          Source type
	 * @tparam  Allocator  *(optional)* Array allocator
	 * @param   array      Source array (going out of scope while working with the view will result in undefined behavior)
	 * @return  a non-owning byte array view of the source array
	 */
	template<typename T, typename Allocator = FDefaultAllocator>
	TArrayView<uint8> MakeByteArrayViewFromTyped(TArray<T, Allocator>& array)
	{
		return MakeArrayView(
			reinterpret_cast<uint8*>(array.GetData()),
			array.Num() * sizeof(T)
		);
	}

	/**
	 * @brief   Makes a non-owning typed array view of the source byte array.
	 * @tparam  T          Destination type
	 * @tparam  Allocator  *(optional)* Array allocator
	 * @param   array      Source array (going out of scope while working with the view will result in undefined behavior)
	 * @return  a non-owning typed array view of the source array
	 */
	template<typename T, typename Allocator = FDefaultAllocator>
	TArrayView<T> MakeTypedArrayViewFromBytes(TArray<uint8, Allocator>& array)
	{
		return MakeArrayView(
			reinterpret_cast<T*>(array.GetData()),
			array.Num() / sizeof(T)
		);
	}
}
