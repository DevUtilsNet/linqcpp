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

#include <functional>
#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/optional.hpp>

namespace linq
{
template< typename P >
struct StdShim;
template< typename P >
struct StdConstShim;
template< typename P >
struct _shared_getter;
template< typename I >
struct ItShim;
template< typename P >
struct Shim;

template< typename P >
Shim< P > From( Shim< P >&& p )
{
   return std::move( p );
}
template< typename P >
StdShim< P > From( StdShim< P >&& p )
{
   return std::move( p );
}
template< typename P >
StdShim< P > From( P&& p )
{
   return StdShim< P >{ { { { _shared_getter< P >{ std::forward< P >( p ) } } } } };
}
template< typename I >
Shim< ItShim< I > > From( I&& b, I&& e, size_t capacity )
{
   return Shim< ItShim< I > >{ { std::forward< I >( b ), std::forward< I >( e ), capacity } };
}
template< typename P >
const StdConstShim< const P& > From( const P& p )
{
   return StdConstShim< const P& >{ { { { _shared_getter< const P& >{ p } } } } };
}

template< typename P >
auto From( std::initializer_list< P > p )
{
   return From( std::vector< P >{ std::move( p ) } );
}

namespace details
{
template< class T >
using optional = boost::optional< T >;

template< typename P >
size_t get_capacity( const P& )
{
   return 0;
}
template< typename T >
size_t get_capacity( const std::list< T >& l )
{
   return l.size();
}
template< typename T >
size_t get_capacity( const std::vector< T >& v )
{
   return v.size();
}
template< typename T >
size_t get_capacity( const std::unordered_set< T >& s )
{
   return s.size();
}
template< typename K, typename T >
size_t get_capacity( const std::unordered_map< K, T >& m )
{
   return m.size();
}

template< class T >
struct unwrap_reference
{
   using type = T;
};
template< class U >
struct unwrap_reference< std::reference_wrapper< U > >
{
   using type = U&;
};
template< class T >
using unwrap_reference_t = typename unwrap_reference< T >::type;
} // namespace details

template< typename I >
class ITearOffContainer;

namespace details
{
template< typename P >
struct LoShim
{
   P _p;
   constexpr static bool shared = P::shared;

   using iterator = typename P::iterator;
   using const_iterator = typename P::const_iterator;

   using lo_iterator = decltype( std::begin( _p ) );

   size_t get_capacity() const
   {
      return _p.get_capacity();
   }
};

template< typename I1, typename I2 >
struct LoIterator1
{
   using iterator_category = std::forward_iterator_tag;
   using pointer = typename std::iterator_traits< I2 >::pointer;
   using reference = typename std::iterator_traits< I2 >::reference;
   using value_type = typename std::iterator_traits< I2 >::value_type;
   using difference_type = typename std::iterator_traits< I2 >::difference_type;

   I2 _i;
   I1& operator++()
   {
      ++_i;
      return ( I1& )*this;
   }
   I1 operator++( int )
   {
      auto ret = ( I1& )*this;
      ++( I1& )*this;
      return ret;
   }
   reference operator*()
   {
      return *_i;
   }
   reference operator*() const
   {
      return *_i;
   }
   bool operator==( const I1& i ) const
   {
      return _i == i._i;
   }
   bool operator!=( const I1& i ) const
   {
      return !( ( const I1& )*this == i );
   }
};

template< typename I1, typename I2 >
struct LoIterator2 : LoIterator1< I1, I2 >
{
   I2 _e;
};

template< typename P2 >
struct HiShim1 : P2
{
   using typename P2::const_iterator;
   using typename P2::iterator;

   iterator end()
   {
      return { { std::end( this->_p ) } };
   }
   iterator begin()
   {
      return { { std::begin( this->_p ) } };
   }
   const_iterator end() const
   {
      return { { std::end( const_cast< HiShim1* >( this )->_p ) } };
   }
   const_iterator begin() const
   {
      return { { std::begin( const_cast< HiShim1* >( this )->_p ) } };
   }
};

template< typename P2 >
struct HiShim11 : P2
{
   using typename P2::const_iterator;
   using typename P2::iterator;

   iterator end()
   {
      return { { { std::end( this->_p ) } }, this };
   }
   iterator begin()
   {
      return { { { std::begin( this->_p ) } }, this };
   }
   const_iterator end() const
   {
      return { { { std::end( const_cast< HiShim11* >( this )->_p ) } }, this };
   }
   const_iterator begin() const
   {
      return { { { std::begin( const_cast< HiShim11* >( this )->_p ) } }, this };
   }
};

template< typename P2 >
struct HiShim2 : P2
{
   using typename P2::const_iterator;
   using typename P2::iterator;

   iterator end()
   {
      return { { { { std::end( this->_p ) } }, std::end( this->_p ) }, this };
   }
   iterator begin()
   {
      return { { { { std::begin( this->_p ) } }, std::end( this->_p ) }, this };
   }
   const_iterator end() const
   {
      return { { { { std::end( const_cast< HiShim2* >( this )->_p ) } }, std::end( const_cast< HiShim2* >( this )->_p ) }, this };
   }
   const_iterator begin() const
   {
      return { { { { std::begin( const_cast< HiShim2* >( this )->_p ) } }, std::end( const_cast< HiShim2* >( this )->_p ) }, this };
   }
};

template< typename P2 >
struct HiShim22 : P2
{
   using typename P2::const_iterator;
   using typename P2::iterator;

