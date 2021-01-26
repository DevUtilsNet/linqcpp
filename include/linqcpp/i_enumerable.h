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

#include "optional.h"

namespace linq
{
template< class V >
struct IEnumerableIterator
{
   using ResultType = d::optional< V >;

   virtual ~IEnumerableIterator() = default;
   virtual ResultType Next() const = 0;
   virtual bool Eq( const IEnumerableIterator* i ) const = 0;
};

template< class V >
struct IEnumerable
{
   virtual ~IEnumerable() = default;
   virtual size_t GetCapacity() const = 0;
   virtual std::unique_ptr< IEnumerableIterator< V > > CreateIterator() const = 0;
};
} // namespace linq