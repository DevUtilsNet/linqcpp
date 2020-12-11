# LinqCpp

In a basic sense, LINQ CPP represents a new approach to collections. 
In the old way, you had to write complex for loops that specified how to retrieve data from a collection. 
In the LINQ approach, you write declarative code that describes what you want to retrieve.

In addition, LINQ queries offer three main advantages over traditional foreach loops:

1. They are more concise and readable, especially when filtering multiple conditions.

2. They provide powerful filtering, ordering, and grouping capabilities with a minimum of application code.

3. They can be ported to other data sources with little or no modification.

In general, the more complex the operation you want to perform on the data, the more benefit you will realize by using LINQ instead of traditional iteration techniques.

```
auto result =
      linq::From( getData() )
         .Where( []( const T1& m ) { return !!m.result; } )
         .Select< const T1::T3& >( []( const T1& m ) -> const T1::T3& { return *m.result; } )
         .SelectMany< T1::T2 >( []( const T1::T3& m ) {
            return linq::From( { m.td2 } )
               .Concat(
                  linq::From( m.td4 )
                     .Select< T1::T2 >( []( const T1::T3::T4& m ) {
                        return m.td2;
                     } ) );
         } )
         .Where( []( const T1::T2& m ) { return m.v == 1; } )
         .Select< int64_t >( []( const T1::T2& m ) { return m.v; } )
         .FirstOrNone();
```
