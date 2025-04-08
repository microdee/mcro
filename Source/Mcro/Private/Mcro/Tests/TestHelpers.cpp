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

#include "TestHelpers.h"

auto FCopyConstructCounter::operator=(FCopyConstructCounter const& other) -> FCopyConstructCounter&
{
	if (this == &other) return *this;

	CopyAssignCount = other.CopyAssignCount + 1;
	return *this;
}

auto FCopyConstructCounter::operator=(FCopyConstructCounter&& other) noexcept -> FCopyConstructCounter&
{
	if (this == &other) return *this;
		
	MoveAssignCount = other.MoveAssignCount + 1;
	return *this;
}
