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
#include "Mcro/Range.h"
#include "Mcro/Range/Views.h"
#include "Mcro/Range/Conversion.h"

/** @brief Namespace containing utilities and base classes for type composition */
namespace Mcro::Composition
{
	using namespace Mcro::Any;
	using namespace Mcro::Range;

	/**
	 *	@brief
	 *	This template is used in `IComponent::With(TAlias<...>)` so it can have deduced this type and explicit
	 *	variadic template arguments when specifying multiple aliases.
	 */
	template <typename...>
	struct TAlias {};

	class IComposable;

	/**
	 *	@brief
	 *	Inherit from this empty interface to signal that the inheriting class knows that it's a component and that it
	 *	can receive info about the composable class it is being registered to.
	 *
	 *	Define `OnComponentRegistered(T& to)` in the inheriting class, where T is the expected type of the
	 *	composable parent. In case this component is registered to a composable class which is not convertible to T
	 *	then `OnComponentRegistered` will be silently ignored.
	 */
	struct IComponent
	{
		// void OnComponentRegistered(T& to) {}
	};

	/**
	 *	@brief
	 *	Inherit from this empty interface to signal that the inheriting class knows that it's a component and that it
	 *	can receive info about the composable class it is being registered to.
	 *
	 *	Define `OnComponentRegistered(T& to)` in the inheriting class, where T is the expected type of the
	 *	composable parent. In case of `IStrictComponent`, it is a compile error to register this class to a composable
	 *	class which is not convertible to T.
	 */
	struct IStrictComponent : IComponent
	{
		// void OnComponentRegistered(T& to) {}
	};

	template <typename T>
	concept CComposable = CDerivedFrom<T, IComposable>;

	template <typename T>
	concept CExplicitComponent = CDerivedFrom<T, IComponent>;

	template <typename T>
	concept CStrictComponent = CDerivedFrom<T, IStrictComponent>;
	
	template <typename T, typename Composition>
	concept CCompatibleExplicitComponent = CExplicitComponent<T>
		&& requires(std::decay_t<T>& t, Composition&& parent)
		{
			t.OnComponentRegistered(parent);
		}
	;

	template <typename T, typename Composition>
	concept CCompatibleStrictComponent = CStrictComponent<T> && CCompatibleExplicitComponent<T, Composition>;

	template <typename T, typename Composition>
	concept CCompatibleComponent = !CStrictComponent<T> || CCompatibleStrictComponent<T, Composition>;

