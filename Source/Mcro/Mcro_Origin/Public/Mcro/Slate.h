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
#include "Mcro/FunctionTraits.h"

/**
 *	@brief
 *	Extra functionalities for general Slate programming chores, including enhancements of the Slate declarative syntax
 */
namespace Mcro::Slate
{
	using namespace Mcro::FunctionTraits;

	/** @brief Constraining given type to a Slate widget */
	template <typename T>
	concept CWidget = CDerivedFrom<T, SWidget>;

	/** @brief Constraining given type to a slot of a widget */
	template <typename T>
	concept CSlot = CDerivedFrom<T, FSlotBase>;

	/** @brief Constraining given type to either a slot or a widget */
	template <typename T>
	concept CWidgetOrSlot = CWidget<T> || CSlot<T>;

	/** @brief Constraining given type to the arguments of either a widget or a slot */
	template <typename T>
	concept CWidgetOrSlotArguments = CSameAsDecayed<T, typename T::WidgetArgsType>;

	/** @brief Constraining given type to the arguments of a widget  */
	template <typename T>
	concept CWidgetArguments = requires(typename T::WidgetType& t) { t; };

	/** @brief Constraining given type to the arguments of a slot  */
	template <typename T>
	concept CSlotArguments = CDerivedFrom<T, FSlotBase::FSlotArguments>
		&& requires(T& args)
		{
			args.GetSlot();
		}
	;

	/** @brief Constraining given type to a widget which can receive slots */
	template <typename T>
	concept CWidgetWithSlots = requires(typename T::FSlot&)
	{
		T::Slot();
	};

	template <typename T>
	struct TArgumentsOf_S {};
	
	template <CWidget T>
	struct TArgumentsOf_S<T>
	{
		using Type = typename T::FArguments;
	};
	
	template <CSlot T>
	struct TArgumentsOf_S<T>
	{
		using Type = typename T::FSlotArguments;
	};

	/** @brief Get the type of arguments from either a widget or a slot type (FArguments or FSlotArguments) */
	template <typename T>
	using TArgumentsOf = typename TArgumentsOf_S<T>::Type;

	/**
	 *	@brief
	 *	Alias for an attribute block function which takes in reference of FArguments or FSlotArguments and returns the
	 *	same reference but presumably setting some Slate attributes before that. This is useful for modularizing the
	 *	Slate declarative syntax.
	 */
	template <CWidgetOrSlot T>
	using TAttributeBlock = TUniqueFunction<TArgumentsOf<T>&(TArgumentsOf<T>&)>;

	/** @brief Same as TAttributeBlock but allows to make copies of the functor */
	template <CWidgetOrSlot T>
	using TAttributeBlockCopyable = TFunction<TArgumentsOf<T>&(TArgumentsOf<T>&)>;

	/** @brief An attribute block which does nothing */
	template <CWidgetOrSlot T>
	TAttributeBlock<T> InertAttributeBlock = [](TArgumentsOf<T>& args) -> auto& { return args; };

	/**
	 *	@brief
	 *	The "append attribute block" operator which allows pre-defined "blocks of slate attributes" naturally fit inside
	 *	the Slate declarative syntax. Traditionally repeated structures in Slate were expressed as either explicit
	 *	mutations on widgets after they were created or as entirely separate compound widgets. Either way breaks the
	 *	flow of the declarative syntax and makes using Slate sometimes pretty clunky. This operator aims to make widget
	 *	composition more comfortable.
	 *	
	 *	@tparam  Arguments   Right hand side FArguments or FSlotArguments
	 *	@tparam  AttrBlock   The type of the attribute block function
	 *	@param   args        l-value reference right hand side FArguments or FSlotArguments
	 *	@param   attributes  An attribute block function
	 *	@return  The same reference as args or a new slot if that has been added inside the attribute block
	 */
	template <CWidgetOrSlotArguments Arguments, CFunctionLike AttrBlock>
	requires (
		TFunction_ArgCount<AttrBlock> == 1
		&& CSameAs<Arguments&, TFunction_Arg<AttrBlock, 0>>
	)
	TFunction_Return<AttrBlock> operator / (Arguments& args, const AttrBlock& attributes)
	{
		return attributes(args);
	}