   iterator end()
   {
      return { { std::end( this->_p ), std::end( this->_p ), this } };
   }
   iterator begin()
   {
      return { { std::begin( this->_p ), std::end( this->_p ), this } };
   }
   const_iterator end() const
   {
      return { { std::end( const_cast< HiShim22* >( this )->_p ), std::end( const_cast< HiShim22* >( this )->_p ), this } };
   }
   const_iterator begin() const
   {
      return { { std::begin( const_cast< HiShim22* >( this )->_p ), std::end( const_cast< HiShim22* >( this )->_p ), this } };
   }
};
} // namespace details

template< typename P >
struct Shim : P
{
   using typename P::const_iterator;
   using typename P::iterator;

   template< typename F >
   struct WhereShim : details::LoShim< P >
   {
      F _f;
      using baseShim = details::LoShim< P >;

      struct WhereIt : details::LoIterator2< WhereIt, typename baseShim::lo_iterator >
      {
         const WhereShim* _o = nullptr;
         using base = details::LoIterator2< WhereIt, typename baseShim::lo_iterator >;
         bool _init = _chk();
         WhereIt& operator++()
         {
            base::operator++();
            _chk();
            return *this;
         }
         bool _chk()
         {
            for( ; base::_i != base::_e && !( bool )_o->_f( *base::_i ); ++base::_i )
            {
            }
            return true;
         }
      };

      using iterator = WhereIt;
      using const_iterator = WhereIt;
   };
   template< typename F >
   Shim< details::HiShim2< WhereShim< F > > > Where( F&& f )
   {
      return Shim< details::HiShim2< WhereShim< F > > >{ { { { *this }, std::forward< F >( f ) } } };
   }
   template< typename F >
   const Shim< details::HiShim2< WhereShim< F > > > Where( F&& f ) const
   {
      return Shim< details::HiShim2< WhereShim< F > > >{ { { { *this }, std::forward< F >( f ) } } };
   }

   template< typename V2, typename F >
   struct SelectShim : details::LoShim< P >
   {
      F _f;
      using baseShim = details::LoShim< P >;

      struct SelectIt : details::LoIterator1< SelectIt, typename baseShim::lo_iterator >
      {
         const SelectShim* _o = nullptr;

         using value_type = V2;
         using pointer = std::add_pointer_t< value_type >;
         using reference = V2;

         reference operator*()
         {
            return ( details::unwrap_reference_t< decltype( _o->_f( *this->_i ) ) > )_o->_f( *this->_i );
         }
         reference operator*() const
         {
            return ( details::unwrap_reference_t< decltype( _o->_f( *this->_i ) ) > )_o->_f( *this->_i );
         }
      };

      using iterator = SelectIt;
      using const_iterator = SelectIt;
   };
   template< typename V2, typename F >
   Shim< details::HiShim11< SelectShim< V2, F > > > Select( F&& f )
   {
      return Shim< details::HiShim11< SelectShim< V2, F > > >{ { { { *this }, std::forward< F >( f ) } } };
   }
   template< typename V2, typename F >
   const Shim< details::HiShim11< SelectShim< V2, F > > > Select( F&& f ) const
   {
      return Shim< details::HiShim11< SelectShim< V2, F > > >{ { { { *this }, std::forward< F >( f ) } } };
   }

   template< typename V2, typename F >
   struct SelectWhereShim : details::LoShim< P >
   {
      F _f;
      using baseShim = details::LoShim< P >;

      struct SelectWhereIt : details::LoIterator2< SelectWhereIt, typename baseShim::lo_iterator >
      {
         const SelectWhereShim* _o = nullptr;

         using base = details::LoIterator2< SelectWhereIt, typename baseShim::lo_iterator >;

         using value_type = V2;
         using pointer = std::add_pointer_t< value_type >;
         using reference = V2;

         using check_type = decltype( std::declval< F >()( *base::_i ) );

         reference operator*()
         {
            return *( details::unwrap_reference_t< check_type > )_o->_f( *base::_i );
         }
         reference operator*() const
         {
            return *( details::unwrap_reference_t< check_type > )_o->_f( *base::_i );
         }

         bool _init = _chk();
         SelectWhereIt& operator++()
         {
            base::operator++();
            _chk();
            return *this;
         }
         bool _chk()
         {
            for( ; base::_i != base::_e; ++base::_i )
            {
               if( !!( details::unwrap_reference_t< check_type > )_o->_f( *base::_i ) )
               {
                  break;
               }
            }
            return true;
         }
      };
      using iterator = SelectWhereIt;
      using const_iterator = SelectWhereIt;
   };
   template< typename V2, typename F >
   Shim< details::HiShim2< SelectWhereShim< V2, F > > > SelectWhere( F&& f )
   {
      return Shim< details::HiShim2< SelectWhereShim< V2, F > > >{ { { { *this }, std::forward< F >( f ) } } };
   }
   template< typename V2, typename F >
   const Shim< details::HiShim2< SelectWhereShim< V2, F > > > SelectWhere( F&& f ) const
   {
      return Shim< details::HiShim2< SelectWhereShim< V2, F > > >{ { { { *this }, std::forward< F >( f ) } } };
   }

   template< typename V2, typename F >
   struct SelectManyShim : details::LoShim< P >
   {
      F _f;
      using baseShim = details::LoShim< P >;

