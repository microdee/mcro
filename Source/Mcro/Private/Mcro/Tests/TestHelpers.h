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

struct FCopyForbidden : FNoncopyable {};

struct FCopyConstructCounter
{
	FORCEINLINE FCopyConstructCounter() {}
	FORCEINLINE FCopyConstructCounter(FCopyConstructCounter const& other)
		: CopyCount(other.CopyCount + 1)
	{}
	FORCEINLINE FCopyConstructCounter(FCopyConstructCounter&& other) noexcept
		: MoveCount(other.MoveCount + 1)
	{}

	auto operator=(FCopyConstructCounter const& other) -> FCopyConstructCounter&;
	auto operator=(FCopyConstructCounter&& other) noexcept -> FCopyConstructCounter&;

	int32 CopyCount = 0;
	int32 MoveCount = 0;
	int32 CopyAssignCount = 0;
	int32 MoveAssignCount = 0;
};