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
	 *	//// Given the following types we want to use as components:
	 *	
	 *	struct FSimpleComponent { int A = 0; };
	 *	
	 *	struct IBaseComponent { int B; };
	 *	
	 *	//                                | This allows us not repeating ourselves, more on this later
	 *	//                                V
	 *	struct FComponentImplementation : TInherit<IBaseComponent>
	 *	{
	 *		FComponentImplementation(int b, int c) : B(b), C(c) {}
	 *		int C;
	 *	};
	 *	
	 *	struct IRegularBase { int D; };
	 *	
	 *	//                         | We have to repeat this if we want to get components with this base class
	 *	//                         V
	 *	struct FRegularInherited : IRegularBase
	 *	{
	 *		FRegularInherited(int d, int e) : D(d), E(e) {}
	 *		int E;
	 *	};
	 *	
	 *	//// Given the following composable type:
	 *	
	 *	class FMyComposableType : public IComposable {};
	 *	
	 *	//// Declare their composition at construction:
	 *	
	 *	auto MyStuff = FMyComposableType()
	 *		.With<FSimpleComponent>()                 // <- Simply add components with their types
	 *		.With(new FComponentImplementation(1, 2)) // <- Or simply use new operator
	 *		                                          //    (IComposable assumes ownership)
	 *		                                          //    Because FComponentImplementation uses TInherit
	 *		                                          //    IBaseComponent is not needed to be repeated here
	 *		.With(new FRegularInherited(3, 4))        //
	 *		    .WithAlias<IRegularBase>()            // <- FRegularInherited cannot tell automatically that it
	 *		                                          //    inherits from IRegularBase so we need to specify
	 *		                                          //    that here explicitly in case we want to get
	 *		                                          //    FRegularInherited component via its base class
	 *	;
	 *	
	 *	//// Get components at runtime:
	 *	
	 *	int a = MyStuff.Get<FSimpleComponent>().A; //
	 *	// -> 0                                    //
	 *	int b = MyStuff.Get<IBaseComponent>().B;   // <- We can get the component via base class here, only
	 *	// -> 1                                    //    because it was exposed via TInherit
	 *	int d = MyStuff.Get<IRegularBase>().D;     // <- We can get the component via base class here, because
	 *	// -> 3                                    //    we explicitly specified it during registration
	 *	FVector* v = MyStuff.TryGet<FVector>();    // <- If there's any doubt that a component may not have
	 *	// -> nullptr; FVector wasn't a component  //    been registered, use TryGet instead.
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
	 *	//// Given the following types we want to use as components:
	 *	
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
	 *	//// Given the following composable types:
	 *	
	 *	class FExpectedParent : public IComposable {};
	 *	class FSomeOtherParent : public IComposable {};
	 *	
	 *	//// Declare their composition at construction:
	 *	
	 *	auto MyOtherStuff = FExpectedParent()
	 *		.With<FChillComponent>()  // OK, and OnComponentRegistered is called
	 *		.With<FStrictComponent>() // OK, and OnComponentRegistered is called
	 *	
	 *	auto MyStuff = FSomeOtherParent()
	 *		.With<FChillComponent>()  // OK, but OnComponentRegistered won't be called.
	 *		
	 *		.With<FStrictComponent>() // COMPILE ERROR, CCompatibleComponent concept is not satisfied because
	 *		                          // FSomeOtherParent is not convertible to FExpectedParent at
	 *		                          // OnComponentRegistered(FExpectedParent& to)
	 *	;
	 *	@endcode
	 *	
	 *	Explicit components can explicitly support multiple composable classes via function overloading or templating
	 *	(with deduced type parameters).
	 *	
	 *	If a component type uses `TInherit` template or has a `using Bases = TTypes<...>` member alias in a similar way:
	 *	@code
	 *	class FMyComponent : public TInherit<IFoo, IBar, IEtc>
	 *	{
	 *		// ...
	 *	}
	 *	@endcode
	 *	Then the specified base classes will be automatically registered as component aliases. When this is used for
	 *	explicit components, `IComponent` or `IStrictComponent` is strongly discouraged to be used in `TInherit`'s
	 *	parameter pack. So declare inheritance the following way:
	 *	
	 *	@code
	 *	class FMyComponent
	 *		: public TInherit<IFoo, IBar, IEtc>
	 *		, public IComponent
	 *	{
	 *		// ...
	 *	}
	 *	@endcode
	 *	
	 *	@todo
	 *	C++ 26 has promising proposal for static value-based reflection, which can gather metadata from classes
	 *	or even emit them. The best summary I found so far is a stack-overflow answer https://stackoverflow.com/a/77477029
	 *	Once that's available we can gather base classes in compile time, and do dynamic casting of objects without
	 *	the need for intrusive extra syntax, or extra work at construction.
	 *	Currently GCC's `__bases` would be perfect for the job, but other popular compilers don't have similar
	 *	intrinsics. Once such a feature becomes widely available base classes can be automatically added as aliases for
	 *	registered components.
	 */
	class MCRO_API IComposable
	{
		FTypeHash LastAddedComponentHash = 0;
		mutable TMap<FTypeHash, FAny> Components;
		mutable TMap<FTypeHash, TArray<FTypeHash>> ComponentAliases;

		bool HasExactComponent(FTypeHash typeHash) const;
		bool HasComponentAliasUnchecked(FTypeHash typeHash) const;
		bool HasComponentAlias(FTypeHash typeHash) const;
		void AddComponentAlias(FTypeHash mainType, FTypeHash validAs);
		
		template <typename ValidAs>
		void AddComponentAlias(FTypeHash mainType)
		{
			Components[mainType].WithAlias<ValidAs>();
			AddComponentAlias(mainType, TTypeHash<ValidAs>);

			if constexpr (CHasBases<ValidAs>)
			{
				Mcro::Any::Detail::ForEachExplicitBase<typename ValidAs::Bases>([&, this] <typename Base> (TTypes<Base>&&)
				{
					AddComponentAlias(mainType, TTypeHash<Base>);
				});
			}
		}
		
		ranges::any_view<FAny*> GetExactComponent(FTypeHash typeHash) const;
		ranges::any_view<FAny*> GetAliasedComponents(FTypeHash typeHash) const;

	protected:
		/**
		 *	@brief
		 *	Override this function in your composable class to do custom logic when a component is added. A bit of
		 *	dynamically typed programming is needed through the FAny API.
		 *
		 *	This is executed in AddComponent before IComponent::OnComponentRegistered and after automatic aliases has
		 *	been set up (if they're available). This is not executed with subsequent setup of manual aliases. 
		 *	
		 *	@param component  The component being added. Query component type with the FAny API
		 */
		TFunction<void(FAny&)> OnComponentAdded;
		
	public:

		/**
		 *	@brief   Get components determined at runtime
		 *	@param   typeHash  The runtime determined type-hash the desired components are represented with
		 *	@return  A type erased range view for all the components matched with given type-hash
		 */
		ranges::any_view<FAny*> GetComponentsDynamic(FTypeHash typeHash) const;

		/**
		 *	@brief
		 *	Add a component to this composable class.
		 *
		 *	@tparam MainType  The exact component type (deduced from `newComponent`
		 *	@tparam     Self  Deducing this
		 *	@param      self  Deducing this
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
			
			FAny& boxedComponent = self.Components.Add(TTypeHash<MainType>, FAny(newComponent, facilities));
			self.LastAddedComponentHash = TTypeHash<MainType>;

			if constexpr (CHasBases<MainType>)
			{
				Mcro::Any::Detail::ForEachExplicitBase<typename MainType::Bases>([&] <typename Base> (TTypes<Base>&&)
				{
					// FAny also deals with CHasBases so we can skip explicitly registering them here
					self.AddComponentAlias(TTypeHash<MainType>, TTypeHash<Base>);
				});
			}

			OnComponentAdded(boxedComponent);
			if constexpr (CCompatibleExplicitComponent<MainType, Self>)
				newComponent->OnComponentRegistered(self);
		}
		
		/**
		 *	@brief
		 *	Add a default constructed component to this composable class. 
		 *
		 *	@tparam MainType  The exact component type
		 *	@tparam     Self  Deducing this
		 *	@param      self  Deducing this
		 *
		 *	@param facilities
		 *	Customization point for object copy/move and delete methods. See `TAnyTypeFacilities`
		 */
		template <CDefaultInitializable MainType, typename Self>
		requires CCompatibleComponent<MainType, Self>
		void AddComponent(this Self&& self, TAnyTypeFacilities<MainType> const& facilities = {})
		{
			Forward<Self>(self).template AddComponent<MainType, Self>(new MainType(), facilities);
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
				->WithDetails(TEXT_"Make sure `AddAlias` or `WithAlias` is called after `AddComponent` / `With`.")
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
		 *	@tparam     Self  Deducing this
		 *	@param      self  Deducing this
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
		auto With(this Self&& self, MainType* newComponent, TAnyTypeFacilities<MainType> const& facilities = {})
		{
			Forward<Self>(self).template AddComponent<MainType, Self>(newComponent, facilities);
			return StaticCastSharedRef<std::decay_t<Self>>(self.AsShared());
		}
		
		/**
		 *	@brief
		 *	Add a default constructed component to this composable class with a fluent API. 
		 *
		 *	This overload is available for composable classes which also inherit from `TSharedFromThis`.
		 *
		 *	@tparam MainType  The exact component type
		 *	@tparam     Self  Deducing this
		 *	@param      self  Deducing this
		 *
		 *	@param facilities
		 *	Customization point for object copy/move and delete methods. See `TAnyTypeFacilities`
		 *
		 *	@return
		 *	If the composable class also inherits from `TSharedFromThis` return a shared ref.
		 */
		template <CDefaultInitializable MainType, CSharedFromThis Self>
		requires CCompatibleComponent<MainType, Self>
		auto With(this Self&& self, TAnyTypeFacilities<MainType> const& facilities = {})
		{
			Forward<Self>(self).template AddComponent<MainType, Self>(facilities);
			return StaticCastSharedRef<std::decay_t<Self>>(self.AsShared());
		}

		/**
		 *	@brief
		 *	Add a component to this composable class with a fluent API.
		 *
		 *	This overload is available for composable classes which are not explicitly meant to be used with shared pointers.
		 *
		 *	@tparam MainType  The exact component type (deduced from `newComponent`
		 *	@tparam     Self  Deducing this
		 *	@param      self  Deducing this
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
		decltype(auto) With(this Self&& self, MainType* newComponent, TAnyTypeFacilities<MainType> const& facilities = {})
		{
			Forward<Self>(self).template AddComponent<MainType, Self>(newComponent, facilities);
			return Forward<Self>(self);
		}

		/**
		 *	@brief
		 *	Add a default constructed component to this composable class with a fluent API. 
		 *
		 *	This overload is available for composable classes which are not explicitly meant to be used with shared pointers.
		 *
		 *	@tparam MainType  The exact component type
		 *	@tparam     Self  Deducing this
		 *	@param      self  Deducing this
		 *
		 *	@param facilities
		 *	Customization point for object copy/move and delete methods. See `TAnyTypeFacilities`
		 *
		 *	@return
		 *	Perfect-forwarded self.
		 */
		template <CDefaultInitializable MainType, typename Self>
		requires (CCompatibleComponent<MainType, Self> && !CSharedFromThis<Self>)
		decltype(auto) With(this Self&& self, TAnyTypeFacilities<MainType> const& facilities = {})
		{
			Forward<Self>(self).template AddComponent<MainType, Self>(facilities);
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
		 *		.With<FMyComponent>()
		 *			.WithAlias<FMyComponentBase>()
		 *			.WithAlias<IMyComponentInterface>()
		 *	;
		 *	@endcode
		 *
		 *	For declaring multiple aliases in one go, use `With(TTypes<...>)` member template method.
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
			Forward<Self>(self).template AddAlias<ValidAs>();
			return StaticCastSharedRef<std::decay_t<Self>>(self.AsShared());
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
		 *		.With<FMyComponent>()
		 *			.WithAlias<FMyComponentBase>()
		 *			.WithAlias<IMyComponentInterface>()
		 *	;
		 *	@endcode
		 *
		 *	For declaring multiple aliases in one go, use `With(TTypes<...>)` member template method.
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
			Forward<Self>(self).template AddAlias<ValidAs>();
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
		 *		.With<FMyComponent>().With(TAlias<
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
		auto With(this Self&& self, TTypes<ValidAs...>&&)
		{
			Forward<Self>(self).template AddAlias<ValidAs...>();
			return StaticCastSharedRef<std::decay_t<Self>>(self.AsShared());
		}

		/**
		 *	@brief
		 *	Add a list of types the last added component is convertible to and may be used to get the last component
		 *	among others which may list the same aliases.
		 *
		 *	Usage:
		 *	@code
		 *	auto result = IComposable()
		 *		.With<FMyComponent>().With(TAlias<
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
		decltype(auto) With(this Self&& self, TTypes<ValidAs...>&&)
		{
			Forward<Self>(self).template AddAlias<ValidAs...>();
			return Forward<Self>(self);
		}

		/**
		 *	@brief
		 *	Modify a component inline, with a lambda function. The component type is inferred from the function's first
		 *	argument, and a reference of that component is passed into it. The component must exist before calling this
		 *	method, or if it doesn't, the application will crash.
		 *	
		 *	@tparam Self  Deducing this
		 *	
		 *	@tparam Function
		 *	Function type for modifying a component inline. The component type is deduced from the first parameter of the
		 *	function. CV-ref qualifiers are not enforced but mutable-ref or const-ref are the only useful options.
		 *	Function result is discarded when returning anything.
		 *	
		 *	@param self  Deducing this
		 *	
		 *	@param function
		 *	Function for modifying a component inline. The component type is deduced from the first parameter of the
		 *	function. CV-ref qualifiers are not enforced but mutable-ref or const-ref are the only useful options.
		 *	Function result is discarded when returning anything.
		 *	
		 *	@return
		 *	If the composable class also inherits from `TSharedFromThis` return a shared ref.
		 */
		template <
			CSharedFromThis Self,
			CFunctionLike Function
		>
		requires (TFunction_ArgCount<Function> == 1)
		auto With(this Self&& self, Function&& function)
		{
			function(self.template Get<TFunction_ArgDecay<Function, 0>>());
			return StaticCastSharedRef<std::decay_t<Self>>(self.AsShared());
		}

		/**
		 *	@brief
		 *	Modify a component inline, with a lambda function. The component type is inferred from the function's first
		 *	argument, and a reference of that component is passed into it. The component must exist before calling this
		 *	method, or if it doesn't, the application will crash.
		 *	
		 *	@tparam Self  Deducing this
		 *	
		 *	@tparam Function
		 *	Function type for modifying a component inline. The component type is deduced from the first parameter of the
		 *	function. CV-ref qualifiers are not enforced but mutable-ref or const-ref are the only useful options.
		 *	Function result is discarded when returning anything.
		 *	
		 *	@param self  Deducing this
		 *	
		 *	@param function
		 *	Function for modifying a component inline. The component type is deduced from the first parameter of the
		 *	function. CV-ref qualifiers are not enforced but mutable-ref or const-ref are the only useful options.
		 *	Function result is discarded when returning anything.
		 *	
		 *	@return
		 *	Perfect-forwarded self.
		 */
		template <
			typename Self,
			CFunctionLike Function
		>
		requires (!CSharedFromThis<Self> && TFunction_ArgCount<Function> == 1)
		decltype(auto) With(this Self&& self, Function&& function)
		{
			function(self.template Get<TFunction_ArgDecay<Function, 0>>());
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
			return GetComponentsDynamic(TTypeHash<T>)
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
		const T* TryGet() const
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
		T* TryGet()
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
		 *	`TryGet` instead as this function can crash at runtime.
		 *	
		 *	@tparam T  Desired component type.
		 *	
		 *	@return
		 *	A reference to the desired component. It is a runtime crash if the component doesn't exist.
		 */
		template <typename T>
		T const& Get() const
		{
			const T* result = TryGet<T>();
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
		 *	`TryGet` instead as this function can crash at runtime.
		 *	
		 *	@tparam T  Desired component type.
		 *	
		 *	@return
		 *	A reference to the desired component. It is a runtime crash if the component doesn't exist.
		 */
		template <typename T>
		T& Get()
		{
			T* result = TryGet<T>();
			ASSERT_CRASH(result, ->WithMessageF(TEXT_"Component {0} was unavailable.", TTypeName<T>));
			return *result;
		}
	};
}