      struct SelectManyIt : details::LoIterator2< SelectManyIt, typename baseShim::lo_iterator >
      {
         using base = details::LoIterator2< SelectManyIt, typename baseShim::lo_iterator >;
         using P2 = details::unwrap_reference_t< std::result_of_t< F( typename std::iterator_traits< typename baseShim::lo_iterator >::reference ) > >;
         const SelectManyShim* _o = nullptr;
         mutable details::optional< P2 > _p2;

         using I2 = decltype( std::begin( _p2.value() ) );

         using value_type = V2;
         using pointer = std::add_pointer_t< value_type >;
         using reference = V2;

         mutable details::optional< I2 > _i2;
         mutable details::optional< I2 > _e2;

         void _ensh() const
         {
            if( !_i2 )
            {
               const_cast< SelectManyIt* >( this )->_chk();
            }
         }
         SelectManyIt& operator++()
         {
            _ensh();
            ++( _i2.value() );
            _chk();
            return *this;
         }
         reference operator*()
         {
            _ensh();
            return *( _i2.value() );
         }
         reference operator*() const
         {
            _ensh();
            return *( _i2.value() );
         }
         bool operator==( const SelectManyIt& i ) const
         {
            _ensh();
            return this->_i == i._i;
         }
         void _chk()
         {
            for( ; this->_i != this->_e && _i2 == _e2; )
            {
               if( _i2 )
               {
                  ++this->_i;
                  if( this->_i == this->_e )
                  {
                     break;
                  }
               }
               _p2.emplace( ( details::unwrap_reference_t< P2 > )_o->_f( *this->_i ) );
               _i2.emplace( std::begin( _p2.value() ) );
               _e2.emplace( std::end( _p2.value() ) );
            }
         }
      };

      using iterator = SelectManyIt;
      using const_iterator = SelectManyIt;
   };
   template< typename V2, typename F >
   Shim< details::HiShim2< SelectManyShim< V2, F > > > SelectMany( F&& f )
   {
      return Shim< details::HiShim2< SelectManyShim< V2, F > > >{ { { { *this }, std::forward< F >( f ) } } };
   }
   template< typename V2, typename F >
   const Shim< details::HiShim2< SelectManyShim< V2, F > > > SelectMany( F&& f ) const
   {
      return Shim< details::HiShim2< SelectManyShim< V2, F > > >{ { { { *this }, std::forward< F >( f ) } } };
   }

   template< typename P2 >
   struct ConcatShim : details::LoShim< P >
   {
      P2 _p2;
      using P2Type = std::remove_reference_t< P2 >;
      using baseShim = details::LoShim< P >;

      struct ConcatIt : details::LoIterator2< ConcatIt, typename baseShim::lo_iterator >
      {
         using base = details::LoIterator2< ConcatIt, typename baseShim::lo_iterator >;

         using I2 = decltype( std::begin( _p2 ) );

         I2 _i2;
         bool _f = base::_i != base::_e;
         bool operator!=( const ConcatIt& i ) const
         {
            return !( *this == i );
         }
         bool operator==( const ConcatIt& i ) const
         {
            return _f == i._f && ( _f ? base::_i == i._i : _i2 == i._i2 );
         }
         ConcatIt& operator++()
         {
            if( _f && ++base::_i != base::_e )
            {
               return *this;
            }
            if( !_f )
            {
               ++_i2;
            }
            _f = false;
            return *this;
         }

         using V1 = typename base::reference;
         using V2 = typename std::iterator_traits< decltype( std::begin( _p2 ) ) >::reference;

         using reference = std::conditional_t<
            !std::is_reference< V1 >::value, V1,
            std::conditional_t< !std::is_reference< V2 >::value, V2,
                                std::conditional_t< std::is_const<
                                                       std::remove_reference_t< V1 > >::value,
                                                    V1, V2 > > >;

         reference operator*()
         {
            if( _f )
            {
               return *base::_i;
            }
            return *_i2;
         }
         reference operator*() const
         {
            if( _f )
            {
               return *base::_i;
            }
            return *_i2;
         }
      };

      using iterator = ConcatIt;
      using const_iterator = ConcatIt;

      size_t get_capacity() const
      {
         return baseShim::get_capacity() + From( _p2 ).get_capacity();
      }
      iterator begin()
      {
         return { { { { std::begin( baseShim::_p ) } }, std::end( baseShim::_p ) }, std::begin( _p2 ) };
      }
      iterator end()
      {
         return { { { { std::end( baseShim::_p ) } }, std::end( baseShim::_p ) }, std::end( _p2 ), false };
      }
      const_iterator begin() const
      {
         return { { { { std::begin( const_cast< ConcatShim* >( this )->_p ) } }, std::end( const_cast< ConcatShim* >( this )->_p ) }, std::begin( const_cast< ConcatShim* >( this )->_p2 ) };
      }
      const_iterator end() const
      {
         return { { { { std::end( const_cast< ConcatShim* >( this )->_p ) } }, std::end( const_cast< ConcatShim* >( this )->_p ) }, std::end( const_cast< ConcatShim* >( this )->_p2 ), false };
      }
   };
   template< typename P2 >
   Shim< ConcatShim< P2 > > Concat( P2&& p )
   {
      return Shim< ConcatShim< P2 > >{ { { *this }, std::forward< P2 >( p ) } };
   }
   template< typename P2 >
   const Shim< ConcatShim< P2 > > Concat( P2&& p ) const
   {
      return Shim< ConcatShim< P2 > >{ { { *this }, std::forward< P2 >( p ) } };
   }