	/**
	 *	@brief  A base class which can bring type based class-composition to a derived class
	 *
	 *	This exists because indeed Unreal has its own composition model (actors / actor-components) or it has the
	 *	subsystem architecture for non-actors, they still require to be used with UObjects. `IComposable` allows
	 *	any C++ objects to have type-safe runtime managed optional components which can be configured separately for
	 *	each instance.
	 *
	 *	The only requirement for components is that they need to be copy and move constructible (as is the default with
	 *	plain-old-C++ objects, if they don't have members where either constructors are deleted or inaccessible). This
	 *	limitation is imposed by `FAny` only for easier interfacing, but the API for managing components will never move
	 *	or copy them by itself.
	 *	The composable class doesn't have that limitation.
	 *
	 *	Usage:
	 *	@code
	 *	struct IBaseComponent { int A; };
	 *	
	 *	struct FComponentImplementation : IBaseComponent
	 *	{
	 *		FComponentImplementation(int a, int b) : A(a), B(b) {}
	 *		int B;
	 *	};
	 *	struct FSimpleComponent { int C; };
	 *
	 *	class FMyComposableType : public IComposable {};
	 *
	 *	FMyComposableType MyStuff()
	 *		.WithComponent(new FComponentImplementation(1, 2)).WithAlias<IBaseComponent>()
	 *		.WithComponent<FSimpleComponent>()
	 *	;
	 *	@endcode
	 *
	 *	As mentioned earlier, components are not required to have any arbitrary type traits, but if they inherit from
	 *	`IComponent` or `IStrictComponent` they can receive extra information when they're registered for a composable
	 *	class. The difference between the two is that `IComponent` doesn't mind if it's attached to a composable class
	 *	it doesn't know about, however it is a compile error if an `IStrictComponent` is attempted to be attached to
	 *	an incompatible class.
	 *
	 *	For example
	 *	@code
	 *	struct FChillComponent : IComponent
	 *	{
	 *		void OnComponentRegistered(FExpectedParent& to) {}
	 *	};
	 *	
	 *	struct FStrictComponent : IStrictComponent
	 *	{
	 *		void OnComponentRegistered(FExpectedParent& to) {}
	 *	};
	 *
	 *	class FExpectedParent : public IComposable {};
	 *	class FSomeOtherParent : public IComposable {};
	 *	
	 *	auto MyOtherStuff = FExpectedParent()
	 *		.WithComponent<FChillComponent>()  // OK, and OnComponentRegistered is called
	 *		.WithComponent<FStrictComponent>() // OK, and OnComponentRegistered is called
	 *
	 *	auto MyStuff = FSomeOtherParent()
	 *		.WithComponent<FChillComponent>()  // OK, but OnComponentRegistered won't be called.
	 *		
	 *		.WithComponent<FStrictComponent>() // COMPILE ERROR, CCompatibleComponent concept is not satisfied
	 *		                                   // because FSomeOtherParent is not convertible to FExpectedParent
	 *		                                   // at OnComponentRegistered(FExpectedParent& to)
	 *	;
	 *	@endcode
	 *	
	 *	@todo
	 *	C++ 26 has promising proposal for static value-based reflection, which can gather metadata from classes
	 *	or even emit them. The best summary I found so far is a stack-overflow answer https://stackoverflow.com/a/77477029
	 *	Once that's available we can gather base classes in compile time, and do dynamic casting of objects without
	 *	the need for intrusive extra syntax, or extra work at construction.
	 *	Currently GCC's `__bases` would be perfect for the job, but other popular compilers don't have similar
	 *	intrinsics. Once such a feature becomes widely available base classes can be automatically added as aliases for
	 *	registered components
	 */
	class MCRO_API IComposable
	{
		uint64 LastAddedComponentHash = 0;
		mutable TMap<uint64, FAny> Components;
		mutable TMap<uint64, TArray<uint64>> ComponentAliases;

		bool HasExactComponent(uint64 typeHash) const;
		bool HasComponentAliasUnchecked(uint64 typeHash) const;
		bool HasComponentAlias(uint64 typeHash) const;
		void AddComponentAlias(uint64 mainType, uint64 validAs);
		
		template <typename ValidAs>
		void AddComponentAlias(uint64 mainType)
		{
			Components[mainType].ValidAs<ValidAs>();
			AddComponentAlias(mainType, TTypeHash<ValidAs>);
		}
		
		ranges::any_view<FAny*> GetExactComponent(uint64 typeHash) const;
		ranges::any_view<FAny*> GetAliasedComponents(uint64 typeHash) const;
		ranges::any_view<FAny*> GetComponentsPrivate(uint64 typeHash) const;
		
	public:

