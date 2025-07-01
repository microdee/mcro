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
#include "Mcro/Slate.h"
#include "Mcro/AssertMacros.h"
#include "Mcro/Observable.h"
#include "Mcro/Threading.h"
#include "Mcro/Range.h"
#include "Mcro/Range/Views.h"
#include "Mcro/Range/Conversion.h"

namespace Mcro::Slate
{
	using namespace Mcro::Observable;
	using namespace Mcro::Error;
	using namespace Mcro::Threading;
	using namespace Mcro::Range;

	namespace Detail
	{
		template <
			typename Item,
			CRangeMember Range,
			CWidgetWithSlots ContainerWidget,
			CWidget ChildWidget,
			CRangeMember ChildrenRange,
			typename IndexType
		>
		class TReactiveWidgetBase : public SCompoundWidget
		{
		public:
			using StateRangeType = Range;
			using ChildrenRangeType = ChildrenRange;
			using ContainerSlotArguments = TArgumentsOf<typename ContainerWidget::FSlot>;
			
			using FCreateChild = TDelegate<ContainerSlotArguments(
				TSharedRef<ContainerWidget> const& container,
				Item const& from,
				IndexType const& at
			)>;
			using FUpdateChild = TDelegate<void(
				TSharedRef<ChildWidget> const& child,
				Item const& from,
				IndexType const& at
			)>;
			using FRemoveChild = TDelegate<void(
				TSharedRef<ContainerWidget> const& container,
				TSharedRef<ChildWidget> const& child,
				IndexType const& at
			)>;
			
		protected:
			
			IStateWeakPtr<Range> State;
			FCreateChild CreateChild;
			FUpdateChild UpdateChild;
			FRemoveChild RemoveChild;
			TSharedPtr<ContainerWidget> Container;
			ChildrenRange Children;
			virtual void OnStateChange(Range const& next) = 0;
			
			static void DefaultRemoveChild(FRemoveChild& delegate)
			{
				if constexpr (requires(ContainerWidget& container, TSharedRef<SWidget> child)
				{
					container.RemoveSlot(child);
				})
				{
					delegate = FRemoveChild::CreateLambda([](
						TSharedRef<ContainerWidget> const& container,
						TSharedRef<ChildWidget> const& child, int32 at
					) {
						container->RemoveSlot(child);
					});
				}
			}

			template <CWidgetArguments ThisArguments>
			requires requires(ThisArguments& args)
			{
				{ args._State }       -> CSameAsDecayed< IStatePtr<Range> >;
				{ args._Container }   -> CSameAsDecayed< TSharedPtr<ContainerWidget> >;
				{ args._CreateChild } -> CSameAsDecayed< FCreateChild >;
				{ args._UpdateChild } -> CSameAsDecayed< FUpdateChild >;
				{ args._RemoveChild } -> CSameAsDecayed< FRemoveChild >;
			}
			void ConstructBase(ThisArguments const& args)
			{
				ASSERT_CRASH(args._Container);
				ASSERT_CRASH(args._State);
				ASSERT_CRASH(args._CreateChild.IsBound());
				ASSERT_CRASH(args._RemoveChild.IsBound());
		
				Container = args._Container;
				State = args._State.ToWeakPtr();
				CreateChild = args._CreateChild;
				UpdateChild = args._UpdateChild;
				RemoveChild = args._RemoveChild;
		
				ChildSlot[Container.ToSharedRef()];

				args._State->OnChange(this, [this](Range const& next)
				{
					if (IsInGameThread()) OnStateChange(next);
					else
					{
						RunInGameThread(WeakSelf(this), [this]
						{
							if (auto state = State.Pin())
							{
								auto [value, lock] = state->GetOnAnyThread();
								OnStateChange(value);
							}
						});
					}
				});
			}
		};
	}

	/**
	 *	@brief
	 *	A widget template which can automatically handle changes in an input array state, with given delegates which
	 *	tell this widget how children are supposed to be created, updated and removed.
	 *	
	 *	@tparam            Item  The type of the items which are transformed into child widgets. 
	 *	@tparam ContainerWidget  The panel which provides the slots for the child widgets.
	 *	@tparam     ChildWidget  The storage type of child widgets, it's fine to leave it SWidget
	 */
	template <
		typename Item,
		CWidgetWithSlots ContainerWidget = SWidget,
		CWidget ChildWidget = SWidget,
		typename Base = Detail::TReactiveWidgetBase<
			Item, TArray<Item>,
			ContainerWidget, ChildWidget, TArray<TSharedRef<ChildWidget>>, int32
		>
	>
	class TArrayReactiveWidget : public Base
	{
	public:
		using FCreateChild = Base::FCreateChild;
		using FUpdateChild = Base::FUpdateChild;
		using FRemoveChild = Base::FRemoveChild;
		using ContainerSlotArguments = Base::ContainerSlotArguments;
		