   template< typename F, typename SetType >
   struct ExcludeShim : details::LoShim< P >
   {
      using baseShim = details::LoShim< P >;
      SetType _set;
      F _f;

      struct ExcludeIt : details::LoIterator2< ExcludeIt, typename baseShim::lo_iterator >
      {
         using base = details::LoIterator2< ExcludeIt, typename baseShim::lo_iterator >;
         const ExcludeShim* _o = nullptr;
         bool _init = _chk();
         ExcludeIt& operator++()
         {
            ++base::_i;
            _chk();
            return *this;
         }
         bool _chk()
         {
            for( ; base::_i != base::_e && _o->_set.find( _o->_f( *base::_i ) ) != std::end( _o->_set ); ++base::_i )
            {
            }
            return true;
         }
      };

      using iterator = ExcludeIt;
      using const_iterator = ExcludeIt;
   };
   template< typename P2, typename F >
   auto Exclude( P2&& p, F&& f ) -> Shim< details::HiShim2< ExcludeShim< F, decltype( From( p ).ToUnorderedSet() ) > > >
   {
      return Shim< details::HiShim2< ExcludeShim< F, decltype( From( p ).ToUnorderedSet() ) > > >{ { { { *this }, From( p ).ToUnorderedSet(), std::forward< F >( f ) } } };
   }
   template< typename P2, typename F >
   auto Exclude( P2&& p, F&& f ) const -> const Shim< details::HiShim2< ExcludeShim< F, decltype( From( p ).ToUnorderedSet() ) > > >
   {
      return Shim< details::HiShim2< ExcludeShim< F, decltype( From( p ).ToUnorderedSet() ) > > >{ { { { *this }, From( p ).ToUnorderedSet(), std::forward< F >( f ) } } };
   }
   struct _ExcludeFunctor
   {
      typename const_iterator::reference operator()( typename const_iterator::reference v ) const
      {
         return v;
      }
   };
   template< typename P2 >
   auto Exclude( P2&& p ) -> Shim< details::HiShim2< ExcludeShim< _ExcludeFunctor, decltype( From( p ).ToUnorderedSet() ) > > >
   {
      return Exclude( p, _ExcludeFunctor{} );
   }
   template< typename P2 >
   auto Exclude( P2&& p ) const -> const Shim< details::HiShim2< ExcludeShim< _ExcludeFunctor, decltype( From( p ).ToUnorderedSet() ) > > >
   {
      return Exclude( p, _ExcludeFunctor{} );
   }

   template< typename F, typename SetType >
   struct IntersectShim : details::LoShim< P >
   {
      using baseShim = details::LoShim< P >;
      SetType _set;
      F _f;

      struct IntersecIt : details::LoIterator2< IntersecIt, typename baseShim::lo_iterator >
      {
         using base = details::LoIterator2< IntersecIt, typename baseShim::lo_iterator >;
         const IntersectShim* _o = nullptr;
         bool _init = _chk();
         IntersecIt& operator++()
         {
            base::operator++();
            _chk();
            return *this;
         }
         bool _chk()
         {
            for( ; base::_i != base::_e && _o->_set.find( _o->_f( *base::_i ) ) == std::end( _o->_set ); ++base::_i )
            {
            }
            return true;
         }
      };

      using iterator = IntersecIt;
      using const_iterator = IntersecIt;
   };
   template< typename P2, typename F >
   auto Intersect( P2&& p, F&& f ) -> Shim< details::HiShim2< IntersectShim< F, decltype( From( p ).ToUnorderedSet() ) > > >
   {
      return Shim< details::HiShim2< IntersectShim< F, decltype( From( p ).ToUnorderedSet() ) > > >{ { { { *this }, From( p ).ToUnorderedSet(), std::forward< F >( f ) } } };
   }
   template< typename P2, typename F >
   auto Intersect( P2&& p, F&& f ) const -> const Shim< details::HiShim2< IntersectShim< F, decltype( From( p ).ToUnorderedSet() ) > > >
   {
      return Shim< details::HiShim2< IntersectShim< F, decltype( From( p ).ToUnorderedSet() ) > > >{ { { { *this }, From( p ).ToUnorderedSet(), std::forward< F >( f ) } } };
   }
   struct _IntersectFunctor
   {
      typename const_iterator::reference operator()( typename const_iterator::reference v ) const
      {
         return v;
      }
   };
   template< typename P2 >
   auto Intersect( P2&& p ) -> Shim< details::HiShim2< IntersectShim< _IntersectFunctor, decltype( From( p ).ToUnorderedSet() ) > > >
   {
      return Intersect( std::forward< P2 >( p ), _IntersectFunctor{} );
   }
   template< typename P2 >
   auto Intersect( P2&& p ) const -> const Shim< details::HiShim2< IntersectShim< _IntersectFunctor, decltype( From( p ).ToUnorderedSet() ) > > >
   {
      return Intersect( std::forward< P2 >( p ), _IntersectFunctor{} );
   }

   template< typename V2 >
   struct CastShim : details::LoShim< P >
   {
      using baseShim = details::LoShim< P >;

