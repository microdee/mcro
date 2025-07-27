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
	IComposable::IComposable(const IComposable& other)
		: LastAddedComponentHash(other.LastAddedComponentHash)
		, Components(other.Components)
		, ComponentLogistics(other.ComponentLogistics)
		, ComponentAliases(other.ComponentAliases)
		, OnComponentAdded(other.OnComponentAdded)
	{
		NotifyCopyComponents(other);
	}

	IComposable::IComposable(IComposable&& other) noexcept
		: LastAddedComponentHash(other.LastAddedComponentHash)
		, Components(MoveTemp(other.Components))
		, ComponentLogistics(MoveTemp(other.ComponentLogistics))
		, ComponentAliases(MoveTemp(other.ComponentAliases))
		, OnComponentAdded(MoveTemp(other.OnComponentAdded))
	{
		NotifyMoveComponents(FWD(other));
	}

	bool IComposable::HasExactComponent(FTypeHash typeHash) const
	{
		return Components.Contains(typeHash);
	}

	bool IComposable::HasComponentAliasUnchecked(FTypeHash typeHash) const
	{
		return ComponentAliases.Contains(typeHash);
	}

	bool IComposable::HasComponentAlias(FTypeHash typeHash) const
	{
		if (HasComponentAliasUnchecked(typeHash))
		{
			auto& components = ComponentAliases[typeHash];
			components.RemoveAll([this](FTypeHash i)
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

	void IComposable::AddComponentAlias(FTypeHash mainType, FTypeHash validAs)
	{
		if (HasComponentAliasUnchecked(validAs))
			ComponentAliases[validAs].Add(mainType);
		else ComponentAliases.Add(validAs, { mainType });
	}

	void IComposable::NotifyCopyComponents(IComposable const& other)
	{
		for (auto const& [key, logistics] : ComponentLogistics)
		{
			logistics.Copy(this, other.Components[key]);
		}
	}

	void IComposable::NotifyMoveComponents(IComposable&& other)
	{
		if (this == &other) return;
		for (auto const& [key, logistics] : ComponentLogistics)
		{
			logistics.Move(this);
		}
		other.ResetComponents();
	}

	void IComposable::ResetComponents()
	{
		Components.Empty();
		ComponentLogistics.Empty();
		ComponentAliases.Empty();
		LastAddedComponentHash = 0;
	}

	ranges::any_view<FAny*> IComposable::GetExactComponent(FTypeHash typeHash) const
	{
		namespace r = ranges;
		namespace rv = ranges::views;
			
		if (HasExactComponent(typeHash)) return rv::single(Components.Find(typeHash));
		return ranges::empty_view<FAny*>();
	}

	ranges::any_view<FAny*> IComposable::GetAliasedComponents(FTypeHash typeHash) const
	{
		namespace r = ranges;
		namespace rv = ranges::views;
			
		if (HasComponentAlias(typeHash))
			return ComponentAliases[typeHash]
				| rv::transform([this](FTypeHash i) -> decltype(auto) { return Components.Find(i); });
			
		return r::empty_view<FAny*>();
	}

	ranges::any_view<FAny*> IComposable::GetComponentsDynamic(FTypeHash typeHash) const
	{
		return GetExactComponent(typeHash) | Concat(GetAliasedComponents(typeHash));
	}
}
