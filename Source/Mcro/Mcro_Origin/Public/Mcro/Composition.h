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
#include "Mcro/Any.h"
#include "Mcro/TextMacros.h"
#include "Mcro/AssertMacros.h"

/** @brief Namespace containing utilities and base classes for type composition */
namespace Mcro::Composition
{
	using namespace Mcro::Any;

	/**
	 *	@brief  
	 */
	class IHaveComponents
	{
		mutable TMap<uint64, TVariant<FAny, uint64>> Components;

		template <typename T>
		bool HasExactComponent() const
		{
			return Components.Contains(TTypeHash<T>) && Components[TTypeHash<T>].template IsType<FAny>();
		}

		template <typename T>
		bool HasComponentAliasUnchecked() const
		{
			return Components.Contains(TTypeHash<T>) && Components[TTypeHash<T>].template IsType<uint64>();
		}

		template <typename T>
		uint64 GetRealComponentHash() const
		{
			return HasExactComponent<T>()
				? TTypeHash<T>
				: HasComponentAliasUnchecked<T>()
				? Components[TTypeHash<T>].template Get<uint64>()
				: 0;
		}

		template <typename T>
		bool HasComponentAlias() const
		{
			if (HasComponentAliasUnchecked<T>())
			{
				uint64 realComponent = GetRealComponentHash<T>();
				if (Components.Contains(realComponent)) return true;
				Components.Remove(TTypeHash<T>);
			}
			return false;
		}

		template <typename T>
		const FAny* TryGetComponentPrivate() const
		{
			uint64 realComponent = GetRealComponentHash<T>();
			if (const auto* variant = Components.Find(realComponent))
				return &variant->Get<FAny>();
			return nullptr;
		}

		template <typename T>
		FAny* TryGetComponentPrivate()
		{
			uint64 realComponent = GetRealComponentHash<T>();
			if (auto* variant = Components.Find(realComponent))
				return &variant->Get<FAny>();
			return nullptr;
		}

		template <typename MainType, typename ValidAs>
		void AddComponentAlias()
		{
			if (!ensureMsgf(
				!HasExactComponent<ValidAs>(),
				TEXT_"%s won't be used with %s because a real component already exists under that type.",
				*TTypeString<MainType>(),
				*TTypeString<ValidAs>()
			)) return;
			
			if (!ensureMsgf(
				!HasComponentAlias<ValidAs>(),
				TEXT_"%s won't be used with %s because another component %s already uses it as an alias.",
				*TTypeString<MainType>(),
				*TTypeString<ValidAs>(),
				*TryGetComponentPrivate<ValidAs>()->GetType().ToStringCopy()
			)) return;

			Components.Add(TTypeHash<ValidAs>, {TInPlaceType<uint64>(), TTypeHash<MainType>});
		}

	public:
		template <typename MainType, typename... ValidAs>
		MainType& AddComponent(MainType* newComponent, TAnyTypeFacilities<MainType> const& facilities = {})
		{
			ASSERT_CRASH(newComponent);
			ASSERT_CRASH(!HasExactComponent<MainType>(), ->WithMessageF(
				TEXT_"{0} cannot be added because another component already exists under that type.",
				TTypeName<MainType>
			));
			ASSERT_CRASH(!HasComponentAlias<MainType>(), ->WithMessageF(
				TEXT_"{0} cannot be added because another component {1} already uses {0} as an alias.",
				TTypeName<MainType>,
				TryGetComponentPrivate<MainType>()->GetType()
			));
			
			Components.Add(TTypeHash<MainType>, {TInPlaceType<FAny>(), FAny(newComponent, facilities).ValidAs<ValidAs...>()});
			(AddComponentAlias<MainType, ValidAs>(), ...);
			return *newComponent;
		}
		
		template <CDefaultInitializable MainType, typename... ValidAs>
		MainType& AddComponent(TAnyTypeFacilities<MainType> const& facilities = {})
		{
			return AddComponent<MainType, ValidAs...>(new MainType(), facilities);
		}
		
		template <CDefaultInitializable MainType, typename... ValidAs>
		MainType& GetOrAddComponent(TAnyTypeFacilities<MainType> const& facilities = {})
		{
			return AddComponent<MainType, ValidAs...>(new MainType(), facilities);
		}

		template <typename T>
		const T* TryGetComponent() const
		{
			if (const FAny* component = TryGetComponentPrivate<T>())
				return component->TryGet<T>();
			return nullptr;
		}

		template <typename T>
		T* TryGetComponent()
		{
			if (FAny* component = TryGetComponentPrivate<T>())
				return component->TryGet<T>();
			return nullptr;
		}
		
		template <typename T>
		T const& GetComponent() const
		{
			const T* result = TryGetComponent<T>();
			ASSERT_CRASH(result, ->WithMessageF(TEXT_"Component {0} was unavailable.", TTypeName<T>));
			return *result;
		}
		
		template <typename T>
		T& GetComponent()
		{
			T* result = TryGetComponent<T>();
			ASSERT_CRASH(result, ->WithMessageF(TEXT_"Component {0} was unavailable.", TTypeName<T>));
			return *result;
		}
	};
}