      struct CastIt : details::LoIterator1< CastIt, typename baseShim::lo_iterator >
      {
         using base = details::LoIterator1< CastIt, typename baseShim::lo_iterator >;

         using value_type = V2;
         using pointer = std::add_pointer_t< value_type >;
         using reference = V2;

         reference operator*()
         {
            return static_cast< reference >( base::operator*() );
         }
         reference operator*() const
         {
            return static_cast< reference >( base::operator*() );
         }
      };

      using iterator = CastIt;
      using const_iterator = CastIt;
   };
   template< typename V2 >
   Shim< details::HiShim1< CastShim< V2 > > > Cast()
   {
      return Shim< details::HiShim1< CastShim< V2 > > >{ { { { *this } } } };
   }
   template< typename V2 >
   const Shim< details::HiShim1< CastShim< V2 > > > Cast() const
   {
      return Shim< details::HiShim1< CastShim< V2 > > >{ { { { *this } } } };
   }

   struct TakeShim : details::LoShim< P >
   {
      using baseShim = details::LoShim< P >;
      size_t _v = 0;

      struct TakeIt : details::LoIterator2< TakeIt, typename baseShim::lo_iterator >
      {
         const TakeShim* _o = nullptr;
         size_t _v = _o->_v;
         using base = details::LoIterator2< TakeIt, typename baseShim::lo_iterator >;

         TakeIt& operator++()
         {
            --_v;
            return base::operator++();
         }
         bool operator==( const TakeIt& i ) const
         {
            return ( _v <= 0 && i._i == base::_e ) ? true : base::operator==( i );
         }
      };

      using iterator = TakeIt;
      using const_iterator = TakeIt;
   };
   Shim< details::HiShim2< TakeShim > > Take( size_t value )
   {
      return Shim< details::HiShim2< TakeShim > >{ { { { *this }, value } } };
   }
   const Shim< details::HiShim2< TakeShim > > Take( size_t value ) const
   {
      return Shim< details::HiShim2< TakeShim > >{ { { { *this }, value } } };
   }

   struct ThrottleShim : details::LoShim< P >
   {
      using baseShim = details::LoShim< P >;
      size_t _v = 0;

      struct ThrottleIt : details::LoIterator2< ThrottleIt, typename baseShim::lo_iterator >
      {
         using base = details::LoIterator2< ThrottleIt, typename baseShim::lo_iterator >;
         const ThrottleShim* _o = nullptr;
         ThrottleIt& operator++()
         {
            for( auto v = _o->_v; base::_i != base::_e && v > 0; ++base::_i, --v )
            {
            }
            return *this;
         }

         auto operator*()
         {
            return From( base::_i, base::_e, _o->_v ).Take( _o->_v );
         }
         auto operator*() const
         {
            return From( base::_i, base::_e, _o->_v ).Take( _o->_v );
         }
      };

      using iterator = ThrottleIt;
      using const_iterator = ThrottleIt;
   };
   Shim< details::HiShim2< ThrottleShim > > Throttle( size_t value )
   {
      return Shim< details::HiShim2< ThrottleShim > >{ { { { *this }, value } } };
   }
   const Shim< details::HiShim2< ThrottleShim > > Throttle( size_t value ) const
   {
      return Shim< details::HiShim2< ThrottleShim > >{ { { { *this }, value } } };
   }

   using reference = typename std::iterator_traits< iterator >::reference;
   using value_type = typename std::iterator_traits< iterator >::value_type;
   using const_reference = typename std::iterator_traits< const_iterator >::reference;
   using const_value_type = typename std::iterator_traits< const_iterator >::value_type;

   template< typename F >
   struct DistinctShim : details::LoShim< P >
   {
      using baseShim = details::LoShim< P >;

      F _f;

      struct DistinctIt : details::LoIterator2< DistinctIt, typename baseShim::lo_iterator >
      {
         const DistinctShim* _o = nullptr;
         using base = details::LoIterator2< DistinctIt, typename baseShim::lo_iterator >;

         std::unordered_set< std::result_of_t< F( typename base::reference ) > > _set;

         DistinctIt& operator++()
         {
            if( _set.empty() )
            {
               _set.insert( std::move( _o->_f( *base::_i ) ) );
            }

            for( base::operator++(); base::_i != base::_e; base::operator++() )
            {
               if( _set.insert( std::move( _o->_f( *base::_i ) ) ).second )
               {
                  break;
               }
            }

            return *this;
         }
      };

      using iterator = DistinctIt;
      using const_iterator = DistinctIt;
   };
   template< typename F >
   Shim< details::HiShim2< DistinctShim< F > > > Distinct( F&& f )
   {
      return Shim< details::HiShim2< DistinctShim< F > > >{ { { { *this }, std::forward< F >( f ) } } };
   }
   template< typename F >
   const Shim< details::HiShim2< DistinctShim< F > > > Distinct( F&& f ) const
   {
      return Shim< details::HiShim2< DistinctShim< F > > >{ { { { *this }, std::forward< F >( f ) } } };
   }
   struct _DistinctFunctor
   {
      const_value_type operator()( const_reference v ) const
      {
         return v;
      }
   };
   Shim< details::HiShim2< DistinctShim< _DistinctFunctor > > > Distinct()
   {
      return Distinct( _DistinctFunctor{} );
   }
   const Shim< details::HiShim2< DistinctShim< _DistinctFunctor > > > Distinct() const
   {
      return Distinct( _DistinctFunctor{} );
   }