	/**
	 *	@brief
	 *	The "append attribute block" operator which allows pre-defined "blocks of slate attributes" naturally fit inside
	 *	the Slate declarative syntax. Traditionally repeated structures in Slate were expressed as either explicit
	 *	mutations on widgets after they were created or as entirely separate compound widgets. Either way breaks the
	 *	flow of the declarative syntax and makes using Slate sometimes pretty clunky. This operator aims to make widget
	 *	composition more comfortable.
	 *	
	 *	@tparam  Arguments   Right hand side FArguments or FSlotArguments
	 *	@tparam  AttrBlock   The type of the attribute block function
	 *	@param   args        r-value reference right hand side FArguments or FSlotArguments
	 *	@param   attributes  An attribute block function
	 *	@return  The same reference as args or a new slot if that has been added inside the attribute block
	 */
	template <CWidgetOrSlotArguments Arguments, CFunctionLike AttrBlock>
	requires (
		TFunction_ArgCount<AttrBlock> == 1
		&& CSameAs<Arguments&, TFunction_Arg<AttrBlock, 0>>
	)
	TFunction_Return<AttrBlock> operator / (Arguments&& args, const AttrBlock& attributes)
	{
		return attributes(args);
	}

	/**
	 *	@brief  Add multiple slots at the same time with the declarative syntax derived from an input data array.
	 *	
	 *	@code
	 *	void SMyWidget::Construct(const FArguments& args)
	 *	{
	 *		using namespace Mcro::Slate;
	 *		
	 *		ChildSlot
	 *		[
	 *			SNew(SVerticalBox)
	 *			+ TSlots(args._DataArray, [](const FMyData& data)
	 *			{
	 *				FText dataText = FText::FromString(data.ToString());
	 *				return SVerticalBox::Slot()
	 *					. HAlign(HAlign_Fill)
	 *					. AutoHeight()
	 *					[
	 *						SNew(STextBlock)
	 *						. Text(dataText)
	 *					];
	 *			})
	 *			+ SVerticalBox::Slot()
	 *			. HAlign(HAlign_Fill)
	 *			. AutoHeight()
	 *			[
	 *				SNew(STextBlock)
	 *				. Text(INVTEXT_"Footer after the list of data")
	 *			]
	 *		];
	 *	}
	 *	@endcode
	 */
	template <
		CRange Range,
		CFunctionLike Transform,
		CFunctionLike OnEmpty = TUniqueFunction<TFunction_Return<Transform>()>,
		CSlotArguments SlotArguments = TFunction_Return<Transform>
	>
	requires (TFunction_ArgCount<Transform> == 1)
	struct TSlots
	{
		TSlots(const Range& range, Transform&& transform, TOptional<OnEmpty>&& onEmpty = {})
			: RangeRef(range)
			, TransformStorage(MoveTemp(transform))
			, OnEmptyStorage(MoveTemp(onEmpty))
		{}

		TSlots(const TSlots&) = delete;
		TSlots(TSlots&& o) noexcept
			: RangeRef(o.RangeRef),
			  TransformStorage(MoveTemp(o.TransformStorage)),
			  OnEmptyStorage(MoveTemp(o.OnEmptyStorage))
		{
		}

		TSlots& operator=(const TSlots&) = delete;
		TSlots& operator=(TSlots&& o) noexcept
		{
			if (this == &o)
				return *this;
			RangeRef = o.RangeRef;
			TransformStorage = MoveTemp(o.TransformStorage);
			OnEmptyStorage = MoveTemp(o.OnEmptyStorage);
			return *this;
		}

		template <CWidgetArguments Arguments>
		void Append(Arguments& args)
		{
			if (RangeRef.begin() == RangeRef.end() && OnEmptyStorage.IsSet())
			{
				args + OnEmptyStorage.GetValue()();
				return;
			}

			for (auto it = RangeRef.begin(); it != RangeRef.end(); ++it)
				args + TransformStorage(*it);
		}

	private:
		const Range& RangeRef;
		Transform TransformStorage;
		TOptional<OnEmpty> OnEmptyStorage;
	};

	/** @copydoc TSlots */
	template <
		CRange Range,
		CFunctionLike Transform,
		CFunctionLike OnEmpty,
		CSlotArguments SlotArguments,
		CWidgetArguments Arguments
	>
	Arguments& operator + (Arguments& args, TSlots<Range, Transform, OnEmpty, SlotArguments>&& slots)
	{
		slots.Append(args);
		return args;
	}

	/** @brief Convenience function for typing less when widget visibility depends on a boolean */
	MCRO_API EVisibility IsVisible(bool visible, EVisibility hiddenState = EVisibility::Collapsed);
}