		/**
		 *	@brief
		 *	Add a component to this composable class.
		 *
		 *	@tparam MainType  The exact component type (deduced from `newComponent`
		 *	@tparam Self      Deducing this
		 *	@param  self      Deducing this
		 *	
		 *	@param newComponent
		 *	A pointer to the new component being added. `IComposable` will assume ownership of the new component
		 *	adhering to RAII. Make sure the lifespan of the provided object is not managed by something else or the
		 *	stack, in fact better to stick with the `new` operator.
		 *
		 *	@param facilities
		 *	Customization point for object copy/move and delete methods. See `TAnyTypeFacilities`
		 */
		template <typename MainType, typename Self>
		requires CCompatibleComponent<MainType, Self>
		void AddComponent(this Self&& self, MainType* newComponent, TAnyTypeFacilities<MainType> const& facilities = {})
		{
			ASSERT_CRASH(newComponent);
			ASSERT_CRASH(!self.HasExactComponent(TTypeHash<MainType>),
				->WithMessageF(
					TEXT_"{0} cannot be added because another component already exists under that type.",
					TTypeName<MainType>
				)
				->WithDetailsF(TEXT_
					"Try wrapping your component in an empty derived type, and register it with its base type {0} as its"
					" alias. Later on both the current and the already existing component can be accessed via"
					" `GetComponents<{0}>()` which returns a range of all matching components.",
					TTypeName<MainType>
				)
			);
			
			self.Components.Add(TTypeHash<MainType>, FAny(newComponent, facilities));
			self.LastAddedComponentHash = TTypeHash<MainType>;

			if constexpr (CCompatibleExplicitComponent<MainType, Self>)
				newComponent->OnComponentRegistered(self);
		}
		
		/**
		 *	@brief
		 *	Add a default constructed component to this composable class. 
		 *
		 *	@tparam MainType  The exact component type
		 *	@tparam Self      Deducing this
		 *	@param  self      Deducing this
		 *
		 *	@param facilities
		 *	Customization point for object copy/move and delete methods. See `TAnyTypeFacilities`
		 */
		template <CDefaultInitializable MainType, typename Self>
		requires CCompatibleComponent<MainType, Self>
		void AddComponent(this Self&& self, TAnyTypeFacilities<MainType> const& facilities = {})
		{
			self.template AddComponent<MainType, Self>(new MainType(), facilities);
		}

		/**
		 *	@brief
		 *	Add a list of types the last added component is convertible to and may be used to get the last component
		 *	among others which may list the same aliases.
		 *
		 *	@warning
		 *	Calling this function before adding a component may result in a runtime crash!
		 *	
		 *	@tparam ValidAs
		 *	The list of other types the last added component is convertible to and may be used to get the last component
		 *	among others which may list the same aliases.
		 */
		template <typename... ValidAs>
		void AddAlias()
		{
			ASSERT_CRASH(LastAddedComponentHash != 0 && Components.Contains(LastAddedComponentHash),
				->WithMessage(TEXT_"Component aliases were listed, but no components were added before.")
				->WithDetails(TEXT_"Make sure `AddAlias` or `WithAlias` is called after `AddComponent` / `WithComponent`.")
			);
			(AddComponentAlias<ValidAs>(LastAddedComponentHash), ...);
		}

		/**
		 *	@brief
		 *	Add a component to this composable class with a fluent API.
		 *
		 *	This overload is available for composable classes which also inherit from `TSharedFromThis`.
		 *
		 *	@tparam MainType  The exact component type (deduced from `newComponent`
		 *	@tparam Self      Deducing this
		 *	@param  self      Deducing this
		 *	
		 *	@param newComponent
		 *	A pointer to the new component being added. `IComposable` will assume ownership of the new component
		 *	adhering to RAII. Make sure the lifespan of the provided object is not managed by something else or the
		 *	stack, in fact better to stick with the `new` operator.
		 *
		 *	@param facilities
		 *	Customization point for object copy/move and delete methods. See `TAnyTypeFacilities`
		 *
		 *	@return
		 *	If the composable class also inherits from `TSharedFromThis` return a shared ref.
		 */
		template <typename MainType, CSharedFromThis Self>
		requires CCompatibleComponent<MainType, Self>
		auto WithComponent(this Self&& self, MainType* newComponent, TAnyTypeFacilities<MainType> const& facilities = {})
		{
			self.template AddComponent<MainType, Self>(newComponent, facilities);
			return self.SharedThis(&self);
		}
		
