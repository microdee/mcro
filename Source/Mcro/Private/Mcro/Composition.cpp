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

#include "Mcro/Composition.h"

namespace Mcro::Composition
{
	bool IComposable::HasExactComponent(uint64 typeHash) const
	{
		return Components.Contains(typeHash);
	}

	bool IComposable::HasComponentAliasUnchecked(uint64 typeHash) const
	{
		return ComponentAliases.Contains(typeHash);
	}

	bool IComposable::HasComponentAlias(uint64 typeHash) const
	{
		if (HasComponentAliasUnchecked(typeHash))
		{
			TArray<uint64>& components = ComponentAliases[typeHash];
			components.RemoveAll([this](uint64 i)
			{
				return !Components.Contains(i);
			});
			if (components.IsEmpty())
			{
				ComponentAliases.Remove(typeHash);
				return false;
			}
			return true;
		}
		return false;
	}

	void IComposable::AddComponentAlias(uint64 mainType, uint64 validAs)
	{
		if (HasComponentAliasUnchecked(validAs))
			ComponentAliases[validAs].Add(mainType);
		else ComponentAliases.Add(validAs, { mainType });
	}

	ranges::any_view<FAny*> IComposable::GetExactComponent(uint64 typeHash) const
	{
		namespace r = ranges;
		namespace rv = ranges::views;
			
		if (HasExactComponent(typeHash)) return rv::single(Components.Find(typeHash));
		return ranges::empty_view<FAny*>();
	}

	ranges::any_view<FAny*> IComposable::GetAliasedComponents(uint64 typeHash) const
	{
		namespace r = ranges;
		namespace rv = ranges::views;

		auto asd = IComposable().WithComponent<FVector>();
			
		if (HasComponentAlias(typeHash))
			return ComponentAliases[typeHash]
				| rv::transform([this](uint64 i) -> decltype(auto) { return Components.Find(i); });
			
		return r::empty_view<FAny*>();
	}

	ranges::any_view<FAny*> IComposable::GetComponentsPrivate(uint64 typeHash) const
	{
		return GetExactComponent(typeHash) | Concat(GetAliasedComponents(typeHash));
	}
}
