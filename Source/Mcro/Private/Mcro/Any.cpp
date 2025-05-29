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
		: Storage(other.Storage)
		, MainType(MoveTemp(other.MainType))
		, Destruct(MoveTemp(other.Destruct))
		, CopyConstruct(MoveTemp(other.CopyConstruct))
		, ValidTypes(MoveTemp(other.ValidTypes))
	{
		other.Reset();
	}

	FAny::~FAny()
	{
		if (static_cast<bool>(Destruct))
			Destruct(this);
		Reset();
	}
	
	void FAny::AddAlias(FType const& alias)
	{
		if (!ValidTypes.Contains(alias))
			ValidTypes.Add(alias);
	}

	void FAny::CopyTypeInfo(FAny* self, const FAny* other)
	{
		self->MainType = other->MainType;
		self->ValidTypes = other->ValidTypes;
		self->CopyConstruct = other->CopyConstruct;
		self->Destruct = other->Destruct;
	}

	void FAny::Reset()
	{
		Storage = nullptr;
		MainType = {};
		Destruct.Reset();
		CopyConstruct.Reset();
		ValidTypes.Empty();
	}
}