		/**
		 *	@brief
		 *	Add a default constructed component to this composable class with a fluent API. 
		 *
		 *	This overload is available for composable classes which also inherit from `TSharedFromThis`.
		 *
		 *	@tparam MainType  The exact component type
		 *	@tparam Self      Deducing this
		 *	@param  self      Deducing this
		 *
		 *	@param facilities
		 *	Customization point for object copy/move and delete methods. See `TAnyTypeFacilities`
		 *
		 *	@return
		 *	If the composable class also inherits from `TSharedFromThis` return a shared ref.
		 */
		template <CDefaultInitializable MainType, CSharedFromThis Self>
		requires CCompatibleComponent<MainType, Self>
		auto WithComponent(this Self&& self, TAnyTypeFacilities<MainType> const& facilities = {})
		{
			self.template AddComponent<MainType, Self>(facilities);
			return self.SharedThis(&self);
		}

		/**
		 *	@brief
		 *	Add a component to this composable class with a fluent API.
		 *
		 *	This overload is available for composable classes which are not explicitly meant to be used with shared pointers.
		 *
		 *	@tparam MainType  The exact component type (deduced from `newComponent`
		 *	@tparam Self      Deducing this
		 *	@param  self      Deducing this
		 *	
		 *	@param newComponent
		 *	A pointer to the new component being added. `IComposable` will assume ownership of the new component
		 *	adhering to RAII. Make sure the lifespan of the provided object is not managed by something else or the
		 *	stack, in fact better to stick with the `new` operator.
		 *
		 *	@param facilities
		 *	Customization point for object copy/move and delete methods. See `TAnyTypeFacilities`
		 *
		 *	@return
		 *	Perfect-forwarded self.
		 */
		template <typename MainType, typename Self>
		requires (CCompatibleComponent<MainType, Self> && !CSharedFromThis<Self>)
		decltype(auto) WithComponent(this Self&& self, MainType* newComponent, TAnyTypeFacilities<MainType> const& facilities = {})
		{
			self.template AddComponent<MainType, Self>(newComponent, facilities);
			return Forward<Self>(self);
		}

		/**
		 *	@brief
		 *	Add a default constructed component to this composable class with a fluent API. 
		 *
		 *	This overload is available for composable classes which are not explicitly meant to be used with shared pointers.
		 *
		 *	@tparam MainType  The exact component type
		 *	@tparam Self      Deducing this
		 *	@param  self      Deducing this
		 *
		 *	@param facilities
		 *	Customization point for object copy/move and delete methods. See `TAnyTypeFacilities`
		 *
		 *	@return
		 *	Perfect-forwarded self.
		 */
		template <CDefaultInitializable MainType, typename Self>
		requires (CCompatibleComponent<MainType, Self> && !CSharedFromThis<Self>)
		decltype(auto) WithComponent(this Self&& self, TAnyTypeFacilities<MainType> const& facilities = {})
		{
			self.template AddComponent<MainType, Self>(facilities);
			return Forward<Self>(self);
		}

		/**
		 *	@brief
		 *	Add a type, the last added component is convertible to and may be used to get the last component among
		 *	others which may list the same aliases.
		 *
		 *	Only one alias may be specified this way because of templating syntax intricacies, but it may be called
		 *	multiple times in a sequence to add multiple aliases for the same component.
		 *
		 *	Usage:
		 *	@code
		 *	auto result = IComposable()
		 *		.WithComponent<FMyComponent>()
		 *			.WithAlias<FMyComponentBase>()
		 *			.WithAlias<IMyComponentInterface>()
		 *	;
		 *	@endcode
		 *
		 *	For declaring multiple aliases in one go, use `With(TAlias<...>)` member template method.
		 *	
		 *	This overload is available for composable classes which also inherit from `TSharedFromThis`.
		 *
		 *	@warning
		 *	Calling this function before adding a component may result in a runtime crash!
		 *	
		 *	@tparam ValidAs
		 *	A type, the last added component is convertible to and may be used to get the last component among others
		 *	which may list the same aliases.
		 *	
		 *	@tparam Self  Deducing this
		 *	@param  self  Deducing this
		 *
		 *	@return
		 *	If the composable class also inherits from `TSharedFromThis` return a shared ref.
		 */
		template <typename ValidAs, CSharedFromThis Self>
		auto WithAlias(this Self&& self)
		{
			self.template AddAlias<ValidAs>();
			return self.SharedThis(&self);
		}

