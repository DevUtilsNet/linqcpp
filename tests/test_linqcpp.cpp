#include <optional>

#include <linqcpp/linqcpp.h>

namespace linq
{
BOOST_AUTO_TEST_SUITE( linqcpp )
namespace test
{
namespace
{
template< typename T >
std::vector< T > MoveToVector( const std::initializer_list< T >& t )
{
   std::vector< T > vec( t.size() );
   std::move( const_cast< T* >( std::begin( t ) ), const_cast< T* >( std::end( t ) ), std::begin( vec ) );
   return vec;
}
} // namespace

BOOST_AUTO_TEST_CASE( Iterator )
{
   {
      const std::vector vector{ 1 };
      auto iterator = d::MakeIterator( d::MakeIterator( vector.begin(), vector.end() ) );
      static_assert( std::is_same_v< decltype( *vector.begin() ), decltype( *iterator ) > );
   }

   {
      std::vector vector{ 1, 2 };
      auto iterator = d::MakeIterator( vector.begin(), vector.end() );
      auto begin = d::MakeIterator( iterator );
      auto end = d::MakeEndIterator< decltype( iterator ) >();
      BOOST_TEST_REQUIRE( *begin == 1 );
      BOOST_TEST_REQUIRE( ( begin != end ) );

      BOOST_TEST_REQUIRE( *begin == 1 );
      BOOST_TEST_REQUIRE( *++begin == 2 );
      BOOST_TEST_REQUIRE( *begin == 2 );
      BOOST_TEST_REQUIRE( ( ++begin == end ) );
      BOOST_TEST_REQUIRE( ( end == begin ) );

      iterator = d::MakeIterator( vector.begin(), vector.end() );
      begin = ++d::MakeIterator( iterator );
      auto begin2 = ++d::MakeIterator( iterator );

      BOOST_TEST_REQUIRE( ( begin == begin2 ) );
   }

   {
      std::vector vector{ 1 };
      auto iterator = d::MakeIterator( d::MakeIterator( vector.begin(), vector.end() ) );
      *iterator = 2;
      BOOST_TEST_REQUIRE( vector.at( 0 ) == 2 );
   }

   {
      std::vector vector{ 1, 2, 3, 4 };
      auto iterator = d::MakeIterator( vector.begin(), vector.end() );
      auto collection = From( d::MakeIterator( iterator ), d::MakeEndIterator< decltype( iterator ) >(), 4 );
      BOOST_REQUIRE_EQUAL_COLLECTIONS( vector.begin(), vector.end(), collection.begin(), collection.end() );
   }
}

BOOST_AUTO_TEST_CASE( IteratorShim )
{
   {
      const std::vector vector{ 1 };
      auto iterator = d::MakeIterator( d::MakeIteratorShim< d::optional< int > >(
         d::MakeIterator( vector.begin(), vector.end() ),
         []( auto&& m ) { return d::optional< int >{ m.Next().value() + 1 }; } ) );
      BOOST_TEST_REQUIRE( *iterator == 2 );
   }

   {
      std::vector vector{ 1 };
      auto iterator = d::MakeIterator( d::MakeIteratorShim< d::optional< int& > >(
         d::MakeIterator( vector.begin(), vector.end() ),
         []( auto&& m ) { return d::optional< int& >{ m.Next().value() }; } ) );
      BOOST_TEST_REQUIRE( *iterator == 1 );
      *iterator = 2;
      BOOST_TEST_REQUIRE( vector.at( 0 ) == 2 );
   }
}

BOOST_AUTO_TEST_CASE( StdShim )
{
   {
      std::vector< int > vector{ 1, 2, 3, 4, 5 };
      auto collection = From( vector );
      BOOST_REQUIRE_EQUAL_COLLECTIONS( vector.begin(), vector.end(), collection.begin(), collection.end() );
   }

   {
      const std::vector< int > vector{ 1, 2, 3, 4, 5 };
      auto collection = From( vector );
      std::vector< int > vector2{ 1, 2, 3, 4, 5 };
      BOOST_REQUIRE_EQUAL_COLLECTIONS( collection.begin(), collection.end(), vector2.begin(), vector2.end() );
   }
}

BOOST_AUTO_TEST_CASE( ToVector )
{
   {
      std::vector< std::string > vector{ "1", "2", "3", "4", "5" };
      auto collection = From( vector ).Where( []( auto&& m ) { return !m.empty(); } ).ToVector();
      BOOST_REQUIRE_EQUAL_COLLECTIONS( collection.begin(), collection.end(), vector.begin(), vector.end() );
   }
   {
      const std::vector< int > vector;
      static_assert( std::is_same_v< decltype( From( vector ).mShim.CreateIterator().Next().value() ), const int& > );
      const std::vector< int >& vector2 = vector;
      static_assert( std::is_same_v< decltype( From( vector2 ).mShim.CreateIterator().Next().value() ), const int& > );
   }
}

BOOST_AUTO_TEST_CASE( LRValue )
{
   {
      auto vector{ MoveToVector( { std::make_unique< int >( 1 ) } ) };
      BOOST_TEST_REQUIRE( *vector.at( 0 ) == 1 );
      BOOST_TEST_REQUIRE( *From( vector ).Where( []( auto&& ) { return true; } ).mShim.CreateIterator().Next().value() == 1 );
      *From( vector ).Where( []( auto&& ) { return true; } ).mShim.CreateIterator().Next().value() = 2;
      BOOST_TEST_REQUIRE( *vector.at( 0 ) == 2 );

      auto from = From( vector );
      auto where = from.Select< std::unique_ptr< int >& >( []( std::unique_ptr< int >& m ) { return std::ref( m ); } );
      BOOST_TEST_REQUIRE( *where.mShim.CreateIterator().Next().value() == 2 );
      *where.mShim.CreateIterator().Next().value() = 3;
      BOOST_TEST_REQUIRE( *vector.at( 0 ) == 3 );
   }

   {
      auto vector{ MoveToVector( { std::make_unique< int >( 1 ) } ) };
      BOOST_TEST_REQUIRE( *From( std::move( vector ) ).Where( []( auto&& ) { return true; } ).mShim.CreateIterator().Next().value() == 1 );
   }

   {
      auto vector{ MoveToVector( { std::make_unique< int >( 1 ) } ) };
      auto vector2 = From( std::move( vector ) ).Select< std::unique_ptr< int > >( []( std::unique_ptr< int >& m ) { return std::move( m ); } ).ToVector();
      BOOST_TEST_REQUIRE( vector2.size() == 1 );
      BOOST_TEST_REQUIRE( *vector2.at( 0 ) == 1 );
   }
}

BOOST_AUTO_TEST_CASE( FromTest )
{
   {
      auto collection = From( MoveToVector( { std::make_unique< int >( 1 ) } ) );
      auto collection2 = From( std::move( collection ) );
      collection = std::move( collection2 );
      BOOST_TEST_REQUIRE( collection.Count() == 1 );
      BOOST_TEST_REQUIRE( *collection.mShim.CreateIterator().Next().value() == 1 );
   }

   {
      auto collection = From( { 1 } );
      auto collection2 = From( collection );
      collection2 = collection;
      BOOST_TEST_REQUIRE( collection.Count() == 1 );
      BOOST_TEST_REQUIRE( collection.mShim.CreateIterator().Next().value() == 1 );
   }

   {
      From( From( MoveToVector( { std::make_unique< int >( 2 ) } ) ) );
   }

   {
      auto container = From( { 1, 2 } );
      auto it = container.begin();
      auto it2 = it;
      it2++;
      BOOST_TEST_REQUIRE( *it == 1 );
      BOOST_TEST_REQUIRE( *it2 == 2 );
   }
}

struct TestType
{
   int _t;

