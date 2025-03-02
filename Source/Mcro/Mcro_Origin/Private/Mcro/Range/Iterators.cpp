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

#include "Mcro/Range/Iterators.h"

namespace Mcro::Range
{
	FTempStringIterator::FTempStringIterator(FString&& string, bool end)
	{
		Current       = end ? *string + string.Len() : *string;
		String        = MakeShared<TSharedStorage<FString>>();
		String->Value = MoveTempIfPossible(string);
	}

	auto FTempStringIterator::operator ++ () -> FTempStringIterator&
	{
		++Current;
		return *this;
	}

	auto FTempStringIterator::operator ++ (int) -> FTempStringIterator
	{
		FTempStringIterator previous = *this;
		++Current;
		return previous;
	}

	auto FTempStringIterator::operator -- () -> FTempStringIterator&
	{
		--Current;
		return *this;
	}

	auto FTempStringIterator::operator -- (int) -> FTempStringIterator
	{
		FTempStringIterator previous = *this;
		--Current;
		return previous;
	}

	auto FTempStringIterator::operator * () -> TCHAR const&
	{
		return *Current;
	}

	auto FTempStringIterator::operator * () const -> TCHAR const&
	{
		return *Current;
	}

	auto FTempStringIterator::operator += (int steps) -> FTempStringIterator&
	{
		Current += steps;
		return *this;
	}

	auto FTempStringIterator::operator -= (int steps) -> FTempStringIterator&
	{
		Current -= steps;
		return *this;
	}

	auto FTempStringIterator::operator + (int steps) const -> FTempStringIterator
	{
		FTempStringIterator result = *this;
		result.Current += steps;
		return result;
	}

	auto FTempStringIterator::operator - (int steps) const -> FTempStringIterator
	{
		FTempStringIterator result = *this;
		result.Current -= steps;
		return result;
	}
	
	auto operator - (FTempStringIterator const& l, FTempStringIterator const& r) -> FTempStringIterator::difference_type
	{
		return r.Current - l.Current;
	}
}