   struct VarShim : details::LoShim< P >
   {
      using baseShim = details::LoShim< P >;

      struct VarIt : details::LoIterator1< VarIt, typename baseShim::lo_iterator >
      {
         using base = details::LoIterator1< VarIt, typename baseShim::lo_iterator >;

         using typename base::reference;

         const VarShim* _o = nullptr;
         mutable details::optional< typename base::value_type > _v;
         VarIt& operator++()
         {
            base::operator++();
            _v.reset();
            return *this;
         }
         reference operator*()
         {
            if( !_v.is_initialized() )
            {
               _v.emplace( base::operator*() );
            }
            return _v.value();
         }
         reference operator*() const
         {
            if( !_v.is_initialized() )
            {
               _v.emplace( base::operator*() );
            }
            return _v.value();
         }
      };

      using iterator = VarIt;
      using const_iterator = VarIt;
   };
   Shim< details::HiShim1< VarShim > > Var()
   {
      return Shim< details::HiShim1< VarShim > >{ { { { *this } } } };
   }
   const Shim< details::HiShim1< VarShim > > Var() const
   {
      return Shim< details::HiShim1< VarShim > >{ { { { *this } } } };
   }

   struct _SkipFunctor
   {
      mutable size_t _c;
      bool operator()( const_reference ) const
      {
         if( _c == 0 )
         {
            return true;
         }
         --_c;
         return false;
      }
   };
   const Shim< details::HiShim2< WhereShim< _SkipFunctor > > > Skip( size_t c ) const
   {
      return this->Where( _SkipFunctor{ c } );
   }

   template< typename VT, typename T >
   static auto FirstOrNone( T* self )
   {
      auto ret = std::begin( *self );
      if( ret == std::end( *self ) )
      {
         return details::optional< VT >{};
      }
      return details::optional< VT >{ *ret };
   }

   template< typename VT = value_type >
   auto FirstOrNone()
   {
      return this->template FirstOrNone< VT >( this );
   }

   template< typename VT = const_value_type >
   auto FirstOrNone() const
   {
      return this->template FirstOrNone< VT >( this );
   }

   template< typename VT = value_type, typename F >
   auto FirstOrNone( F&& f )
   {
      auto self = this;
      return self->template Where( std::forward< F >( f ) ).template FirstOrNone< VT >();
   }

   template< typename VT = const_value_type, typename F >
   auto FirstOrNone( F&& f ) const
   {
      auto self = this;
      return self->template Where( std::forward< F >( f ) ).template FirstOrNone< VT >();
   }

   template< typename VT, typename T >
   static VT First( T* self ) noexcept( false )
   {
      auto ret = self->template FirstOrNone< VT >();
      if( !ret )
      {
         throw std::out_of_range( "The element isn't found." );
      }
      return *ret;
   }

   reference First() noexcept( false )
   {
      return this->First< reference >( this );
   }

   const_reference First() const noexcept( false )
   {
      return this->First< const_reference >( this );
   }

   template< typename F >
   reference First( F&& f ) noexcept( false )
   {
      auto self = this;
      return self->template Where( std::forward< F >( f ) ).First();
   }

   template< typename F >
   const_reference First( F&& f ) const noexcept( false )
   {
      auto self = this;
      return self->template Where( std::forward< F >( f ) ).First();
   }

   template< typename VT, typename T >
   static auto LastOrNone( T* self )
   {
      auto ret = std::begin( *self );
      for( auto it = ret; it != std::end( *self ); ++it )
      {
         ret = it;
      }
      if( ret == std::end( *self ) )
      {
         return details::optional< VT >();
      }
      return details::optional< VT >{ *ret };
   }

   template< typename VT = value_type >
   auto LastOrNone()
   {
      return this->template LastOrNone< VT >( this );
   }

   template< typename VT = const_value_type >
   auto LastOrNone() const
   {
      return this->template LastOrNone< VT >( this );
   }

   template< typename VT = value_type, typename F >
   auto LastOrNone( F&& f )
   {
      auto self = this;
      return self->template Where( std::forward< F >( f ) ).template LastOrNone< VT >();
   }

   template< typename VT = const_value_type, typename F >
   auto LastOrNone( F&& f ) const
   {
      auto self = this;
      return self->template Where( std::forward< F >( f ) ).template LastOrNone< VT >();
   }

   template< typename VT, typename T >
   static VT Last( T* self ) noexcept( false )
   {
      auto ret = self->template LastOrNone< VT >();
      if( !ret )
      {
         throw std::out_of_range( "The element isn't found." );
      }
      return *ret;
   }

   reference Last() noexcept( false )
   {
      return this->Last< reference >( this );
   }

   const_reference Last() const noexcept( false )
   {
      return this->Last< const_reference >( this );
   }

   template< typename F >
   reference Last( F&& f ) noexcept( false )
   {
      auto self = this;
      return self->template Where( std::forward< F >( f ) ).Last();
   }

   template< typename F >
   const_reference Last( F&& f ) const noexcept( false )
   {
      auto self = this;
      return self->template Where( std::forward< F >( f ) ).Last();
   }