		/**
		 *	@brief
		 *	Add a type, the last added component is convertible to and may be used to get the last component among
		 *	others which may list the same aliases.
		 *
		 *	Only one alias may be specified this way because of templating syntax intricacies, but it may be called
		 *	multiple times in a sequence to add multiple aliases for the same component.
		 *
		 *	Usage:
		 *	@code
		 *	auto result = IComposable()
		 *		.WithComponent<FMyComponent>()
		 *			.WithAlias<FMyComponentBase>()
		 *			.WithAlias<IMyComponentInterface>()
		 *	;
		 *	@endcode
		 *
		 *	For declaring multiple aliases in one go, use `With(TAlias<...>)` member template method.
		 *	
		 *	This overload is available for composable classes which are not explicitly meant to be used with shared pointers.
		 *
		 *	@warning
		 *	Calling this function before adding a component may result in a runtime crash!
		 *	
		 *	@tparam ValidAs
		 *	A type, the last added component is convertible to and may be used to get the last component among others
		 *	which may list the same aliases.
		 *	
		 *	@tparam Self  Deducing this
		 *	@param  self  Deducing this
		 *
		 *	@return
		 *	Perfect-forwarded self.
		 */
		template <typename ValidAs, typename Self>
		requires (!CSharedFromThis<Self>)
		decltype(auto) WithAlias(this Self&& self)
		{
			self.template AddAlias<ValidAs>();
			return Forward<Self>(self);
		}

		/**
		 *	@brief
		 *	Add a list of types the last added component is convertible to and may be used to get the last component
		 *	among others which may list the same aliases.
		 *
		 *	Usage:
		 *	@code
		 *	auto result = IComposable()
		 *		.WithComponent<FMyComponent>().With(TAlias<
		 *			FMyComponentBase,
		 *			IMyComponentInterface
		 *		>)
		 *	;
		 *	@endcode
		 *	
		 *	This overload is available for composable classes which also inherit from `TSharedFromThis`.
		 *
		 *	@warning
		 *	Calling this function before adding a component may result in a runtime crash!
		 *	
		 *	@tparam ValidAs
		 *	The list of other types the last added component is convertible to and may be used to get the last component
		 *	among others which may list the same aliases.
		 *	
		 *	@tparam Self  Deducing this
		 *	@param  self  Deducing this
		 *
		 *	@return
		 *	If the composable class also inherits from `TSharedFromThis` return a shared ref.
		 */
		template <CSharedFromThis Self, typename... ValidAs>
		auto With(this Self&& self, TAlias<ValidAs...>&&)
		{
			self.template AddAlias<ValidAs...>();
			return self.SharedThis(&self);
		}

		/**
		 *	@brief
		 *	Add a list of types the last added component is convertible to and may be used to get the last component
		 *	among others which may list the same aliases.
		 *
		 *	Usage:
		 *	@code
		 *	auto result = IComposable()
		 *		.WithComponent<FMyComponent>().With(TAlias<
		 *			FMyComponentBase,
		 *			IMyComponentInterface
		 *		>)
		 *	;
		 *	@endcode
		 *	
		 *	This overload is available for composable classes which are not explicitly meant to be used with shared pointers.
		 *
		 *	@warning
		 *	Calling this function before adding a component may result in a runtime crash!
		 *	
		 *	@tparam ValidAs
		 *	The list of other types the last added component is convertible to and may be used to get the last component
		 *	among others which may list the same aliases.
		 *	
		 *	@tparam Self  Deducing this
		 *	@param  self  Deducing this
		 *
		 *	@return
		 *	Perfect-forwarded self.
		 */
		template <typename Self, typename... ValidAs>
		requires (!CSharedFromThis<Self>)
		decltype(auto) With(this Self&& self, TAlias<ValidAs...>&&)
		{
			self.template AddAlias<ValidAs...>();
			return Forward<Self>(self);
		}