   TestType( int t )
      : _t( t )
   {
   }

   TestType() = default;

   TestType( TestType&& ) = default;

   TestType( const TestType& ) = delete;

   ~TestType()
   {
      _t = -1;
   }

   TestType& operator=( TestType&& ) = default;
};

struct TestType2
{
   int _t;

   TestType2( int t )
      : _t( t )
   {
   }

   TestType2() = default;

   TestType2( TestType2&& ) = default;

   TestType2( const TestType2& ) = default;

   ~TestType2()
   {
      _t = -1;
   }

   TestType2& operator=( TestType2&& ) = default;

   TestType2& operator=( const TestType2& ) = default;

   TestType2 operator+( const TestType2& t )
   {
      return TestType2( _t + t._t );
   }

   operator int() const
   {
      return _t;
   }
};

BOOST_AUTO_TEST_CASE( InitializerList )
{
   const auto from = From( { TestType2( 1 ), TestType2( 1 ) } );
   BOOST_REQUIRE_EQUAL( from.Sum(), 2 );
   BOOST_REQUIRE_EQUAL( From( { 1, 2, 3, 4, 5 } ).Count(), 5 );
}

BOOST_AUTO_TEST_CASE( Concat )
{
   {
      std::vector v1{ 1 };
      std::vector v2{ 1 };
      auto container = From( v1 ).Concat( v2 );
      auto it = container.begin();
      ( *it ) = 2;
      ++it;
      ( *it ) = 3;
      BOOST_TEST_REQUIRE( v1.at( 0 ) == 2 );
      BOOST_TEST_REQUIRE( v2.at( 0 ) == 3 );
   }

   BOOST_REQUIRE_EQUAL( From( { 1, 2, 3, 4, 5 } ).Concat( std::vector< int >() ).Count(), 5 );
   BOOST_REQUIRE_EQUAL( From( { 1, 2, 3, 4, 5 } ).Concat( From( { 1, 2, 3, 4, 5 } ) ).Count(), 10 );
   BOOST_REQUIRE_EQUAL( From( std::vector< int >() ).Concat( From( std::vector< int >( { 1, 2, 3, 4, 5 } ) ) ).Count(), 5 );

   {
      const std::vector v1{ 1 };
      std::vector v2{ 1 };
      auto container1 = From( v1 ).Concat( v2 );
      auto container2 = From( v2 ).Concat( v1 );

      BOOST_TEST_REQUIRE( *container1.begin() == 1 );
      BOOST_TEST_REQUIRE( *container2.begin() == 1 );
   }

   {
      std::vector v1{ MoveToVector( { std::make_unique< int >( 1 ) } ) };
      BOOST_TEST_REQUIRE( *From( v1 ).Concat( MoveToVector( { std::make_unique< int >( 2 ) } ) ).mShim.CreateIterator().Next().value() == 1 );
   }

   {
      std::vector v1{ MoveToVector( { std::make_unique< int >( 1 ) } ) };
      BOOST_TEST_REQUIRE( *From( v1 ).Concat( From( MoveToVector( { std::make_unique< int >( 2 ) } ) ) ).mShim.CreateIterator().Next().value() == 1 );
   }

   {
      From( { 1 } )
         .Concat(
            From( { 1 } )
               .SelectMany< int >( []( int ) {
                  return std::vector< int >{};
               } ) )
         .ToVector();
   }
}

BOOST_AUTO_TEST_CASE( Container )
{
   BOOST_REQUIRE_EQUAL( From( std::list< int >( { 1, 2, 3, 4, 5 } ) ).Count(), 5 );
   BOOST_REQUIRE_EQUAL( From( std::vector< int >( { 1, 2, 3, 4, 5 } ) ).Count(), 5 );
   BOOST_REQUIRE_EQUAL( From( From( std::vector< int >( { 1, 2, 3, 4, 5 } ) ) ).Count(), 5 );
}

BOOST_AUTO_TEST_CASE( Where )
{
   {
      std::vector< int > vector{ 1, 2, 3, 4, 5 };
      auto collection =
         From( vector )
            .Where( []( int m ) {
               return m != 2;
            } )
            .Where( []( int m ) {
               return m != 4;
            } );

      std::vector< int > vector2{ 1, 3, 5 };
      BOOST_REQUIRE_EQUAL_COLLECTIONS( collection.begin(), collection.end(), vector2.begin(), vector2.end() );
   }

   auto col11 =
      From( { 1, 2, 3, 4, 5 } )
         .Where( []( int i ) { return i > 3; } )
         .ToVector();
   auto col12 = { 4, 5 };
   BOOST_REQUIRE_EQUAL_COLLECTIONS( col11.begin(), col11.end(), col12.begin(), col12.end() );

   BOOST_REQUIRE_EQUAL(
      From( { 1, 2, 3, 4, 5 } )
         .Where( []( const int& i ) { return i > 3; } )
         .Count(),
      2 );

   const auto container1 = From( { TestType2( 1 ), TestType2( 2 ) } );
   const auto container = container1;
   auto iterator1 = container1.begin();
   auto iterator = std::move( iterator1 );

   BOOST_TEST_REQUIRE( ( iterator != container1.end() ) );

   BOOST_REQUIRE_EQUAL(
      container
         .Where( []( const TestType2& ) { return true; } )
         .Where( []( const TestType2& ) { return true; } )
         .Sum(),
      3 );

   {
      const std::vector< int > constVector = { 1, 2, 3 };
      const auto constCont =
         From( constVector )
            .Where( []( const int& m ) {
               return m == 2;
            } );
      BOOST_TEST_REQUIRE( constCont.Sum() == 2 );
      BOOST_TEST_REQUIRE( *constCont.begin() == 2 );
   }

   {
      const std::vector< int > constVector = { 1, 2, 3 };
      auto container = From( constVector ).Where( []( auto&& ) { return true; } );
      BOOST_TEST_REQUIRE( ( container.begin() != container.end() ) );
   }

   {
      auto functor = +[]( int ) { return true; };
      From( { 1 } ).Where( functor );
   }
}

BOOST_AUTO_TEST_CASE( Until )
{
   {
      std::vector< int > vector{ 1, 2, 3, 4, 5 };
      auto collection =
         From( vector )
            .Until( []( int m ) {
               return m == 3;
            } );

      std::vector< int > vector2{ 1, 2 };
      BOOST_REQUIRE_EQUAL_COLLECTIONS( vector2.begin(), vector2.end(), collection.begin(), collection.end() );
   }
}

BOOST_AUTO_TEST_CASE( Select )
{
   auto vector{ MoveToVector( { TestType{ 1 }, { 2 }, { 3 }, { 4 }, { 5 } } ) };
   BOOST_REQUIRE_EQUAL(
      From( std::move( vector ) )
         .Select< int >( []( const TestType& s ) { return s._t; } )
         .Sum(),
      15 );

   struct T1
   {
      int v;

      ~T1()
      {
         v = 0;
      }
   };

   auto getData = [] {
      return std::vector< T1 >( { T1{ 1 }, T1{ 2 }, T1{ 3 }, T1{ 4 }, T1{ 5 } } );
   };
   {
      auto ts = getData();
      BOOST_REQUIRE_EQUAL(
         From(
            From( ts )
               .Select< int >( []( const T1& m ) {
                  return m.v;
               } )
               .ToVector() )
            .Sum(),
         15 );
   }
   {
      std::vector< int > vector = { 2, 2, 3 };
      *From( vector )
          .Select< int& >( []( int& m ) {
             return std::ref( m );
          } )
          .begin() = 1;

      BOOST_TEST_REQUIRE(
         From( std::as_const( vector ) )
            .Select< const int& >( []( const int& m ) {
               return std::ref( m );
            } )
            .Sum() == 6 );
   }

   {
      std::vector< int > vector = { 2, 2, 3 };
      auto pvector =
         From( vector )
            .Select< int* >( []( int& m ) {
               return &m;
            } )
            .ToVector();

      *pvector.at( 0 ) = 1;

      BOOST_TEST_REQUIRE( vector.at( 0 ) == 1 );
   }

   {
      const std::vector vector = MoveToVector( { std::make_unique< int >( 1 ) } );
      From( vector ).Select< int >( []( const std::unique_ptr< int >& m ) { return *m; } ).ToVector();
   }
}

BOOST_AUTO_TEST_CASE( SelectWhere )
{
   BOOST_REQUIRE_EQUAL(
      From( MoveToVector( { TestType( 1 ), { 2 } } ) )
         .SelectWhere< int >( []( const TestType& ) { return boost::optional< int >{}; } )
         .Count(),
      0 );

   BOOST_REQUIRE_EQUAL(
      From( MoveToVector( { TestType( 1 ), { 2 } } ) )
         .SelectWhere< int >( []( const TestType& ) {
            int* ret = nullptr;
            return ret;
         } )
         .Count(),
      0 );

   BOOST_REQUIRE_EQUAL(
      From( MoveToVector( { TestType( 1 ), { 2 } } ) )
         .SelectWhere< int >( []( const TestType& ) {
            std::unique_ptr< int > ret;
            return ret;
         } )
         .Count(),
      0 );

   auto col11 =
      From( { 300, 2, 500, 4, 600 } )
         .SelectWhere< char >( []( int i ) {
            if( i > 255 )
            {
               return boost::optional< char >{};
            }
            return boost::optional< char >{ static_cast< char >( i ) };
         } )
         .ToVector();
   auto col12 = { 2, 4 };
   BOOST_REQUIRE_EQUAL_COLLECTIONS( col11.begin(), col11.end(), col12.begin(), col12.end() );

   {
      std::vector< int > vector = { 2, 2, 3 };
      auto pvector =
         From( vector )
            .SelectWhere< int* >( []( int& m ) {
               return boost::optional< int* >{ &m };
            } )
            .ToVector();

      *pvector.at( 0 ) = 1;

      BOOST_TEST_REQUIRE( vector.at( 0 ) == 1 );
   }

   {
      struct Test
      {
         boost::optional< std::wstring > mData;
      };

      std::vector vector{ { Test{ boost::optional< std::wstring >{ L"1" } } } };
      auto container1 =
         From( vector )
            .SelectWhere< std::wstring >( []( const Test& m ) {
               return m.mData;
            } );
      auto it = container1.begin();
      BOOST_TEST_REQUIRE( *it == L"1" );
      BOOST_TEST_REQUIRE( *it == L"1" );

      auto container2 =
         From( vector )
            .SelectWhere< std::wstring& >( []( Test& m ) {
               return boost::optional< std::wstring& >{ m.mData.value() };
            } );

      *container2.begin() = L"2";
      BOOST_TEST_REQUIRE( vector.at( 0 ).mData.value() == L"2" );

      auto container3 =
         From( vector )
            .SelectWhere< std::wstring& >( []( Test& m ) {
               return std::ref( m.mData );
            } );

      *container3.begin() = L"3";
      BOOST_TEST_REQUIRE( vector.at( 0 ).mData.value() == L"3" );
   }

   {
      const std::vector< int > constVector = { 1 };
      auto count = 0;
      auto container =
         From( constVector )
            .SelectWhere< int >(
               [&]( auto&& ) {
                  ++count;
                  return boost::optional< int >{ 1 };
               } )
            .ToVector();
      BOOST_TEST_REQUIRE( count == 1 );
   }
}

BOOST_AUTO_TEST_CASE( FromMove )
{
   std::vector< TestType > v( 5 );
   auto count = 0;
   for( auto& it : v )
   {
      it._t = ++count;
   }

   BOOST_REQUIRE_EQUAL(
      From( v )
         .Where( []( const TestType& ) { return true; } )
         .Concat( From( std::vector< TestType >() ) )
         .Select< int >( []( const TestType& t ) { return t._t; } )
         .Sum(),
      15 );
   BOOST_REQUIRE_EQUAL( v.size(), 5 );

   auto from1 = From( v ).Select< int >( []( const TestType& t ) { return t._t; } );
   auto from = from1;

   BOOST_REQUIRE_EQUAL( from1.Sum(), 15 );
   // это не копипаст, это такой тест.
   BOOST_REQUIRE_EQUAL( from1.Sum(), 15 );

   BOOST_REQUIRE_EQUAL( from.Sum(), 15 );

   std::vector< int > v2( 5 );
   count = 0;
   for( auto& it : v2 )
   {
      it = ++count;
   }

   auto from3 = From( std::move( v2 ) );
   auto from4 = from3;

   BOOST_REQUIRE_EQUAL( from3.Sum(), 15 );

   BOOST_REQUIRE_EQUAL( from4.Sum(), 15 );

   BOOST_REQUIRE_EQUAL(
      From( std::move( v ) )
         .Where( []( const TestType& ) { return true; } )
         .Concat( From( std::vector< TestType >() ) )
         .Select< int >( []( const TestType& t ) { return t._t; } )
         .Concat( From( std::vector< int >( { 2 } ) ) )
         .Exclude( From( std::vector< int >() ) )
         .Exclude( From( { 1 } ) )
         .Exclude( From( std::vector< int >( { 100 } ) ) )
         .Sum(),
      16 );
   BOOST_REQUIRE_EQUAL( v.size(), 0 );

   {
      std::vector< std::string > vector{ "1", "2" };
      From( vector )
         .Move()
         .ToVector();

      std::vector< std::string > vector2{ "", "" };
      BOOST_REQUIRE_EQUAL_COLLECTIONS( vector.begin(), vector.end(), vector2.begin(), vector2.end() );
   }
}

BOOST_AUTO_TEST_CASE( SelectMany )
{
   BOOST_REQUIRE_EQUAL(
      From( std::vector< int >() )
         .SelectMany< int >( []( int ) {
            return std::vector< int >();
         } )
         .Count(),
      0 );

   BOOST_REQUIRE_EQUAL(
      From( { std::vector< int >() } )
         .SelectMany< int >( []( const std::vector< int >& s ) {
            return s;
         } )
         .Sum(),
      0 );

   BOOST_REQUIRE_EQUAL(
      From( std::vector{ std::vector< int >{}, std::vector< int >{} } )
         .SelectMany< int >( []( std::vector< int >& s ) {
            return s;
         } )
         .Sum(),
      0 );

   auto v_select =
      From( { std::vector< TestType2 >( { TestType2( 1 ) } ), std::vector< TestType2 >( { TestType2( 1 ) } ) } )
         .SelectMany< TestType2 >( []( const std::vector< TestType2 >& s ) {
            return From( std::vector< TestType2 >( s ) ).Concat( std::vector< TestType2 >( { TestType2( 1 ) } ) );
         } )
         .Select< int >( []( const TestType2 t ) { return t._t; } )
         .ToVector();

   BOOST_REQUIRE_EQUAL( From( v_select ).Sum(), 4 );

   BOOST_REQUIRE_EQUAL(
      From( { 1 } )
         .SelectMany< TestType2 >( []( int ) {
            return std::vector< TestType2 >();
         } )
         .Sum(),
      0 );

   BOOST_REQUIRE_EQUAL(
      From( { 1 } )
         .SelectMany< TestType2 >( []( int i ) {
            return std::vector< TestType2 >( { TestType2( i ) } );
         } )
         .Where( []( const TestType2& t ) { return t._t != -1; } )
         // это не копипаст, это такой тест.
         .Where( []( const TestType2& t ) { return t._t != -1; } )
         .Sum(),
      1 );

   BOOST_REQUIRE_EQUAL(
      From( { 1 } )
         .SelectMany< TestType2 >( []( int ) {
            struct R
            {
               std::vector< TestType2 > r;
            };
            auto l = [] {
               R r;
               r.r = std::vector< TestType2 >( { TestType2( 1 ) } );
               return r;
            };
            return std::move( l().r );
         } )
         .Where( []( const TestType2& t ) { return t._t != -1; } )
         .Select< TestType2 >( []( const TestType2& t ) {
            TestType2 ret = t;
            return ret;
         } )
         .Sum(),
      1 );

   std::vector< TestType > v1( 5 );
   auto count = 0;
   for( auto& it : v1 )
   {
      it._t = ++count;
   }

   std::vector< std::vector< TestType > > v2( 1 );
   *v2.begin() = std::move( v1 );

   BOOST_REQUIRE_EQUAL(
      From( std::move( v2 ) )
         .SelectMany< int >( []( const std::vector< TestType >& s ) {
            return From( s )
               .Select< int >( []( const TestType& s ) { return s._t; } )
               .ToVector();
         } )
         .Sum(),
      15 );

   struct T1
   {
      int v1;

      ~T1()
      {
         v1 = 0;
      }
   };

   BOOST_TEST_REQUIRE(
      From( std::unordered_map< int, T1 >{ { 1, T1{ 1 } }, { 2, T1{ 2 } }, { 3, T1{ 1 } } } )
         .Select< const T1& >( []( const std::pair< const int, T1 >& m ) {
            return std::ref( m.second );
         } )
         .SelectMany< int >( []( const T1& t ) {
            return From( { 1 } )
               .Where( [&]( int i ) {
                  return i == t.v1;
               } )
               .Select< int >( [&]( int ) {
                  return t.v1;
               } );
         } )
         .Sum() == 2 );

   struct T2
   {
      std::vector< T1 > _t1;
   };

   auto container =
      From( { T2{ { T1{ 2 }, T1{ 2 }, T1{ 3 }, T1{ 4 }, T1{ 5 } } } } )
         .SelectMany< const T1& >( []( const T2& m ) -> const std::vector< T1 >& {
            return m._t1;
         } )
         .Select< const int& >( []( const T1& m ) -> const int& {
            return m.v1;
         } );

   const_cast< int& >( *container.begin() ) = 1;

   BOOST_REQUIRE_EQUAL( container.Sum(), 15 );

   BOOST_TEST_REQUIRE(
      From( { 1 } )
         .SelectMany< int >( []( int ) {
            return From( { 2, 3, 4 } );
         } )
         .Sum() == 9 );

   BOOST_TEST_REQUIRE(
      From( std::vector< int >{} )
         .SelectMany< int >( []( int ) {
            return From( { 2, 3, 4 } );
         } )
         .Sum() == 0 );

   BOOST_TEST_REQUIRE(
      From( { 1 } )
         .SelectMany< int >( []( int ) {
            return From( { 2, 3, 4 } )
               .Where( []( int i ) {
                  return i != 3;
               } )
               .Select< int >( []( int ) {
                  return 1;
               } );
         } )
         .Sum() == 2 );

   {
      auto vec = std::vector{ std::vector< int >{ 1 }, std::vector< int >{ 2 } };
      auto it =
         From( vec )
            .SelectMany< int& >( []( std::vector< int >& m ) {
               return std::ref( m );
            } )
            .begin();

      *it = 3;
      *++it = 4;

      BOOST_TEST_REQUIRE( vec.at( 0 ).at( 0 ) == 3 );
      BOOST_TEST_REQUIRE( vec.at( 1 ).at( 0 ) == 4 );
   }

   {
      auto con =
         From( std::vector{ std::vector< int >{}, std::vector< int >{} } )
            .SelectMany< int >( []( const std::vector< int >& m ) {
               static const auto r = From( m );
               return std::ref( r );
            } );
      BOOST_TEST_REQUIRE( ( con.begin() == con.end() ) );
   }

   {
      auto con =
         From( std::vector{ std::vector< int >{}, std::vector< int >{} } )
            .SelectMany< int >( []( const std::vector< int >& m ) {
               return From( m );
            } );
      BOOST_TEST_REQUIRE( ( con.begin() == con.end() ) );
   }

   {
      auto con =
         From( std::vector{ std::vector< int >{}, std::vector< int >{} } )
            .SelectMany< int >( []( const std::vector< int >& m ) {
               return From( m ).Where( []( auto&& ) { return true; } );
            } );
      BOOST_TEST_REQUIRE( ( con.begin() == con.end() ) );
   }

   {
      std::vector< int > vector = { 2, 2, 3 };
      auto pvector =
         From( vector )
            .SelectMany< int* >( []( int& m ) {
               return std::vector{ &m };
            } )
            .ToVector();

      *pvector.at( 0 ) = 1;

      BOOST_TEST_REQUIRE( vector.at( 0 ) == 1 );
   }
}

BOOST_AUTO_TEST_CASE( Reuse )
{
   auto from = From( { 1, 1 } );
   BOOST_REQUIRE_EQUAL( from.Select< int >( []( const int& t ) { return t; } ).Sum(), 2 );
   // это не копипаст, это такой тест.
   BOOST_REQUIRE_EQUAL( from.Select< int >( []( const int& t ) { return t; } ).Sum(), 2 );
}

BOOST_AUTO_TEST_CASE( Throttle )
{
   {
      BOOST_TEST_REQUIRE( From( { 1, 2, 3 } ).Throttle( 0 ).Count() == 0 );
   }

   {
      BOOST_TEST_REQUIRE( From( std::vector< int >{} ).Throttle( 10 ).Count() == 0 );
   }

   const auto from = From( { TestType2( 1 ), TestType2( 1 ) } );
   auto count = 0;
   for( const auto& it : from.Throttle( 1 ) )
   {
      BOOST_REQUIRE_EQUAL( it.Count(), 1 );
      ++count;
   }
   BOOST_REQUIRE_EQUAL( count, 2 );

   const auto throttle = From( { 1, 2, 3, 4, 5 } ).Throttle( 2 );
   for( auto i = 0; i < 2; ++i )
   {
      auto sums =
         throttle
            .Select< int >( []( auto&& m ) {
               return From( std::move( m ) ).Sum();
            } )
            .ToVector();

      BOOST_REQUIRE_EQUAL( sums.size(), 3 );

      BOOST_REQUIRE_EQUAL( sums[ 0 ], 3 );
      BOOST_REQUIRE_EQUAL( sums[ 1 ], 7 );
      BOOST_REQUIRE_EQUAL( sums[ 2 ], 5 );
   }

   {
      for( auto&& it : From( MoveToVector( { std::make_unique< int >( 1 ), std::make_unique< int >( 2 ) } ) ).Throttle( 1 ) )
      {
         BOOST_REQUIRE_EQUAL( it.Count(), 1 );
      }
   }

   {
      const std::vector< std::string > vector{ { '1' } };
      {
         auto container1 = linq::From( vector ).Throttle( 2 );
         auto it1 = container1.CreateIterator();
         auto container2 = it1.Next().value();
         auto it2 = container2.CreateIterator();
         BOOST_TEST_REQUIRE( it2.Next().value() == "1" );
         BOOST_TEST_REQUIRE( !it2.Next().is_initialized() );
         BOOST_TEST_REQUIRE( !it1.Next().is_initialized() );
      }

      {
         auto container1 = linq::From( vector ).Throttle( 2 );
         auto it1 = container1.begin();
         auto end1 = container1.end();
         BOOST_TEST_REQUIRE( ( it1 != end1 ) );
         auto container2 = std::move( *it1 );
         auto it2 = container2.begin();
         auto it3 = container2.begin();

         auto end2 = container2.end();
         BOOST_TEST_REQUIRE( ( it2 != end2 ) );
         BOOST_TEST_REQUIRE( *it2 == "1" );
         BOOST_TEST_REQUIRE( ( it3 != end2 ) );
         ++it2;
         BOOST_TEST_REQUIRE( ( it2 == end2 ) );
         ++it1;
         BOOST_TEST_REQUIRE( ( it1 == end1 ) );
      }
   }

   {
      std::vector< std::string > vector{ { '1' } };
      {
         auto container1 = linq::From( vector ).Throttle( 2 );
         for( auto&& it : container1 )
         {
            it.First() = '2';
         }
         BOOST_TEST_REQUIRE( vector.at( 0 ) == "2" );
      }
   }

   {
      std::vector< std::string > vector{ { '1' }, { '2' }, { '3' } };

      {
         auto container1 = linq::From( vector ).Throttle( 2 );
         auto it1 = container1.begin();
         auto it2 = ( *it1 ).begin();
         BOOST_TEST_REQUIRE( *it2 == "1" );
         ++it1;
         it2 = ( *it1 ).begin();
         BOOST_TEST_REQUIRE( *it2 == "3" );
         ++it2;
         BOOST_TEST_REQUIRE( ( it2 == ( *it1 ).end() ) );
         ++it1;
         BOOST_TEST_REQUIRE( ( it1 == container1.end() ) );
      }
   }
}

BOOST_AUTO_TEST_CASE( Capacity )
{
   struct CapacityTest
   {
      size_t* _copyCalled;

      CapacityTest( size_t* copyCalled )
         : _copyCalled( copyCalled )
      {
      }

      CapacityTest( const CapacityTest& k )
      {
         _copyCalled = k._copyCalled;
         ++( *_copyCalled );
      }

      CapacityTest( CapacityTest&& k )
      {
         _copyCalled = k._copyCalled;
         ++( *_copyCalled );
      }

      CapacityTest& operator=( const CapacityTest& k )
      {
         _copyCalled = k._copyCalled;
         ++( *_copyCalled );
         return *this;
      }

      CapacityTest& operator=( CapacityTest&& k )
      {
         _copyCalled = k._copyCalled;
         ++( *_copyCalled );
         return *this;
      }
   };

   std::vector< CapacityTest > data;

   size_t copyCalled = 0;

   data.push_back( CapacityTest( &copyCalled ) );

   auto capacity = data.capacity() * 10;

   data.clear();

   BOOST_REQUIRE_NE( capacity, 0 );

   for( size_t i = 0; i < capacity; ++i )
   {
      data.push_back( CapacityTest( &copyCalled ) );
   }
   copyCalled = 0;

   auto data2 = From( data ).ToVector();

   BOOST_REQUIRE_EQUAL( copyCalled, data.size() );

   copyCalled = 0;
   auto expectedCopyCalled = data.size() + data2.size();

   From( data ).Concat( data2 ).ToVector();

   BOOST_REQUIRE_EQUAL( copyCalled, expectedCopyCalled );
}

BOOST_AUTO_TEST_CASE( Complex )
{
   struct T1
   {
      struct T2
      {
         int v;

         ~T2()
         {
            v = 0;
         }
      };

      struct T3
      {
         struct T4
         {
            T2 td2;
         };
         T2 td2;
         std::vector< T4 > td4;
      };
      boost::optional< T3 > result;
   };

   auto getData = [] {
      return std::vector< T1 >{
         { boost::optional< T1::T3 >( T1::T3{
            T1::T2{ 2 },
            { T1::T3::T4{ T1::T2{ 2 } }, T1::T3::T4{ T1::T2{ 2 } } } } ) },
         { boost::optional< T1::T3 >( T1::T3{
            T1::T2{ 2 },
            { T1::T3::T4{ T1::T2{ 2 } }, T1::T3::T4{ T1::T2{ 1 } } } } ) }
      };
   };

   auto result =
      From( getData() )
         .Where( []( const T1& m ) { return !!m.result; } )
         .Select< const T1::T3& >( []( const T1& m ) -> const T1::T3& { return *m.result; } )
         .SelectMany< T1::T2 >( []( const T1::T3& m ) {
            return From( { m.td2 } )
               .Concat(
                  From( m.td4 )
                     .Select< T1::T2 >( []( const T1::T3::T4& m ) {
                        return m.td2;
                     } ) );
         } )
         .Where( []( const T1::T2& m ) { return m.v == 1; } )
         .Select< int64_t >( []( const T1::T2& m ) { return m.v; } )
         .FirstOrNone();

   BOOST_REQUIRE_EQUAL( !!result, true );
   BOOST_REQUIRE_EQUAL( *result, 1 );
}

BOOST_AUTO_TEST_CASE( Skip )
{
   BOOST_REQUIRE_EQUAL( From( { 1, 2, 3, 4, 5 } ).Skip( 2 ).Sum(), 12 );
}

BOOST_AUTO_TEST_CASE( Take )
{
   BOOST_REQUIRE_EQUAL( From( { 1, 2, 3, 4, 5 } ).Take( 2 ).Sum(), 3 );

   BOOST_REQUIRE_EQUAL( From( { 1, 2, 3, 4, 5 } ).Skip( 2 ).Take( 2 ).Sum(), 7 );
}

// BOOST_AUTO_TEST_CASE(LinqCppAsTearOffContainerTest)
// {

//     std::shared_ptr<linq::ITearOffContainer<int&>> container;
//     {
//         container = linq::From({ 1, 2, 3, 4, 5 }).AsTearOffContainer();
//     }

//     BOOST_REQUIRE_EQUAL(linq::From(container).Sum(), 15);

//     auto sequence1 = linq::From({ 1, 2, 3, 4, 5 });
//     auto sequence2 = linq::From(sequence1.AsTearOffContainer())
//                          .ToVector();

//     BOOST_REQUIRE_EQUAL_COLLECTIONS(sequence1.begin(), sequence1.end(), sequence2.begin(), sequence2.end());
// }

BOOST_AUTO_TEST_CASE( Distinct )
{
   {
      const auto col11 = From( { 1, 2, 3, 4, 5 } ).Distinct();
      const auto col12 = std::vector< int >{ 1, 2, 3, 4, 5 };
      BOOST_REQUIRE_EQUAL_COLLECTIONS( col11.begin(), col11.end(), col12.begin(), col12.end() );
   }

   {
      const auto col21 = From( { 1, 2, 3, 3, 4, 5 } ).Distinct();
      const auto col22 = std::vector< int >{ 1, 2, 3, 4, 5 };
      BOOST_REQUIRE_EQUAL_COLLECTIONS( col21.begin(), col21.end(), col22.begin(), col22.end() );
   }

   {
      const auto col31 = From( { 1, 2, 3, 4, 5, 5 } ).Distinct();
      const auto col32 = std::vector< int >{ 1, 2, 3, 4, 5 };
      BOOST_REQUIRE_EQUAL_COLLECTIONS( col31.begin(), col31.end(), col32.begin(), col32.end() );
   }

   {
      const auto col41 = From( { 1, 1, 2, 3, 4, 5 } ).Distinct();
      const auto col42 = std::vector< int >{ 1, 2, 3, 4, 5 };
      BOOST_REQUIRE_EQUAL_COLLECTIONS( col41.begin(), col41.end(), col42.begin(), col42.end() );
   }

   {
      const auto col51 = From( { 1, 1, 1, 1, 1, 1 } ).Distinct();
      const auto col52 = std::vector< int >{ 1 };
      BOOST_REQUIRE_EQUAL_COLLECTIONS( col51.begin(), col51.end(), col52.begin(), col52.end() );
   }

   {
      std::vector col61{ 1, 2, 3, 4, 3, 3, 3 };
      auto col62 = From( col61 ).Distinct();
      std::vector< int > col63{ col62.ToVector() };
      std::vector< int > col64 = { 1, 2, 3, 4 };
      BOOST_REQUIRE_EQUAL_COLLECTIONS( col63.begin(), col63.end(), col64.begin(), col64.end() );
   }

   {
      struct S
      {
         int V1;
         int V2;

         bool operator==( const S& m ) const
         {
            return V1 == m.V1 && V2 == m.V2;
         }
      };

      auto col61 =
         From( { S{ 1, 2 }, S{ 2, 3 }, S{ 2, 3 }, S{ 3, 4 } } )
            .Distinct( []( const S& m ) { return m.V1; } )
            .ToVector();
      auto col62 = std::vector< S >{ S{ 1, 2 }, S{ 2, 3 }, S{ 3, 4 } };

      BOOST_TEST_REQUIRE( col61 == col62 );
   }
}

BOOST_AUTO_TEST_CASE( Cast )
{
   {
      const std::vector< int > container = { 1, 2, 3 };
      BOOST_TEST_REQUIRE( From( container ).Cast< char >().Sum() == 6 );
   }

   {
      BOOST_TEST_REQUIRE( *linq::From( { std::unique_ptr< int >{ new int{ 1 } } } ).Cast< std::unique_ptr< int >& >().First() == 1 );
   }

   {
      BOOST_TEST_REQUIRE(
         *linq::From( { std::optional< std::unique_ptr< int > >{ std::unique_ptr< int >{ new int{ 1 } } } } )
             .Cast< const std::optional< std::unique_ptr< int > >& >()
             .First()
             .value() == 1 );
   }
}

BOOST_AUTO_TEST_CASE( Intersect )
{
   const std::vector< int > container1 = { 1, 2, 3 };
   const std::vector< int > container2 = { 4, 2, 5 };

   BOOST_TEST_REQUIRE( From( container1 ).Intersect( container2 ).Sum() == 2 );
}

BOOST_AUTO_TEST_CASE( Aggregate )
{
   {
      auto a = 1;
      BOOST_TEST_REQUIRE( From( { 1, 2, 3 } ).Aggregate( a, []( int a, int m ) { return a + m; } ) == 7 );
      BOOST_TEST_REQUIRE( a == 1 );
   }

   {
      const auto a = 1;
      BOOST_TEST_REQUIRE( From( std::vector< int >{} ).Aggregate( a, []( int a, int m ) { return a + m; } ) == 1 );
   }

   {
      auto a = 1;
      BOOST_TEST_REQUIRE(
         From( { 1, 2, 3 } )
            .Aggregate(
               a,
               []( int, int m ) {
                  auto& b = m;
                  return b;
               } ) == 3 );
   }

   {
      auto a = 1;
      BOOST_TEST_REQUIRE(
         From( { 1, 2, 3 } )
            .Aggregate(
               a,
               []( int& a, int ) {
                  return std::ref( a );
               } ) == a );
   }

   {
      auto a = 1;
      BOOST_TEST_REQUIRE(
         From( { 1, 2, 3 } )
            .Aggregate(
               &a,
               []( int* a, int ) {
                  return a;
               } ) == &a );
   }

   {
      const auto a = 1;
      BOOST_TEST_REQUIRE(
         *From( { 1, 2, 3 } )
             .Aggregate(
                &a,
                []( const int* a, const int& m ) {
                   a = &m;
                   return a;
                } ) == 3 );
   }
}

BOOST_AUTO_TEST_CASE( FirstOrNone )
{
   BOOST_TEST_REQUIRE( From( { 1, 2, 3 } ).FirstOrNone( []( int m ) { return m > 2; } ).value() == 3 );
   BOOST_TEST_REQUIRE( From( { 1, 2, 3 } ).First( []( int m ) { return m > 2; } ) == 3 );

   {
      auto container = From( { 1, 2, 3 } );
      linq::d::optional< const int& > v = container.FirstOrNone< const int& >();
      BOOST_TEST_REQUIRE( v.is_initialized() );
   }

   {
      auto container = From( { 1, 2, 3 } );
      linq::d::optional< int > v = container.FirstOrNone();
      BOOST_TEST_REQUIRE( v.is_initialized() );
   }

   {
      const auto container =
         From( { 1, 2, 3 } )
            .Select< int >( []( int ) {
               return 5;
            } );
      linq::d::optional< int > v = container.FirstOrNone();
      BOOST_TEST_REQUIRE( v.is_initialized() );
   }

   {
      auto container = From( std::vector{ 1, 2, 3 } );
      auto pointer = &*container.begin();
      BOOST_TEST_REQUIRE( &container.FirstOrNone< const int& >().value() == pointer );

      container.FirstOrNone< int& >().value() = 5;
      BOOST_TEST_REQUIRE( *pointer == 5 );
   }

   {
      BOOST_TEST_REQUIRE(
         From( { 1, 2, 3 } )
            .Select< int >( []( int ) {
               return 5;
            } )
            .FirstOrNone()
            .value() == 5 );
   }
}

BOOST_AUTO_TEST_CASE( First )
{
   BOOST_TEST_REQUIRE( From( { 1, 2, 3 } ).First() == 1 );
   BOOST_TEST_REQUIRE( From( { 1, 2, 3 } ).First( []( int m ) { return m > 2; } ) == 3 );

   {
      auto container = From( std::vector{ 1, 2, 3 } );
      auto pointer = &*container.begin();
      BOOST_TEST_REQUIRE( &container.First() == pointer );

      container.First() = 5;
      BOOST_TEST_REQUIRE( *pointer == 5 );
   }

   {
      BOOST_TEST_REQUIRE(
         From( { 1, 2, 3 } )
            .Select< int >( []( int ) {
               return 5;
            } )
            .First() == 5 );
   }
}

BOOST_AUTO_TEST_CASE( FirstOr )
{
   {
      BOOST_TEST_REQUIRE( From( { 1 } ).FirstOr( 2 ) == 1 );
   }

   {
      auto container = From( { 1, 2, 3 } );
      int i = 2;
      container.FirstOr( i ) = 5;
      BOOST_TEST_REQUIRE( container.First() == 5 );
   }
}

BOOST_AUTO_TEST_CASE( LastOrNone )
{
   BOOST_TEST_REQUIRE( From( { 1, 2, 3 } ).LastOrNone( []( int m ) { return m < 3; } ).value() == 2 );
   {
      auto container = From( { 1, 2, 3 } );
      linq::d::optional< const int& > v = container.LastOrNone< const int& >();
      BOOST_TEST_REQUIRE( v.is_initialized() );
   }

   {
      auto container = From( { 1, 2, 3 } );
      linq::d::optional< int > v = container.LastOrNone();
      BOOST_TEST_REQUIRE( v.is_initialized() );
   }

   {
      const auto container =
         From( { 1, 2, 3 } )
            .Select< int >( []( int ) {
               return 5;
            } );
      linq::d::optional< int > v = container.LastOrNone();
      BOOST_TEST_REQUIRE( v.is_initialized() );
   }

   {
      auto container = From( std::vector{ 1, 2, 3 } );
      auto pointer = &*++++container.begin();
      BOOST_TEST_REQUIRE( &container.LastOrNone< int& >().value() == pointer );

      container.LastOrNone< int& >().value() = 5;
      BOOST_TEST_REQUIRE( *pointer == 5 );
   }

   {
      BOOST_TEST_REQUIRE(
         From( { 1, 2, 3 } )
            .Select< int >( []( int ) {
               return 5;
            } )
            .LastOrNone()
            .value() == 5 );
   }
}

BOOST_AUTO_TEST_CASE( Last )
{
   BOOST_TEST_REQUIRE( From( { 1, 2, 3 } ).Last() == 3 );
   BOOST_TEST_REQUIRE( From( { 1, 2, 3 } ).LastOrNone( []( int m ) { return m < 3; } ).value() == 2 );

   {
      auto container = From( std::vector{ 1, 2, 3 } );
      auto pointer = &*++++container.begin();
      BOOST_TEST_REQUIRE( &container.Last() == pointer );

      container.Last() = 5;
      BOOST_TEST_REQUIRE( *pointer == 5 );
   }

   {
      BOOST_TEST_REQUIRE(
         From( { 1, 2, 3 } )
            .Select< int >( []( int ) {
               return 5;
            } )
            .Last() == 5 );
   }
}

BOOST_AUTO_TEST_CASE( LastOr )
{
   BOOST_TEST_REQUIRE( From( { 1 } ).LastOr( 2 ) == 1 );

   {
      auto container = From( { 1, 2, 3 } );
      int i = 2;
      container.LastOr( i ) = 5;
      BOOST_TEST_REQUIRE( container.Last() == 5 );
      BOOST_TEST_REQUIRE( container.First() == 1 );
   }
}

BOOST_AUTO_TEST_CASE( SingleOrNone )
{
   BOOST_TEST_REQUIRE( From( { 1, 2, 3 } ).SingleOrNone( []( int m ) { return m < 2; } ).value() == 1 );
   {
      auto container = From( { 1, 2, 3 } );
      linq::d::optional< const int& > v = container.SingleOrNone< const int& >();
      BOOST_TEST_REQUIRE( !v.is_initialized() );
   }

   {
      auto container = From( { 1, 2, 3 } );
      linq::d::optional< int > v = container.SingleOrNone();
      BOOST_TEST_REQUIRE( !v.is_initialized() );
   }

   {
      const auto container =
         From( { 1, 2, 3 } )
            .Select< int >( []( int ) {
               return 5;
            } );
      linq::d::optional< int > v = container.SingleOrNone();
      BOOST_TEST_REQUIRE( !v.is_initialized() );
   }

   {
      auto container = From( std::vector{ 1 } );
      auto pointer = &*container.begin();
      BOOST_TEST_REQUIRE( &container.SingleOrNone< int& >().value() == pointer );

      container.SingleOrNone< int& >().value() = 5;
      BOOST_TEST_REQUIRE( *pointer == 5 );
   }

   {
      BOOST_TEST_REQUIRE(
         From( { 1 } )
            .Select< int >( []( int ) {
               return 5;
            } )
            .SingleOrNone()
            .value() == 5 );
   }

   {
      BOOST_TEST_REQUIRE(
         !From( { 1, 2 } )
             .Select< int >( []( int ) {
                return 5;
             } )
             .SingleOrNone()
             .is_initialized() );
   }
}

BOOST_AUTO_TEST_CASE( Single )
{
   BOOST_TEST_REQUIRE( From( { 1 } ).Single() == 1 );
   BOOST_TEST_REQUIRE( From( { 1, 2, 3 } ).SingleOrNone( []( int m ) { return m == 3; } ).value() == 3 );

   {
      auto container = From( std::vector{ 1 } );
      auto pointer = &*container.begin();
      BOOST_TEST_REQUIRE( &container.Single() == pointer );

      container.Single() = 5;
      BOOST_TEST_REQUIRE( *pointer == 5 );
   }

   {
      BOOST_TEST_REQUIRE(
         From( { 1 } )
            .Select< int >( []( int ) {
               return 5;
            } )
            .Single() == 5 );
   }
}

BOOST_AUTO_TEST_CASE( SingleOr )
{
   BOOST_TEST_REQUIRE( From( { 1 } ).SingleOr( 2 ) == 1 );

   {
      auto container = From( { 1 } );
      int i = 2;
      container.SingleOr( i ) = 5;
      BOOST_TEST_REQUIRE( container.Single() == 5 );
      BOOST_TEST_REQUIRE( container.First() == 5 );
   }
}

BOOST_AUTO_TEST_CASE( MinMax )
{
   {
      auto container = From( { 2, 1, 3 } );
      BOOST_TEST_REQUIRE( container.Min() == 1 );
   }

   {
      auto container = From( { 2, 1, 3 } );
      BOOST_TEST_REQUIRE( container.Max() == 3 );
   }
}

BOOST_AUTO_TEST_CASE( Const )
{
   BOOST_TEST_REQUIRE( 0 == 0 );
   {
      std::vector< const int* > container;
      static_assert( std::is_same_v< decltype( *container.begin() ), const int*& > );
      static_assert( std::is_same_v< decltype( *From( container ).begin() ), const int*& > );

      std::list< const int* > list = From( container ).ToList();
      std::vector< const int* > vector = From( container ).ToVector();
      boost::optional< const int* > minOrNone = From( container ).MinOrNone();
      boost::optional< const int* > maxOrNone = From( container ).MaxOrNone();
      boost::optional< const int* > lastOrNone = From( container ).LastOrNone();
      boost::optional< const int* > firstOrNone = From( container ).FirstOrNone();
      std::unordered_set< const int* > unorderedSet = From( container ).ToUnorderedSet();
   }

   {
      const std::vector< const int* > container;
      static_assert( std::is_same_v< decltype( *container.begin() ), const int* const& > );
      static_assert( std::is_same_v< decltype( *From( container ).begin() ), const int* const& > );

      std::list< const int* > list = From( container ).ToList();
      std::vector< const int* > vector = From( container ).ToVector();
      boost::optional< const int* > minOrNone = From( container ).MinOrNone();
      boost::optional< const int* > maxOrNone = From( container ).MaxOrNone();
      boost::optional< const int* > lastOrNone = From( container ).LastOrNone();
      boost::optional< const int* > firstOrNone = From( container ).FirstOrNone();
      std::unordered_set< const int* > unorderedSet = From( container ).ToUnorderedSet();
   }

   {
      std::vector< int* > container;
      static_assert( std::is_same_v< decltype( *container.begin() ), int*& > );
      static_assert( std::is_same_v< decltype( *From( container ).begin() ), int*& > );

      std::list< int* > list = From( container ).ToList();
      std::vector< int* > vector = From( container ).ToVector();
      boost::optional< int* > minOrNone = From( container ).MinOrNone();
      boost::optional< int* > maxOrNone = From( container ).MaxOrNone();
      boost::optional< int* > lastOrNone = From( container ).LastOrNone();
      boost::optional< int* > firstOrNone = From( container ).FirstOrNone();
      std::unordered_set< int* > unorderedSet = From( container ).ToUnorderedSet();
   }

   {
      const std::vector< int* > container;
      static_assert( std::is_same_v< decltype( *container.begin() ), int* const& > );
      static_assert( std::is_same_v< decltype( *From( container ).begin() ), int* const& > );

      std::list< int* > list = From( container ).ToList();
      std::vector< int* > vector = From( container ).ToVector();
      boost::optional< int* > minOrNone = From( container ).MinOrNone();
      boost::optional< int* > maxOrNone = From( container ).MaxOrNone();
      boost::optional< int* > lastOrNone = From( container ).LastOrNone();
      boost::optional< int* > firstOrNone = From( container ).FirstOrNone();
      std::unordered_set< int* > unorderedSet = From( container ).ToUnorderedSet();
   }

   {
      const std::vector< int > container;
      std::vector< const int* > vector = From( container ).Select< const int* >( []( const int& m ) { return &m; } ).ToVector();
      boost::optional< const int*& > firstOrNone = From( vector ).FirstOrNone< const int*& >();
      ( void )firstOrNone;
   }

   {
      const std::vector< int > container;
      std::vector< const int* > vector = From( container ).Select< const int* >( []( const int& m ) { return &m; } ).ToOrderedVector( []( auto&&, auto&& ) { return false; } );
   }
}

BOOST_AUTO_TEST_CASE( Contains )
{
   BOOST_TEST_REQUIRE( From( { 1, 2, 3 } ).Contains( 2 ) );
   BOOST_TEST_REQUIRE( !From( { 1, 2, 3 } ).Contains( 5 ) );
}

BOOST_AUTO_TEST_CASE( IsIntersect )
{
   BOOST_TEST_REQUIRE( From( { 1, 2, 3 } ).IsIntersect( From( { 2 } ) ) );
   BOOST_TEST_REQUIRE( !From( { 1, 2, 3 } ).IsIntersect( From( { 5 } ) ) );
}

BOOST_AUTO_TEST_CASE( ToUnorderedSet )
{
   auto set = From( { 1, 2, 3, 2 } ).ToUnorderedSet< size_t >( []( int m ) { return m; } );
   BOOST_TEST_REQUIRE( From( set ).Sum() == 6 );
}

BOOST_AUTO_TEST_CASE( Optional )
{
   {
      BOOST_TEST_REQUIRE( From( { boost::optional< int >{ 1 } } ).mShim.CreateIterator().Next().value().get().value() == 1 );
      BOOST_TEST_REQUIRE( From( { boost::optional< int >{ 1 } } ).Where( []( boost::optional< int >& ) { return true; } ).mShim.CreateIterator().Next().value().get().value() == 1 );
   }

   {
      std::vector vector{ boost::optional< int >{ 1 } };
      From( vector ).Select< std::reference_wrapper< boost::optional< int > > >( []( boost::optional< int >& m ) { return std::ref( m ); } ).First().get().value() = 2;
      BOOST_TEST_REQUIRE( vector.at( 0 ).value() == 2 );
   }
}

BOOST_AUTO_TEST_CASE( Array )
{
   {
      int array[] = { 1, 2, 3 };
      From( array ).Last() = 5;
      BOOST_TEST_REQUIRE( array[ 2 ] == 5 );
   }

   {
      const int array2[] = { 1, 2, 3 };
      BOOST_TEST_REQUIRE( From( array2 ).Last() == 3 );
   }

   {
      std::optional< int > array[] = { 1, 2, 3 };
      BOOST_TEST_REQUIRE( From( array ).Select< int >( []( const auto& m ) { return m.value(); } ).Sum() == 6 );
   }

   {
      BOOST_TEST_REQUIRE( From( { std::optional< int >{ 1 }, { { 2 } }, { { 3 } } } ).Select< int >( []( const auto& m ) { return m.value(); } ).Sum() == 6 );
   }

   {
      auto container = From( { std::optional< int >{ 1 }, { { 2 } }, { { 3 } } } );
      BOOST_TEST_REQUIRE( container.Select< int >( []( const auto& m ) { return m.value(); } ).Sum() == 6 );
   }

   {
      auto container = From( { std::optional< int >{ 1 }, { { 2 } }, { { 3 } } } );
      BOOST_TEST_REQUIRE( container.Where( []( const auto& m ) { return m.value() > 2; } ).FirstOrNone().value().value() == 3 );
   }

   {
      auto container = From( { std::optional< int >{ 1 }, { { 2 } }, { { 3 } } } );
      BOOST_TEST_REQUIRE( container.First( []( auto& ) { return true; } ).value() == 1 );
   }
}

BOOST_AUTO_TEST_CASE( Ref )
{
   {
      auto container = From( { std::optional< int >{ 1 } } );
      auto container2 = container;
      container2.First().value() = 5;
      BOOST_TEST_REQUIRE( container.First().value() == 1 );
      BOOST_TEST_REQUIRE( container2.First().value() == 5 );
   }

   {
      auto container = From( { std::optional< int >{ 1 } } );
      auto container2 = container.Ref();
      container2.First().value() = 5;
      BOOST_TEST_REQUIRE( container.First().value() == 5 );
      BOOST_TEST_REQUIRE( container2.First().value() == 5 );
   }

   {
      auto container = From( { std::optional< int >{ 1 } } );
      auto container2 = container.Ref();
      auto container3 = container.Ref().Where( []( auto&& ) { return true; } );
      container2.First().value() = 5;
      BOOST_TEST_REQUIRE( container.First().value() == 5 );
      BOOST_TEST_REQUIRE( container2.First().value() == 5 );
      BOOST_TEST_REQUIRE( container3.First().value() == 5 );
   }
}

BOOST_AUTO_TEST_CASE( FromFn )
{
   {
      auto container = From(
         []() {
            return boost::optional< int >{};
         },
         0 );

      for( auto& it : container )
      {
         BOOST_TEST_REQUIRE( false );
         BOOST_TEST_REQUIRE( it == 0 );
      }
   }

   {
      int i = 0;
      auto container = From< int& >(
         [&]() {
            return &i;
         },
         0 );

      for( auto& it : container )
      {
         BOOST_TEST_REQUIRE( it == 0 );
         it = 1;
         BOOST_TEST_REQUIRE( i == 1 );
         break;
      }
   }

   {
      int i = 0;
      auto container = From(
         [&]() {
            return &i;
         },
         1 );

      for( auto& it : container )
      {
         it = 1;
         break;
      }

      BOOST_TEST_REQUIRE( i == 1 );
   }

   {
      int i = 10;

      auto container = From(
         [&]() {
            if( i > 0 )
            {
               return boost::optional< int >{ i-- };
            }
            return boost::optional< int >{};
         },
         0 );

      auto j = 0;
      for( auto& it : container )
      {
         ++j;
         BOOST_TEST_REQUIRE( it != 0 );
      }

      BOOST_TEST_REQUIRE( j == 10 );
   }

   {
      int i = 0;
      auto container = From(
         [&]() {
            return &++i;
         },
         0 );

      From( { 1, 2, 3, 4, 5 } )
         .Concat( container )
         .Take( 4 )
         .ToVector();
      BOOST_TEST_REQUIRE( i == 0 );

      From( { 1, 2, 3, 4, 5 } )
         .Concat( container )
         .Skip( 5 )
         .Take( 1 )
         .ToVector();
      BOOST_TEST_REQUIRE( i == 2 ); // in future should be i == 1
   }
}

BOOST_AUTO_TEST_CASE( ToArray )
{
   {
      std::vector< int > src = { 1, 2 };
      auto dst =
         linq::From( src )
            .ToArray< 2 >();
      BOOST_REQUIRE_EQUAL_COLLECTIONS( src.begin(), src.end(), dst.begin(), dst.end() );
   }

   {
      std::vector< int > src = { 1 };
      BOOST_REQUIRE_THROW(
         linq::From( src )
            .ToArray< 2 >(),
         std::exception );
   }

   {
      std::vector< int > src = { 1, 2 };
      BOOST_REQUIRE_THROW(
         linq::From( src )
            .ToArray< 1 >(),
         std::exception );
   }

   {
      auto src = MoveToVector( { std::unique_ptr< int >{} } );
      linq::From( src ).Move().ToArray< 1 >();
   }
}
} // namespace test
BOOST_AUTO_TEST_SUITE_END()
} // namespace linq