		SLATE_BEGIN_ARGS(TArrayReactiveWidget)
			{
				Base::DefaultRemoveChild(_RemoveChild);
			}
			SLATE_ARGUMENT(IStatePtr<Base::StateRangeType>, State);
			SLATE_ARGUMENT(TSharedPtr<ContainerWidget>, Container);
			SLATE_EVENT(Base::FCreateChild, CreateChild);
			SLATE_EVENT(Base::FUpdateChild, UpdateChild);
			SLATE_EVENT(Base::FRemoveChild, RemoveChild);
		SLATE_END_ARGS()

		void Construct(FArguments const& args) { Base::ConstructBase(args); }

	protected:
		virtual void OnStateChange(Base::StateRangeType const& next) override
		{
			for (int i = 0; i < FMath::Max(next.Num(), Base::Children.Num()); ++i)
			{
				if (next.IsValidIndex(i) && Base::Children.IsValidIndex(i))
				{
					Base::UpdateChild.ExecuteIfBound(Base::Children[i], next[i], i);
					continue;
				}
				if (next.IsValidIndex(i))
				{
					typename ContainerWidget::FSlot* newSlot = nullptr;
					Base::CreateChild.Execute(Base::Container, next[i], i).Expose(newSlot);
					Base::Children.Add(StaticCastSharedRef<ChildWidget>(newSlot->GetWidget()));
					continue;
				}
				if (Base::Children.IsValidIndex(i))
				{
					Base::RemoveChild.Execute(Base::Container, Base::Children[i], i);
				}
			}
			if (Base::Children.Num() > next.Num()) Base::Children.SetNum(next.Num());
		}
	};
	
	/**
	 *	@brief
	 *	A widget template which can automatically handle changes in an input map state, with given delegates which
	 *	tell this widget how children are supposed to be created, updated and removed.
	 *	
	 *	@tparam             Key  The key associated with items. 
	 *	@tparam            Item  The type of the items which are transformed into child widgets. 
	 *	@tparam ContainerWidget  The panel which provides the slots for the child widgets.
	 *	@tparam     ChildWidget  The storage type of child widgets, it's fine to leave it SWidget
	 */
	template <
		typename Key, typename Item,
		CWidget ContainerWidget = SWidget,
		CWidget ChildWidget = SWidget,
		typename Base = Detail::TReactiveWidgetBase<
			Item, TMap<Key, Item>,
			ContainerWidget, ChildWidget, TMap<Key, TSharedRef<ChildWidget>>, Key
		>
	>
	class TMapReactiveWidget : public Base
	{
	public:
		using FCreateChild = Base::FCreateChild;
		using FUpdateChild = Base::FUpdateChild;
		using FRemoveChild = Base::FRemoveChild;
		
		SLATE_BEGIN_ARGS(TMapReactiveWidget)
			{
				Base::DefaultRemoveChild(_RemoveChild);
			}
			SLATE_ARGUMENT(IStatePtr<Base::StateRangeType>, State);
			SLATE_ARGUMENT(TSharedPtr<ContainerWidget>, Container);
			SLATE_EVENT(Base::FCreateChild, CreateChild);
			SLATE_EVENT(Base::FUpdateChild, UpdateChild);
			SLATE_EVENT(Base::FRemoveChild, RemoveChild);
		SLATE_END_ARGS()

		void Construct(FArguments const& args) { Base::ConstructBase(args); }

	protected:
		virtual void OnStateChange(Base::StateRangeType const& next) override
		{
			auto update = next
				| FilterTuple([this](Key const& key, Item const& value)
				{
					return Base::Children.Contains(key);
				})
				| GetKeys()
			;
			for (Key const& updating : update)
			{
				Base::UpdateChild.ExecuteIfBound(Base::Children[updating], next[updating], updating);
			}
			auto dismiss = Base::Children
				| FilterTuple([&](Key const& key, Item const& value)
				{
					return !next.Contains(key);
				})
				| GetKeys()
				| RenderAs<TArray>()
			;
			for (Key const& dismissing : dismiss)
			{
				Base::RemoveChild.Execute(Base::Container, Base::Children[dismissing], dismissing);
				Base::Children.Remove(dismissing);
			}
			auto add = next
				| FilterTuple([this](Key const& key, Item const& value)
				{
					return !Base::Children.Contains(key);
				})
				| GetKeys()
			;
			for (Key const& adding : add)
			{
				typename ContainerWidget::FSlot* newSlot = nullptr;
				Base::CreateChild.Execute(Base::Container, next[adding], adding).Expose(newSlot);
				Base::Children.Add(adding, StaticCastSharedRef<ChildWidget>(newSlot->GetWidget()));
			}
		}
	};
}
