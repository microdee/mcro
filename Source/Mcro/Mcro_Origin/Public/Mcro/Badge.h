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

namespace Mcro::Badge
{
	/**
	 * Use this template to give exclusive access to functions to specific classes, like so:
	 * class A;
	 * class B;
	 * class C;
	 * 
	 * class A
	 * {
	 *     static void CallFromB(TBadge<B> badge) {...}
	 * };
	 *
	 * class B
	 * {
	 *     void Foobar()
	 *     {
	 *         // OK
	 *         A::CallFromB({});
	 *     }
	 * };
	 *
	 * class C
	 * {
	 *     void Foobar()
	 *     {
	 *         // compile error:
	 *         // badge template instance declared in A::CallFromB is only friends with class B,
	 *         // therefore the private default constructor is inaccessible from class C
	 *         A::CallFromB({});
	 *     }
	 * };
	 */
	template<class T>
	class TBadge
	{
		friend T;
	private: TBadge() {}
	};
}

template <typename T>
using TBadge = Mcro::Badge::TBadge<T>;
