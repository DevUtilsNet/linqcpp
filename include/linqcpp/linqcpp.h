// https://github.com/DevUtilsNet/linqcpp
// Copyright (C) 2018 Kapitonov Maxim
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

#include <array>
#include <deque>
#include <functional>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace linq
{
namespace d
{
template< class T >
struct Shim;

template< class T >
struct StdShim;
} // namespace d

template< class T >
d::Shim< d::StdShim< T > > From( T&& t );

template< class T >
d::Shim< T > From( d::Shim< T > t );

namespace d
{

template< class T >
constexpr T UnwrapReference( T&& t )
{
   return std::forward< T >( t );
}

template< class T >
constexpr T& UnwrapReference( std::reference_wrapper< T > t )
{
   return t;
}

template< class T >
struct ReferenceTraits
{
   using Type = T;
};

template< class T >
struct ReferenceTraits< optional< T >& >
{
   using Type = std::reference_wrapper< optional< T > >;
};

template< class T >
struct ReferenceTraits< const optional< T >& >
{
   using Type = std::reference_wrapper< const optional< T > >;
};

template< class I >
struct StdItAdr
{
   using Iterator = I;

   using iterator_category = std::forward_iterator_tag;
   using pointer = typename std::iterator_traits< Iterator >::pointer;
   using reference = typename std::iterator_traits< Iterator >::reference;
   using value_type = typename std::iterator_traits< Iterator >::value_type;

   using Reference = typename ReferenceTraits< reference >::Type;

   using ResultType = optional< Reference >;

   mutable I mCur;
   I mEnd;

   ResultType Next() const
   {
      if( mCur == mEnd )
      {
         return {};
      }
      ResultType ret;
      ret.emplace( *mCur++ );
      return ret;
   }

   bool operator==( const StdItAdr& i ) const
   {
      return mCur == i.mCur;
   }
};

template< class I >
struct ItStdAdr
{
   using Iterator = I;
   using ResultType = typename Iterator::ResultType;

   using iterator_category = std::forward_iterator_tag;

   using difference_type = int;
   using pointer = typename ResultType::pointer_type;
   using value_type = typename ResultType::value_type;
   using reference = typename ResultType::reference_type;

   I mIterator;
   mutable ResultType mResult;

   void EmplaceNext() const
   {
      auto value = mIterator.Next();
      if( !value.is_initialized() )
      {
         mResult.reset();
      }
      else
      {
         mResult.emplace( std::move( value ).value() );
      }
   }

   ItStdAdr& operator++()
   {
      EmplaceNext();
      return *this;
   }

   ItStdAdr operator++( int )
   {
      auto ret = *this;
      ++*this;
      return ret;
   }

   reference operator*() const
   {
      return mResult.value();
   }

   bool operator==( const ItStdAdr& i ) const
   {
      if( mResult.is_initialized() == i.mResult.is_initialized() )
      {
         if( !mResult.is_initialized() )
         {
            return true;
         }
         return mIterator == i.mIterator;
      }
      return false;
   }

   bool operator!=( const ItStdAdr& i ) const
   {
      return !( *this == i );
   }
};

template< class I, class R = typename std::decay_t< I >::ResultType >
struct ShimIt
{
   using ResultType = R;
   using pointer = typename ResultType::pointer_type;
   using value_type = typename ResultType::value_type;
   using reference = typename ResultType::reference_type;

   I mIterator;

   bool operator==( const ShimIt& i ) const
   {
      return mIterator == i.mIterator;
   }
};

template< class I >
ItStdAdr< I > MakeIterator( I it )
{
   auto result = it.Next();
   return { std::move( it ), std::move( result ) };
}

template< class I >
ItStdAdr< I > MakeEndIterator( I it )
{
   return { std::move( it ) };
}

template< class I >
StdItAdr< I > MakeIterator( I begin, I end )
{
   return { std::move( begin ), std::move( end ) };
}

template< class I, class R, class F >
struct ShimItF : ShimIt< I, R >
{
   F mFunctor;
   R Next() const
   {
      return mFunctor( this->mIterator );
   }
};

template< class R, class I, class F >
ShimItF< I, R, F > MakeIteratorShim( I it, F&& f )
{
   return { { std::move( it ) }, std::forward< F >( f ) };
}

template< class T >
struct ShimBase
{
   using DecayT = std::decay_t< T >;

   T mShim;

   size_t GetCapacity() const
   {
      return mShim.GetCapacity();
   }
};

template< class T >
struct Shim : ShimBase< T >
{
   using base = ShimBase< T >;
   using DecayT = typename base::DecayT;

   using ValueType = typename DecayT::Iterator::ResultType::value_type;
   using DecayValueType = std::decay_t< ValueType >;

   typename DecayT::Iterator CreateIterator() const
   {
      return this->mShim.CreateIterator();
   };

   // Where
   template< class F >
   struct WhereShim : ShimBase< T >
   {
      struct Iterator : ShimIt< typename DecayT::Iterator >
      {
         using base = ShimIt< typename DecayT::Iterator >;
         using ResultType = typename base::ResultType;

         const WhereShim* mOwner;

         ResultType Next() const
         {
            for( ;; )
            {
               auto result = this->mIterator.Next();
               if( result.is_initialized() )
               {
                  if( const_cast< F& >( mOwner->mFunctor )( result.value() ) )
                  {
                     return result;
                  }
               }
               else
               {
                  return {};
               }
            }
         }
      };

      F mFunctor;

      Iterator CreateIterator() const
      {
         return { { this->mShim.CreateIterator() }, this };
      };
   };

   template< class F >
   Shim< WhereShim< F > > Where( F&& f ) const&
   {
      return { { { { { this->mShim } }, std::forward< F >( f ) } } };
   }

   template< class F >
   Shim< WhereShim< F > > Where( F&& f ) &&
   {
      return { { { { { std::forward< T >( this->mShim ) } }, std::forward< F >( f ) } } };
   }

   // Select
   template< class V, class F >
   struct SelectShim : ShimBase< T >
   {
      struct Iterator : ShimIt< typename DecayT::Iterator >
      {
         using base = ShimIt< typename DecayT::Iterator >;
         using ResultType = optional< V >;

         const SelectShim* mOwner;

         ResultType Next() const
         {
            auto result = this->mIterator.Next();
            if( result.is_initialized() )
            {
               return static_cast< V >( const_cast< F& >( mOwner->mFunctor )( std::move( result ).value() ) );
            }
            return {};
         }
      };

      F mFunctor;

      Iterator CreateIterator() const
      {
         return { { this->mShim.CreateIterator() }, this };
      };
   };

   template< class V, class F >
   Shim< SelectShim< V, F > > Select( F&& f ) const&
   {
      return { { { { { this->mShim } }, std::forward< F >( f ) } } };
   }

   template< class V, class F >
   Shim< SelectShim< V, F > > Select( F&& f ) &&
   {
      return { { { { { std::forward< T >( this->mShim ) } }, std::forward< F >( f ) } } };
   }

   //SelectWhere
   template< class V, class F >
   struct SelectWhereShim : ShimBase< T >
   {
      struct Iterator : ShimIt< typename DecayT::Iterator >
      {
         using base = ShimIt< typename DecayT::Iterator >;
         using ResultType = optional< V >;

         const SelectWhereShim* mOwner;

         ResultType Next() const
         {
            for( ;; )
            {
               auto result = this->mIterator.Next();
               if( result.is_initialized() )
               {
                  auto ret = const_cast< F& >( mOwner->mFunctor )( std::move( result ).value() );
                  if( UnwrapReference( ret ) )
                  {
                     return *UnwrapReference( std::move( ret ) );
                  }
               }
               else
               {
                  return {};
               }
            }
         }
      };

      F mFunctor;

      Iterator CreateIterator() const
      {
         return { { this->mShim.CreateIterator() }, this };
      };
   };

   template< class V, class F >
   Shim< SelectWhereShim< V, F > > SelectWhere( F&& f ) const&
   {
      return { { { { { this->mShim } }, std::forward< F >( f ) } } };
   }

   template< class V, class F >
   Shim< SelectWhereShim< V, F > > SelectWhere( F&& f ) &&
   {
      return { { { { { std::forward< T >( this->mShim ) } }, std::forward< F >( f ) } } };
   }

   // SelectMany
   template< class V, class F >
   struct SelectManyShim : ShimBase< T >
   {
      struct Iterator : ShimIt< typename DecayT::Iterator >
      {
         using base = ShimIt< typename DecayT::Iterator >;
         using ResultType = optional< V >;

         const SelectManyShim* mOwner;

         using ManyResult = std::invoke_result_t< F, ValueType& >;
         mutable optional< ManyResult > mManyResult;
         using ManyContainer = decltype( From( UnwrapReference( std::move( mManyResult ).value() ) ) );
         mutable optional< ManyContainer > mManyContainer;
         using ManyIterator = decltype( mManyContainer.value().mShim.CreateIterator() );
         mutable optional< ManyIterator > mManyIterator;

         ResultType Next() const
         {
            for( ;; )
            {
               if( mManyIterator.is_initialized() )
               {
                  ResultType result{ mManyIterator.value().Next() };
                  if( result.is_initialized() )
                  {
                     return result;
                  }
               }

               auto result = this->mIterator.Next();
               if( result.is_initialized() )
               {
                  mManyResult.emplace( const_cast< F& >( mOwner->mFunctor )( std::move( result ).value() ) );
                  mManyContainer.emplace( From( UnwrapReference( std::move( mManyResult ).value() ) ) );
                  mManyIterator.emplace( mManyContainer.value().mShim.CreateIterator() );
                  continue;
               }
               mManyIterator.reset();
               mManyContainer.reset();
               mManyResult.reset();
               break;
            }
            return {};
         }
      };

      F mFunctor;

      Iterator CreateIterator() const
      {
         return { { this->mShim.CreateIterator() }, this };
      };
   };

   template< class V, class F >
   Shim< SelectManyShim< V, F > > SelectMany( F&& f ) const&
   {
      return { { { { { this->mShim } }, std::forward< F >( f ) } } };
   }

   template< class V, class F >
   Shim< SelectManyShim< V, F > > SelectMany( F&& f ) &&
   {
      return { { { { { std::forward< T >( this->mShim ) } }, std::forward< F >( f ) } } };
   }

   // Concat
   template< class T2 >
   struct ConcatShim : ShimBase< T >
   {
      struct Iterator : ShimIt< typename DecayT::Iterator >
      {
         using base = ShimIt< typename DecayT::Iterator >;

         const ConcatShim* mOwner;

         mutable bool mFlag = {};

         using RhsIterator = decltype( mOwner->mConcatContainer.mShim.CreateIterator() );
         mutable RhsIterator mRhsIterator;

         using R1 = typename base::ResultType;
         using R2 = typename RhsIterator::ResultType;
         using V1 = typename R1::value_type;
         using V2 = typename R2::value_type;

         using ResultType = std::conditional_t<
            !std::is_reference< V1 >::value, R1,
            std::conditional_t<
               !std::is_reference< V2 >::value, R2,
               std::conditional_t< std::is_const<
                                      std::remove_reference_t< V1 > >::value,
                                   R1, R2 > > >;

         ResultType Next() const
         {
            if( !mFlag )
            {
               auto result = this->mIterator.Next();
               if( result.is_initialized() )
               {
                  return std::move( result ).value();
               }
               mFlag = true;
               mRhsIterator = mOwner->mConcatContainer.mShim.CreateIterator();
            }

            {
               auto result = mRhsIterator.Next();
               if( result.is_initialized() )
               {
                  return std::move( result ).value();
               }
            }

            return {};
         }
      };

      T2 mContainer;

      using ConcatContainer = decltype( From( std::forward< T2 >( mContainer ) ) );
      ConcatContainer mConcatContainer = From( std::forward< T2 >( mContainer ) );

      Iterator CreateIterator() const
      {
         return { { this->mShim.CreateIterator() }, this };
      };

      size_t GetCapacity() const
      {
         return this->mShim.GetCapacity() + mConcatContainer.mShim.GetCapacity();
      }
   };

   template< class T2 >
   Shim< ConcatShim< T2 > > Concat( T2&& t ) const&
   {
      return { { { { { this->mShim } }, std::forward< T2 >( t ) } } };
   }

   template< class T2 >
   Shim< ConcatShim< T2 > > Concat( T2&& t ) &&
   {
      return { { { { { std::forward< T >( this->mShim ) } }, std::forward< T2 >( t ) } } };
   }

   // ExcludeIntersect
   template< typename T2, typename F, bool Exclude >
   struct ExcludeIntersectShim : ShimBase< T >
   {
      struct Iterator : ShimIt< typename DecayT::Iterator >
      {
         using base = ShimIt< typename DecayT::Iterator >;
         using ResultType = typename base::ResultType;

         const ExcludeIntersectShim* mOwner;

         ResultType Next() const
         {
            for( ;; )
            {
               auto result = this->mIterator.Next();
               if( result.is_initialized() )
               {
                  auto it = mOwner->mExcludeIntersectSet.find( const_cast< F& >( mOwner->mFunctor )( result.value() ) );
                  if constexpr( Exclude )
                  {
                     if( it == mOwner->mExcludeIntersectSet.end() )
                     {
                        return result;
                     }
                  }
                  else
                  {
                     if( it != mOwner->mExcludeIntersectSet.end() )
                     {
                        return result;
                     }
                  }
               }
               else
               {
                  break;
               }
            }
            return {};
         }
      };

      constexpr ExcludeIntersectShim( T&& t, T2&& t2, F&& f )
         : ShimBase< T >{ std::forward< T >( t ) }
         , mFunctor{ std::forward< F >( f ) }
         , mExcludeIntersectSet{ From( std::forward< T2 >( t2 ) ).ToUnorderedSet() }
      {
      }

      F mFunctor;
      decltype( From( std::declval< T2 >() ).ToUnorderedSet() ) mExcludeIntersectSet;

      Iterator CreateIterator() const
      {
         return { { this->mShim.CreateIterator() }, this };
      };
   };

   template< typename T2, typename F >
   Shim< ExcludeIntersectShim< T2, F, true > > Exclude( T2&& t, F&& f ) const&
   {
      return { { { this->mShim, std::forward< T2 >( t ), std::forward< F >( f ) } } };
   }

   template< typename T2, typename F >
   Shim< ExcludeIntersectShim< T2, F, true > > Exclude( T2&& t, F&& f ) &&
   {
      return { { { std::forward< T >( this->mShim ), std::forward< T2 >( t ), std::forward< F >( f ) } } };
   }

   template< typename T2 >
   auto Exclude( T2&& t ) const&
   {
      return Exclude( std::forward< T2 >( t ), []( auto&& m ) { return m; } );
   }

   template< typename T2 >
   auto Exclude( T2&& t ) &&
   {
      return std::move( *this ).Exclude( std::forward< T2 >( t ), []( auto&& m ) { return m; } );
   }

   template< typename T2, typename F >
   Shim< ExcludeIntersectShim< T2, F, false > > Intersect( T2&& t, F&& f ) const&
   {
      return { { { this->mShim, std::forward< T2 >( t ), std::forward< F >( f ) } } };
   }

   template< typename T2, typename F >
   Shim< ExcludeIntersectShim< T2, F, false > > Intersect( T2&& t, F&& f ) &&
   {
      return { { { std::forward< T >( this->mShim ), std::forward< T2 >( t ), std::forward< F >( f ) } } };
   }

   template< typename T2 >
   auto Intersect( T2&& t ) const&
   {
      return Intersect( std::forward< T2 >( t ), []( auto&& m ) { return m; } );
   }

   template< typename T2 >
   auto Intersect( T2&& t ) &&
   {
      return std::move( *this ).Intersect( std::forward< T2 >( t ), []( auto&& m ) { return m; } );
   }

   // Cast
   template< class V >
   auto Cast() const&
   {
      return this->template Select< V >( []( auto&& m ) -> V { return static_cast< V >( m ); } );
   }

   template< class V >
   auto Cast() &&
   {
      return std::move( *this ).template Select< V >( []( auto&& m ) -> V { return static_cast< V >( m ); } );
   }

   // Until
   template< class F >
   struct UntilShim : ShimBase< T >
   {
      struct Iterator : ShimIt< typename DecayT::Iterator >
      {
         using base = ShimIt< typename DecayT::Iterator >;
         using ResultType = typename base::ResultType;

         const UntilShim* mOwner;

         mutable bool mBreak = {};

         ResultType Next() const
         {
            if( mBreak )
            {
               return {};
            }
            auto result = this->mIterator.Next();
            if( result.is_initialized() )
            {
               if( !const_cast< F& >( mOwner->mFunctor )( result.value() ) )
               {
                  return result;
               }
            }
            mBreak = true;
            return {};
         }
      };

      F mFunctor;

      Iterator CreateIterator() const
      {
         return { { this->mShim.CreateIterator() }, this };
      };
   };

   template< class F >
   Shim< UntilShim< F > > Until( F&& f ) const&
   {
      return { { { { { this->mShim } }, std::forward< F >( f ) } } };
   }

   template< class F >
   Shim< UntilShim< F > > Until( F&& f ) &&
   {
      return { { { { { std::forward< T >( this->mShim ) } }, std::forward< F >( f ) } } };
   }

   // Take
   auto Take( size_t count ) const&
   {
      return this->Until( [count]( const auto& ) mutable { return count-- == 0; } );
   }

   template< class F >
   auto Take( size_t count ) &&
   {
      return std::move( *this ).Until( [count]( const auto& ) mutable { return count-- == 0; } );
   }

   // Skip
   auto Skip( size_t count ) const&
   {
      return this->Where( [count]( const auto& ) mutable { return count == 0 || count-- == 0; } );
   }

   template< class F >
   auto Skip( size_t count ) &&
   {
      return std::move( *this ).Where( [count]( const auto& ) mutable { return count == 0 || count-- == 0; } );
   }

   // Throttle
   struct ThrottleIteratorShim : ShimBase< T >
   {
      struct Iterator : ShimIt< typename DecayT::Iterator >
      {
         using base = ShimIt< typename DecayT::Iterator >;
         using ResultType = typename base::ResultType;

         mutable size_t mCount;
         const ThrottleIteratorShim* mOwner;

         ResultType Next() const
         {
            if( mCount > 0 )
            {
               --mCount;
               return this->mIterator.Next();
            }
            return {};
         }
      };

      size_t mCount;
      typename DecayT::Iterator mIterator;

      Iterator CreateIterator() const
      {
         return { { mIterator }, mCount, this };
      };
   };

   struct ThrottleShim : ShimBase< T >
   {
      struct Iterator : ShimIt< typename DecayT::Iterator >
      {
         using base = ShimIt< typename DecayT::Iterator >;
         using BaseResultType = typename base::ResultType;
         using ResultType = optional< Shim< typename Shim< const T& >::ThrottleIteratorShim > >;

         const ThrottleShim* mOwner;

         ResultType Next() const
         {
            auto iterator = this->mIterator;
            size_t i = 0;
            for( ; i < mOwner->mCount && this->mIterator.Next().is_initialized(); ++i )
            {
            }

            if( i == 0 )
            {
               return {};
            }

            return { { { { { { mOwner->mShim } }, mOwner->mCount, std::move( iterator ) } } } };
         }
      };

      size_t mCount;

      size_t GetCapacity() const
      {
         return mCount;
      }

      Iterator CreateIterator() const
      {
         return { { this->mShim.CreateIterator() }, this };
      };
   };

   Shim< ThrottleShim > Throttle( size_t count ) const&
   {
      return { { { { { this->mShim } }, count } } };
   }

   Shim< ThrottleShim > Throttle( size_t count ) &&
   {
      return { { { { { std::forward< T >( this->mShim ) } }, count } } };
   }

   //Distinct
   template< class F >
   auto Distinct( F&& f ) const&
   {
      std::unordered_set< std::invoke_result_t< F, DecayValueType > > set;
      return this->Where( [set{ std::move( set ) }, f{ std::forward< F >( f ) }]( const DecayValueType& m ) mutable {
         return set.insert( f( m ) ).second;
      } );
   }

   template< class F >
   auto Distinct( F&& f ) &&
   {
      std::unordered_set< std::invoke_result_t< F, DecayValueType > > set;
      return std::move( *this ).Where( [set{ std::move( set ) }, f{ std::forward< F >( f ) }]( const DecayValueType& m ) mutable {
         return set.insert( f( m ) ).second;
      } );
   }

   auto Distinct() const&
   {
      return this->Distinct( []( DecayValueType m ) { return m; } );
   }

   auto Distinct() &&
   {
      return std::move( *this ).Distinct( []( DecayValueType m ) { return m; } );
   }

   // Move
   auto Move() const&
   {
      return this->Select< std::decay_t< DecayValueType > >( []( DecayValueType& m ) { return std::move( m ); } );
   }

   auto Move() &&
   {
      return std::move( *this ).template Select< std::decay_t< DecayValueType& > >( []( DecayValueType& m ) { return std::move( m ); } );
   }

   auto end() const
   {
      return MakeEndIterator( this->mShim.CreateIterator() );
   }

   auto begin() const
   {
      return MakeIterator( this->mShim.CreateIterator() );
   }

   Shim< std::add_lvalue_reference_t< T > > Ref()
   {
      return { { this->mShim } };
   }

   const Shim< std::add_lvalue_reference_t< T > > Ref() const
   {
      return { { const_cast< Shim< T >* >( this )->mShim } };
   }

   // #include <linqcpp/enumerable.h> is required
   auto ToEnumerable() const&;
   auto ToEnumerable() &&;

   template< class I >
   void StdEmplace( I i ) const
   {
      for( auto it = this->mShim.CreateIterator();; )
      {
         auto result = it.Next();
         if( result.is_initialized() )
         {
            i++ = std::move( result ).value();
         }
         else
         {
            break;
         }
      }
   }

   std::list< DecayValueType > ToList() const
   {
      std::list< DecayValueType > ret;
      StdEmplace( std::back_inserter( ret ) );
      return ret;
   }

   std::deque< DecayValueType > ToDeque() const
   {
      std::deque< DecayValueType > ret;
      StdEmplace( std::back_inserter( ret ) );
      return ret;
   }

   std::vector< DecayValueType > ToVector( size_t capacity ) const
   {
      std::vector< DecayValueType > ret;
      ret.reserve( capacity );
      StdEmplace( std::back_inserter( ret ) );
      return ret;
   }

   std::vector< DecayValueType > ToVector() const
   {
      return ToVector( this->mShim.GetCapacity() );
   }

   std::vector< DecayValueType > ToOrderedVector() const
   {
      auto ret = ToVector();
      std::sort( std::begin( ret ), std::end( ret ) );
      return ret;
   }

   template< typename F >
   std::vector< DecayValueType > ToOrderedVector( F&& f ) const
   {
      auto ret = ToVector();
      std::sort( std::begin( ret ), std::end( ret ), std::forward< F >( f ) );
      return ret;
   }

   std::unordered_set< DecayValueType > ToUnorderedSet() const
   {
      std::unordered_set< DecayValueType > ret;
      ret.reserve( this->mShim.GetCapacity() );
      StdEmplace( std::inserter( ret, ret.end() ) );
      return ret;
   }

   template< typename ValueType2, typename F >
   std::unordered_set< ValueType2 > ToUnorderedSet( F&& f ) const
   {
      return this->Ref().template Select< ValueType2 >( std::forward< F >( f ) ).ToUnorderedSet();
   }

   template< typename K, typename V, typename KS, typename VS >
   auto ToUnorderedMap( KS&& keySelector, VS&& valueSelector ) const
   {
      std::unordered_map< K, V > ret;
      ret.reserve( this->mShim.GetCapacity() );
      for( auto it = this->mShim.CreateIterator();; )
      {
         auto result = it.Next();
         if( result.is_initialized() )
         {
            valueSelector( result.value(), ret[ keySelector( std::as_const( result.value() ) ) ] );
         }
         else
         {
            break;
         }
      }
      return ret;
   }

   template< typename K, typename KS >
   auto ToUnorderedMap( KS&& keySelector ) const
   {
      return ToUnorderedMap< K, DecayValueType >( std::forward< KS >( keySelector ), []( const auto v1, auto& v2 ) { v2 = v1; } );
   }

   size_t Count() const
   {
      size_t ret = 0;
      for( auto it = this->mShim.CreateIterator();; )
      {
         auto result = it.Next();
         if( result.is_initialized() )
         {
            ++ret;
         }
         else
         {
            break;
         }
      }
      return ret;
   }

   d::optional< DecayValueType > SumOrNone() const
   {
      d::optional< DecayValueType > ret;
      for( auto it = this->mShim.CreateIterator();; )
      {
         auto result = it.Next();
         if( result.is_initialized() )
         {
            if( !ret.is_initialized() )
            {
               ret = DecayValueType{};
            }
            ret.value() = ret.value() + result.value();
         }
         else
         {
            break;
         }
      }
      return ret;
   }

   DecayValueType Sum() const
   {
      return SumOrNone().value_or( DecayValueType{} );
   }

   template< typename V = DecayValueType >
   optional< V > FirstOrNone() const
   {
      auto result = this->mShim.CreateIterator().Next();
      if( result.is_initialized() )
      {
         return std::move( result ).value();
      }
      return {};
   }

   template< typename V = DecayValueType, typename F >
   optional< V > FirstOrNone( F&& f ) const
   {
      return this->Ref().Where( std::forward< F >( f ) ).template FirstOrNone< V >();
   }

   template< typename V >
   V FirstOr( V&& v ) const
   {
      auto result = FirstOrNone< V >();
      if( !result.is_initialized() )
      {
         return std::forward< V >( v );
      }
      return std::move( result ).value();
   }

   template< typename F, typename V >
   V FirstOr( F&& f, V&& v ) const
   {
      return this->Ref().Where( std::forward< F >( f ) ).template FirstOr( std::forward< V >( v ) );
   }

   ValueType First() const
   {
      auto result = FirstOrNone< ValueType >();
      if( !result.is_initialized() )
      {
         throw std::out_of_range( "The element isn't found." );
      }
      return std::move( result ).value();
   }

   template< typename F >
   ValueType First( F&& f ) const
   {
      auto result = FirstOrNone< ValueType >( std::forward< F >( f ) );
      if( !result.is_initialized() )
      {
         throw std::out_of_range( "The element isn't found." );
      }
      return std::move( result ).value();
   }

   template< typename V = DecayValueType >
   optional< V > LastOrNone() const
   {
      optional< V > ret;

      for( auto iterator = this->mShim.CreateIterator();; )
      {
         auto result = iterator.Next();
         if( result.is_initialized() )
         {
            ret.emplace( result.value() );
         }
         else
         {
            break;
         }
      }
      return ret;
   }

   template< typename V = DecayValueType, typename F >
   optional< V > LastOrNone( F&& f ) const
   {
      return this->Ref().Where( std::forward< F >( f ) ).template LastOrNone< V >();
   }

   template< typename V >
   V LastOr( V&& v ) const
   {
      auto result = LastOrNone< V >();
      if( !result.is_initialized() )
      {
         return std::forward< V >( v );
      }
      return std::move( result ).value();
   }

   template< typename F, typename V >
   V LastOr( F&& f, V&& v ) const
   {
      return this->Ref().Where( std::forward< F >( f ) ).template LastOr( std::forward< V >( v ) );
   }

   ValueType Last() const
   {
      auto result = LastOrNone< ValueType >();
      if( !result.is_initialized() )
      {
         throw std::out_of_range( "The element isn't found." );
      }
      return std::move( result ).value();
   }

   template< typename F >
   ValueType Last( F&& f ) const
   {
      auto result = LastOrNone< ValueType >( std::forward< F >( f ) );
      if( !result.is_initialized() )
      {
         throw std::out_of_range( "The element isn't found." );
      }
      return std::move( result ).value();
   }

   template< typename V = DecayValueType >
   optional< V > SingleOrNone() const
   {
      optional< V > ret;

      auto iterator = this->mShim.CreateIterator();
      auto result = iterator.Next();
      if( result.is_initialized() )
      {
         ret.emplace( result.value() );
      }

      if( iterator.Next().is_initialized() )
      {
         return {};
      }

      return ret;
   }

   template< typename V = DecayValueType, typename F >
   optional< V > SingleOrNone( F&& f ) const
   {
      return this->Ref().Where( std::forward< F >( f ) ).template SingleOrNone< V >();
   }

   template< typename V >
   V SingleOr( V&& v ) const
   {
      auto result = SingleOrNone< V >();
      if( !result.is_initialized() )
      {
         return std::forward< V >( v );
      }
      return std::move( result ).value();
   }

   template< typename F, typename V >
   V SingleOr( F&& f, V&& v ) const
   {
      return this->Ref().Where( std::forward< F >( f ) ).template SingleOr( std::forward< V >( v ) );
   }

   ValueType Single() const
   {
      auto result = SingleOrNone< ValueType >();
      if( !result.is_initialized() )
      {
         throw std::out_of_range( "The element isn't found." );
      }
      return std::move( result ).value();
   }

   template< typename F >
   ValueType Single( F&& f ) const
   {
      auto result = SingleOrNone< ValueType >( std::forward< F >( f ) );
      if( !result.is_initialized() )
      {
         throw std::out_of_range( "The element isn't found." );
      }
      return std::move( result ).value();
   }

   template< typename V = DecayValueType >
   optional< V > MinOrNone() const
   {
      optional< V > ret;

      for( auto iterator = this->mShim.CreateIterator();; )
      {
         auto result = iterator.Next();
         if( result.is_initialized() )
         {
            if( !ret.is_initialized() || ret.value() > result.value() )
            {
               ret.emplace( std::move( result ).value() );
            }
         }
         else
         {
            break;
         }
      }
      return ret;
   }

   ValueType Min() const
   {
      auto result = MinOrNone< ValueType >();
      if( !result.is_initialized() )
      {
         throw std::out_of_range( "The element isn't found." );
      }
      return std::move( result ).value();
   }

   template< typename V = DecayValueType >
   optional< V > MaxOrNone() const
   {
      optional< V > ret;

      for( auto iterator = this->mShim.CreateIterator();; )
      {
         auto result = iterator.Next();
         if( result.is_initialized() )
         {
            if( !ret.is_initialized() || ret.value() < result.value() )
            {
               ret.emplace( std::move( result ).value() );
            }
         }
         else
         {
            break;
         }
      }
      return ret;
   }

   ValueType Max() const
   {
      auto result = MaxOrNone< ValueType >();
      if( !result.is_initialized() )
      {
         throw std::out_of_range( "The element isn't found." );
      }
      return std::move( result ).value();
   }

   template< typename A, typename F >
   A Aggregate( A a, F&& f ) const
   {
      for( auto iterator = this->mShim.CreateIterator();; )
      {
         auto result = iterator.Next();
         if( result.is_initialized() )
         {
            a = f( a, std::move( result ).value() );
         }
         else
         {
            break;
         }
      }
      return a;
   }

   bool Any() const
   {
      return this->mShim.CreateIterator().Next().is_initialized();
   }

   template< typename F >
   bool Any( F&& f ) const
   {
      return this->Ref().Where( std::forward< F >( f ) ).Any();
   }

   template< typename F >
   bool All( F&& f ) const
   {
      return !Any( [&]( const auto& m ) { return !f( m ); } );
   }

   template< typename V >
   bool Contains( const V& v ) const
   {
      for( auto iterator = this->mShim.CreateIterator();; )
      {
         auto result = iterator.Next();
         if( result.is_initialized() )
         {
            if( result.value() == v )
            {
               return true;
            }
         }
         else
         {
            break;
         }
      }
      return false;
   }

   template< typename C >
   bool IsIntersect( const C& c ) const
   {
      auto container = From( c );
      for( auto iterator = this->mShim.CreateIterator();; )
      {
         auto result = iterator.Next();
         if( !result.is_initialized() )
         {
            break;
         }

         for( auto iterator2 = container.mShim.CreateIterator();; )
         {
            auto result2 = iterator2.Next();
            if( !result2.is_initialized() )
            {
               break;
            }
            if( result.value() == result2.value() )
            {
               return true;
            }
         }
      }
      return false;
   }
};

template< class T >
struct StdShim
{
   using DecayT = std::decay_t< T >;
   using iterator = typename DecayT::iterator;
   using const_iterator = typename DecayT::const_iterator;

   T mContainer;

   using Iterator = StdItAdr< decltype( std::begin( mContainer ) ) >;

   size_t GetCapacity() const
   {
      return mContainer.size();
   }

   Iterator CreateIterator()
   {
      return MakeIterator( std::begin( mContainer ), std::end( mContainer ) );
   };

   Iterator CreateIterator() const
   {
      return const_cast< StdShim* >( this )->CreateIterator();
   };
};

template< class I >
struct StdItShim
{
   using iterator = I;
   using const_iterator = I;

   I mEnd;
   I mBegin;
   size_t mCapacity;

   using Iterator = StdItAdr< I >;

   size_t GetCapacity() const
   {
      return mCapacity;
   }

   Iterator CreateIterator() const
   {
      return MakeIterator( mBegin, mEnd );
   };
};

} // namespace d

template< class T >
d::Shim< d::StdShim< T > > From( T&& t )
{
   return { { std::forward< T >( t ) } };
}

template< typename I >
d::Shim< d::StdItShim< I > > From( I b, I e, size_t capacity )
{
   return { { e, b, capacity } };
}

template< class T >
d::Shim< T > From( d::Shim< T > t )
{
   return std::move( t );
}

template< typename P, size_t N >
constexpr auto From( P ( &p )[ N ] )
{
   return From( std::begin( p ), std::end( p ), N );
}

template< typename P, size_t N >
constexpr d::Shim< d::StdShim< std::array< P, N > > > From( P( &&p )[ N ] )
{
   std::array< P, N > array;
   for( size_t i = 0; i < N; ++i )
   {
      array[ i ] = std::move( p[ i ] );
   }
   return From( std::move( array ) );
}
} // namespace linq