   template< typename T >
   static size_t Count( T* self )
   {
      size_t ret = 0;
      for( const auto& it : *self )
      {
         ( void )it;
         ++ret;
      }
      return ret;
   }
   size_t Count()
   {
      return Count( this );
   }
   size_t Count() const
   {
      return Count( this );
   }

   template< typename VT, typename T >
   static auto SumOrNone( T* self )
   {
      details::optional< VT > ret;
      for( auto&& it : *self )
      {
         if( !ret.is_initialized() )
         {
            ret = VT{};
         }
         ret = ( VT )( ret.value() + it );
      }
      return ret;
   }

   auto Sum() const
   {
      return SumOrNone< std::decay_t< value_type > >( this ).value_or( std::decay_t< value_type >{} );
   }

   auto SumOrNone() const
   {
      return SumOrNone< std::decay_t< value_type > >( this );
   }

   template< typename VT = const_value_type >
   auto MinOrNone() const
   {
      auto self = this;
      details::optional< VT > ret;

      for( const auto& it : *self )
      {
         if( !ret || *ret > it )
         {
            ret = it;
         }
      }
      return ret;
   }

   const_reference Min() const noexcept( false )
   {
      auto ret = MinOrNone< const_reference >();
      if( !ret )
      {
         throw std::range_error( "Sequence contains no elements." );
      }
      return *ret;
   }

   template< typename VT = const_value_type >
   auto MaxOrNone() const
   {
      auto self = this;
      details::optional< VT > ret;

      for( const auto& it : *self )
      {
         if( !ret || *ret < it )
         {
            ret = it;
         }
      }
      return ret;
   }

   const_reference Max() const noexcept( false )
   {
      auto ret = MaxOrNone< const_reference >();
      if( !ret )
      {
         throw std::range_error( "Sequence contains no elements." );
      }
      return *ret;
   }

   bool Any() const
   {
      auto self = this;
      for( const auto& it : *self )
      {
         ( void )it;
         return true;
      }
      return false;
   }

   template< typename F >
   bool Any( F&& f ) const
   {
      auto self = this;
      return self->Where( std::forward< F >( f ) ).Any();
   }

   bool Contains( const_reference t ) const
   {
      auto self = this;
      for( const auto& it : *self )
      {
         if( it == t )
         {
            return true;
         }
      }
      return false;
   }

   template< typename C >
   bool IsIntersect( const C& c ) const
   {
      auto self = this;
      for( const auto& it1 : *self )
      {
         for( const auto& it2 : c )
         {
            if( it1 == it2 )
            {
               return true;
            }
         }
      }
      return false;
   }

   template< typename F >
   bool All( const F& f ) const
   {
      auto self = this;
      for( const auto& it : *self )
      {
         if( !f( it ) )
         {
            return false;
         }
      }
      return true;
   }

   template< typename A, typename F >
   A Aggregate( A a, const F& f ) const
   {
      auto self = this;
      for( const auto& it : *self )
      {
         a = f( a, it );
      }
      return a;
   }

   template< typename T, typename O >
   static O Copy( T* self, O dst )
   {
      return std::copy( std::begin( *self ), std::end( *self ), dst );
   }

   template< typename O >
   O Copy( O dst )
   {
      return Copy( this, dst );
   }
   template< typename O >
   O Copy( O dst ) const
   {
      return Copy( this, dst );
   }

   template< typename T >
   static void ToList( T* self, std::list< value_type >& l )
   {
      Copy( self, std::inserter( l, std::begin( l ) ) );
   }

   auto ToList()
   {
      std::list< value_type > ret;
      ToList( this, ret );
      return ret;
   }

   auto ToList() const
   {
      std::list< const_value_type > ret;
      ToList( this, ret );
      return ret;
   }

   template< typename T >
   static void ToVector( T* self, std::vector< value_type >& v )
   {
      v.reserve( self->get_capacity() );
      Copy( self, std::inserter( v, std::begin( v ) ) );
   }

   void ToVector( std::vector< value_type >& v )
   {
      ToVector( this, v );
   }
   void ToVector( std::vector< const_value_type >& v ) const
   {
      ToVector( this, v );
   }

   auto ToVector()
   {
      std::vector< value_type > ret;
      ToVector( ret );
      return ret;
   }

   auto ToVector() const
   {
      std::vector< const_value_type > ret;
      ToVector( ret );
      return ret;
   }

   auto ToVector( size_t n )
   {
      std::vector< value_type > ret;
      ret.reserve( n );
      Copy( std::inserter( ret, std::begin( ret ) ) );
      return ret;
   }

   auto ToVector( size_t n ) const
   {
      std::vector< const_value_type > ret;
      ret.reserve( n );
      Copy( std::inserter( ret, std::begin( ret ) ) );
      return ret;
   }

   auto ToOrderedVector() const
   {
      auto self = this;
      auto ret = self->ToVector();
      std::sort( std::begin( ret ), std::end( ret ) );
      return ret;
   }

   template< typename F >
   auto ToOrderedVector( const F& f ) const
   {
      auto self = this;
      auto ret = self->ToVector();
      std::sort( std::begin( ret ), std::end( ret ), f );
      return ret;
   }

   template< typename K, typename V2, typename KS, typename VS >
   auto ToUnorderedMap( const KS& keySelector, const VS& valueSelector )
   {
      std::unordered_map< K, V2 > ret;
      auto self = this;
      ret.reserve( self->get_capacity() );
      for( auto&& it : *self )
      {
         valueSelector( it, ret[ keySelector( std::as_const( it ) ) ] );
      }
      return ret;
   }

