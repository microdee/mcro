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

#include "Mcro/Any.h"

namespace Mcro::Any
{
	FAny::FAny(FAny const& other)
	{
		if (other.IsValid())
			other.CopyConstruct(this, other);
	}

	FAny::FAny(FAny&& other)
	{
		if (other.IsValid())
			other.MoveConstruct(this, Forward<FAny>(other));
	}

	FAny::~FAny()
	{
		if (static_cast<bool>(Destruct))
			Destruct(this);
	}

	void FAny::CopyTypeInfo(FAny* self, const FAny* other)
	{
		self->MainType = other->MainType;
		self->ValidTypes = other->ValidTypes;
		self->CopyConstruct = other->CopyConstruct;
		self->MoveConstruct = other->MoveConstruct;
		self->Destruct = other->Destruct;
	}
}
