// https://github.com/DevUtilsNet/linqcpp
// Copyright (C) 2021 Kapitonov Maxim
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#pragma once

#include "i_enumerable.h"
#include "linqcpp.h"

#include <memory>

namespace linq
{
namespace d
{

template< class V >
struct EnumerableShim
{
   struct Iterator
   {
      using ResultType = d::optional< V >;
      using pointer = typename ResultType::pointer_type;
      using value_type = typename ResultType::value_type;
      using reference = typename ResultType::reference_type;

      std::unique_ptr< IEnumerableIterator< V > > mInnerIterator;

      ResultType Next() const
      {
         return mInnerIterator->Next();
      }

      bool operator==( const Iterator& i ) const
      {
         return mInnerIterator->Eq( i.mInnerIterator.get() );
      }
   };

   std::unique_ptr< IEnumerable< V > > mInnerEnumerable;

   size_t GetCapacity() const
   {
      return mInnerEnumerable->GetCapacity();
   }

   Iterator CreateIterator() const
   {
      return { mInnerEnumerable->CreateIterator() };
   }
};

template< class I >
struct EnumerableIterator : IEnumerableIterator< typename I::ResultType::value_type >
{
   using base = IEnumerableIterator< typename I::ResultType::value_type >;
   using ResultType = typename I::ResultType;

   I mInnerIterator;

   constexpr EnumerableIterator( I iterator )
      : mInnerIterator{ std::move( iterator ) }
   {
   }

   ResultType Next() const override
   {
      return mInnerIterator.Next();
   }

   bool Eq( const base* iterator ) const override
   {
      return mInnerIterator == static_cast< const EnumerableIterator* >( iterator )->mInnerIterator;
   }
};

template< class T >
struct Enumerable : IEnumerable< typename T::ValueType >
{
   T mInnerEnumerable;

   constexpr Enumerable( T t )
      : mInnerEnumerable{ std::move( t ) }
   {
   }

   size_t GetCapacity() const override
   {
      return mInnerEnumerable.GetCapacity();
   }

   std::unique_ptr< IEnumerableIterator< typename T::ValueType > > CreateIterator() const override
   {
      return std::unique_ptr< IEnumerableIterator< typename T::ValueType > >{ new EnumerableIterator{ mInnerEnumerable.CreateIterator() } };
   }
};

template< class T >
auto Shim< T >::ToEnumerable() const&
{
   return std::unique_ptr< IEnumerable< ValueType > >{ new Enumerable{ *this } };
}

template< class T >
auto Shim< T >::ToEnumerable() &&
{
   return std::unique_ptr< IEnumerable< ValueType > >{ new Enumerable{ std::move( *this ) } };
}
} // namespace d

template< class V >
d::Shim< d::EnumerableShim< V > > From( std::unique_ptr< IEnumerable< V > > t )
{
   return { { { std::move( t ) } } };
}
} // namespace linq