		/**
		 *	@brief
		 *	Get all components added matching~ or aliased by the given type.
		 *	
		 *	@tparam T  Desired component type.
		 *	
		 *	@return
		 *	A range-view containing all the matched components. Components are provided as pointers to ensure they're
		 *	not copied even under intricate object plumbing situations, but invalid pointers are never returned.
		 *	(as long as the composable class is alive of course)
		 */
		template <typename T>
		ranges::any_view<T*> GetComponents() const
		{
			namespace rv = ranges::views;
			return GetComponentsPrivate(TTypeHash<T>)
				| rv::transform([](FAny* component) { return component->TryGet<T>(); })
				| FilterValid();
		}

		/**
		 *	@brief
		 *	Get the first component matching~ or aliased by the given type.
		 *
		 *	The order of components are non-deterministic so this method only make sense when it is trivial that only
		 *	one component will be available for that particular type.
		 *	
		 *	@tparam T  Desired component type.
		 *	
		 *	@return
		 *	A pointer to the component if one at least exists, nullptr otherwise.
		 */
		template <typename T>
		const T* TryGetComponent() const
		{
			return GetComponents<T>() | FirstOrDefault();
		}

		/**
		 *	@brief
		 *	Get the first component matching~ or aliased by the given type.
		 *
		 *	The order of components are non-deterministic so this method only make sense when it is trivial that only
		 *	one component will be available for that particular type.
		 *	
		 *	@tparam T  Desired component type.
		 *	
		 *	@return
		 *	A pointer to the component if one at least exists, nullptr otherwise.
		 */
		template <typename T>
		T* TryGetComponent()
		{
			return GetComponents<T>() | FirstOrDefault();
		}
		
		/**
		 *	@brief
		 *	Get the first component matching~ or aliased by the given type.
		 *
		 *	The order of components are non-deterministic so this method only make sense when it is trivial that only
		 *	one component will be available for that particular type.
		 *
		 *	@warning
		 *	If there may be the slightest doubt that the given component may not exist on this composable class, use
		 *	`TryGetComponent` instead as this function can crash at runtime.
		 *	
		 *	@tparam T  Desired component type.
		 *	
		 *	@return
		 *	A reference to the desired component. It is a runtime crash if the component doesn't exist.
		 */
		template <typename T>
		T const& GetComponent() const
		{
			const T* result = TryGetComponent<T>();
			ASSERT_CRASH(result, ->WithMessageF(TEXT_"Component {0} was unavailable.", TTypeName<T>));
			return *result;
		}
		
		/**
		 *	@brief
		 *	Get the first component matching~ or aliased by the given type.
		 *
		 *	The order of components are non-deterministic so this method only make sense when it is trivial that only
		 *	one component will be available for that particular type.
		 *
		 *	@warning
		 *	If there may be the slightest doubt that the given component may not exist on this composable class, use
		 *	`TryGetComponent` instead as this function can crash at runtime.
		 *	
		 *	@tparam T  Desired component type.
		 *	
		 *	@return
		 *	A reference to the desired component. It is a runtime crash if the component doesn't exist.
		 */
		template <typename T>
		T& GetComponent()
		{
			T* result = TryGetComponent<T>();
			ASSERT_CRASH(result, ->WithMessageF(TEXT_"Component {0} was unavailable.", TTypeName<T>));
			return *result;
		}
	};
}