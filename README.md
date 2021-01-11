# LinqCpp

In a basic sense, LINQ CPP represents a new approach to collections. 
In the old way, you had to write complex for loops that specified how to retrieve data from a collection. 
In the LINQ approach, you write declarative code that describes what you want to retrieve.

In addition, LINQ queries offer three main advantages over traditional foreach loops:

1. They are more concise and readable, especially when filtering multiple conditions.

2. They provide powerful filtering, ordering, and grouping capabilities with a minimum of application code.

3. They can be ported to other data sources with little or no modification.

In general, the more complex the operation you want to perform on the data, the more benefit you will realize by using LINQ instead of traditional iteration techniques.

- [x] **True Lazy**
- [x] **No Virtual Calls**
...
```
auto result =
      From( getData() )
         .Where( []( const T1& m ) { return !!m.result; } )
         .Select< const T1::T3& >( []( const T1& m ) { return std::ref( *m.result ); } )
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
```
...
```
From( std::move( v ) )
         .Where( []( const TestType& ) { return true; } )
         .Concat( From( std::vector< TestType >() ) )
         .Select< int >( []( const TestType& t ) { return t._t; } )
         .Concat( From( { 2 } ) )
         .Exclude( From( std::vector< int >() ) )
         .Exclude( From( { 1 } ) )
         .Exclude( From( { 100 } ) )
         .Sum();
```