   template< typename K, typename KS >
   auto ToUnorderedMap( const KS& keySelector )
   {
      return ToUnorderedMap< K, value_type >( keySelector, []( const_reference v1, reference v2 ) { v2 = v1; } );
   }

   template< typename K, typename V2, typename KS, typename VS >
   auto ToUnorderedMap( const KS& keySelector, const VS& valueSelector ) const
   {
      std::unordered_map< K, V2 > ret;
      auto self = this;
      ret.reserve( self->get_capacity() );
      for( auto&& it : *self )
      {
         valueSelector( it, ret[ keySelector( std::as_const( it ) ) ] );
      }
      return ret;
   }

   template< typename K, typename KS >
   auto ToUnorderedMap( const KS& keySelector ) const
   {
      return ToUnorderedMap< K, const_value_type >( keySelector, []( const_reference v1, reference v2 ) { v2 = v1; } );
   }

   auto ToUnorderedSet()
   {
      std::unordered_set< value_type > ret;
      auto self = this;
      ret.reserve( self->get_capacity() );
      for( auto&& it : *self )
      {
         ret.emplace( it );
      }
      return ret;
   }

   template< typename K, typename F >
   auto ToUnorderedSet( F&& f )
   {
      auto self = this;
      return self->template Select< K >( std::forward< F >( f ) ).ToUnorderedSet();
   }

   auto ToUnorderedSet() const
   {
      std::unordered_set< const_value_type > ret;
      auto self = this;
      for( auto&& it : *self )
      {
         ret.emplace( it );
      }
      return ret;
   }

   template< typename K, typename F >
   auto ToUnorderedSet( F&& f ) const
   {
      auto self = this;
      return self->template Select< K >( std::forward< F >( f ) ).ToUnorderedSet();
   }

   struct _MoveFunctor
   {
      template< typename VT >
      value_type operator()( VT&& v )
      {
         return std::move( v );
      }

      template< typename VT >
      const_value_type operator()( VT&& v ) const
      {
         return std::move( v );
      }
   };

   auto Move()
   {
      return Shim< details::HiShim11< SelectShim< value_type, _MoveFunctor > > >{ { { { *this }, _MoveFunctor{} } } };
   }

   std::shared_ptr< ITearOffContainer< value_type > > AsTearOffContainer();
   std::shared_ptr< ITearOffContainer< const_value_type > > AsTearOffContainer() const;
};

template< typename P >
struct _shared_getter
{
   std::shared_ptr< P > _p;

   using iterator = typename P::iterator;
   using const_iterator = typename P::const_iterator;

   constexpr static bool shared = true;

   explicit _shared_getter( P&& p )
      : _p{ std::make_shared< P >( std::forward< P >( p ) ) }
   {
   }
   _shared_getter( _shared_getter&& ) = default;
   _shared_getter( const _shared_getter& ) = default;
   _shared_getter& operator=( _shared_getter&& ) = default;
   _shared_getter& operator=( const _shared_getter& ) = default;

   size_t get_capacity() const
   {
      return details::get_capacity( *_p );
   }
   auto end()
   {
      return std::end( *_p );
   }
   auto begin()
   {
      return std::begin( *_p );
   }
   auto end() const
   {
      return std::end( *_p );
   }
   auto begin() const
   {
      return std::begin( *_p );
   }
};

template< typename P >
struct _shared_getter< P& >
{
   P* _p = nullptr;

   using iterator = typename P::iterator;
   using const_iterator = typename P::const_iterator;

   constexpr static bool shared = false;

   explicit _shared_getter( P& p )
      : _p( &p )
   {
   }

   ~_shared_getter()
   {
      _p = nullptr;
   }

   _shared_getter( _shared_getter&& ) = default;
   _shared_getter( const _shared_getter& ) = default;
   _shared_getter& operator=( _shared_getter&& ) = default;
   _shared_getter& operator=( const _shared_getter& ) = default;

   size_t get_capacity() const
   {
      return details::get_capacity( *_p );
   }
   auto end()
   {
      return std::end( *_p );
   }
   auto begin()
   {
      return std::begin( *_p );
   }
   auto end() const
   {
      return std::end( *_p );
   }
   auto begin() const
   {
      return std::begin( *_p );
   }
};

template< typename P >
struct StdShim : Shim< _shared_getter< P > >
{
};

template< typename P >
struct StdConstShim : Shim< const _shared_getter< P > >
{
};

template< typename I >
struct ItShim
{
   using iterator = std::remove_reference_t< I >;
   using const_iterator = iterator;

   iterator _b;
   iterator _e;
   size_t _capacity;

   iterator end()
   {
      return _e;
   }
   iterator begin()
   {
      return _b;
   }
   const_iterator end() const
   {
      return _e;
   }
   const_iterator begin() const
   {
      return _b;
   }
   size_t get_capacity() const
   {
      return _capacity;
   }
};

// Do not use. Used only for tests!!!
template< typename T >
StdShim< std::vector< T > > Move( const std::initializer_list< T >& t )
{
   std::vector< T > vec( t.size() );
   std::move( const_cast< T* >( std::begin( t ) ), const_cast< T* >( std::end( t ) ), std::begin( vec ) );
   return linq::From( std::move( vec ) );
}
} // namespace linq
