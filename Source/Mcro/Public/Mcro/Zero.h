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
#include "Mcro/Concepts.h"

namespace Mcro::Zero
{
	using namespace Mcro::Concepts;
	
	/**
	 *	@brief
	 *	A type which acts like a number, but it always returns another FZero for any operations and converting it to
	 *	any scalar type yields 0. So if an FZero is found in a chain of operation the result will be always 0
	 */
	struct FZero
	{
		FORCEINLINE FZero() {}
		FORCEINLINE FZero(FZero const&) {}
		FORCEINLINE FZero(FZero&&) {}
		
		template <CScalar T>
		operator T() { return 0; }

		template <CScalar T>
		FZero(T) {}

		template <CScalar T> FZero& operator =   (T) { return *this; }
		template <CScalar T> FZero& operator +=  (T) { return *this; }
		template <CScalar T> FZero& operator -=  (T) { return *this; }
		template <CScalar T> FZero& operator *=  (T) { return *this; }
		template <CScalar T> FZero& operator /=  (T) { return *this; }
		template <CScalar T> FZero& operator %=  (T) { return *this; }
		template <CScalar T> FZero& operator &=  (T) { return *this; }
		template <CScalar T> FZero& operator |=  (T) { return *this; }
		template <CScalar T> FZero& operator ^=  (T) { return *this; }
		template <CScalar T> FZero& operator <<= (T) { return *this; }
		template <CScalar T> FZero& operator >>= (T) { return *this; }

		FORCEINLINE FZero& operator ++ () { return *this; }
		FORCEINLINE FZero& operator ++ (int) { return *this; }
		FORCEINLINE FZero& operator -- () { return *this; }
		FORCEINLINE FZero& operator -- (int) { return *this; }

		FORCEINLINE FZero operator + () const { return *this; }
		FORCEINLINE FZero operator - () const { return *this; }
		FORCEINLINE FZero operator ~ () const { return *this; }
		
		template <CScalar T> friend FZero operator + (FZero zero, T) { return zero; }
		template <CScalar T> friend FZero operator - (FZero zero, T) { return zero; }
		template <CScalar T> friend FZero operator * (FZero zero, T) { return zero; }
		template <CScalar T> friend FZero operator / (FZero zero, T) { return zero; }
		template <CScalar T> friend FZero operator % (FZero zero, T) { return zero; }
		template <CScalar T> friend FZero operator & (FZero zero, T) { return zero; }
		template <CScalar T> friend FZero operator | (FZero zero, T) { return zero; }
		template <CScalar T> friend FZero operator ^ (FZero zero, T) { return zero; }
		template <CScalar T> friend FZero operator << (FZero zero, T) { return zero; }
		template <CScalar T> friend FZero operator >> (FZero zero, T) { return zero; }
		
		template <CScalar T> friend FZero operator + (T, FZero zero) { return zero; }
		template <CScalar T> friend FZero operator - (T, FZero zero) { return zero; }
		template <CScalar T> friend FZero operator * (T, FZero zero) { return zero; }
		template <CScalar T> friend FZero operator / (T, FZero zero) { return zero; }
		template <CScalar T> friend FZero operator % (T, FZero zero) { return zero; }
		template <CScalar T> friend FZero operator & (T, FZero zero) { return zero; }
		template <CScalar T> friend FZero operator | (T, FZero zero) { return zero; }
		template <CScalar T> friend FZero operator ^ (T, FZero zero) { return zero; }
		template <CScalar T> friend FZero operator << (T, FZero zero) { return zero; }
		template <CScalar T> friend FZero operator >> (T, FZero zero) { return zero; }
		
		template <CScalar T> friend bool operator == (FZero, T scalar) { return scalar == 0; }
		template <CScalar T> friend bool operator != (FZero, T scalar) { return scalar != 0; }
		template <CScalar T> friend bool operator <=> (FZero, T scalar) { return scalar <=> 0; }
		
		template <CScalar T> friend bool operator == (T scalar, FZero) { return scalar == 0; }
		template <CScalar T> friend bool operator != (T scalar, FZero) { return scalar != 0; }
		template <CScalar T> friend bool operator <=> (T scalar, FZero) { return 0 <=> scalar; }
	};
}
