#include <linqcpp/linqcpp.h>

namespace test {
    BOOST_AUTO_TEST_SUITE(__main__)
BOOST_AUTO_TEST_SUITE(linqcpp)

struct TestType {
    int _t;

    TestType(int t)
        : _t(t)
    {
    }

    TestType() = default;

    TestType(TestType&&) = default;

    TestType(const TestType&) = delete;

    ~TestType()
    {
        _t = -1;
    }

    TestType& operator=(TestType&&) = default;
};

struct TestType2 {
    int _t;

    TestType2(int t)
        : _t(t)
    {
    }

    TestType2() = default;

    TestType2(TestType2&&) = default;

    TestType2(const TestType2&) = default;

    ~TestType2()
    {
        _t = -1;
    }

    TestType2& operator=(TestType2&&) = default;

    TestType2& operator=(const TestType2&) = default;

    TestType2 operator+(const TestType2& t)
    {
        return TestType2(_t + t._t);
    }

    operator int() const
    {
        return _t;
    }
};

BOOST_AUTO_TEST_CASE(LinqCppInitializerListTest)
{
    auto from = linq::From({ TestType2(1), TestType2(1) });
    BOOST_REQUIRE_EQUAL(from.Sum(), 2);
    BOOST_REQUIRE_EQUAL(linq::From({ 1, 2, 3, 4, 5 }).Count(), 5);
}

BOOST_AUTO_TEST_CASE(LinqCppConcatTest)
{
    BOOST_REQUIRE_EQUAL(linq::From({ 1, 2, 3, 4, 5 }).Concat(linq::From({ 1, 2, 3, 4, 5 })).Count(), 10);
    BOOST_REQUIRE_EQUAL(linq::From({ 1, 2, 3, 4, 5 }).Concat(linq::From(std::vector<int>())).Count(), 5);
    BOOST_REQUIRE_EQUAL(linq::From(std::vector<int>()).Concat(linq::From(std::vector<int>({ 1, 2, 3, 4, 5 }))).Count(), 5);
}

BOOST_AUTO_TEST_CASE(LinqCppContainerTest)
{
    BOOST_REQUIRE_EQUAL(linq::From(std::list<int>({ 1, 2, 3, 4, 5 })).Count(), 5);
    BOOST_REQUIRE_EQUAL(linq::From(std::vector<int>({ 1, 2, 3, 4, 5 })).Count(), 5);
    BOOST_REQUIRE_EQUAL(linq::From(linq::From(std::vector<int>({ 1, 2, 3, 4, 5 }))).Count(), 5);
}

BOOST_AUTO_TEST_CASE(LinqCppWhereTest)
{
    BOOST_REQUIRE_EQUAL(
        linq::From({ 1, 2, 3, 4, 5 })
            .Where([](int i) { return i > 3; })
            .Count(),
        2);
    BOOST_REQUIRE_EQUAL(
        linq::From({ 1, 2, 3, 4, 5 })
            .Where([](const int& i) { return i > 3; })
            .Count(),
        2);

    auto container1 = linq::From({ TestType2(1), TestType2(2) });
    auto container = container1;
    auto iterator1 = container1.begin();
    auto iterator = std::move(iterator1);

    BOOST_TEST_REQUIRE((iterator != container1.end()));

    BOOST_REQUIRE_EQUAL(
        container
            .Where([](const TestType2&) { return true; })
            .Where([](const TestType2&) { return true; })
            .Sum(),
        3);

    {
        const std::vector<int> constVector = { 1, 2, 3 };
        BOOST_TEST_REQUIRE(
            linq::From(constVector)
                .Where([](const int& m) {
                    return m == 2;
                })
                .Sum()
            == 2);
    }
}

BOOST_AUTO_TEST_CASE(LinqCppSelectTest)
{
    BOOST_REQUIRE_EQUAL(
        linq::Move({ TestType(1), { 2 }, { 3 }, { 4 }, { 5 } })
            .Select<int>([](const TestType& s) { return s._t; })
            .Sum(),
        15);

    struct T1 {
        int v;

        ~T1()
        {
            v = 0;
        }
    };

    auto getData = [] {
        return std::vector<T1>({ T1 { 1 }, T1 { 2 }, T1 { 3 }, T1 { 4 }, T1 { 5 } });
    };
    {
        auto ts = getData();
        BOOST_REQUIRE_EQUAL(
            linq::From(
                linq::From(ts)
                    .Select<int>([](const T1& m) {
                        return m.v;
                    })
                    .ToVector())
                .Sum(),
            15);
    }
    {
        std::vector<int> vector = { 2, 2, 3 };
        *linq::From(vector).Select<int&>([](int& m) {
                               return std::ref(m);
                           })
             .begin()
            = 1;

        BOOST_TEST_REQUIRE(
            linq::From(vector)
                .Select<const int&>([](const int& m) {
                    return std::ref(m);
                })
                .Sum()
            == 6);
    }
}

BOOST_AUTO_TEST_CASE(LinqCppFromMoveTest)
{
    std::vector<TestType> v(5);
    auto count = 0;
    for (auto& it : v) {
        it._t = ++count;
    }

    BOOST_REQUIRE_EQUAL(
        linq::From(v)
            .Where([](const TestType&) { return true; })
            .Concat(linq::From(std::vector<TestType>()))
            .Select<int>([](const TestType& t) { return t._t; })
            .Sum(),
        15);
    BOOST_REQUIRE_EQUAL(v.size(), 5);

    auto from1 = linq::From(v).Select<int>([](const TestType& t) { return t._t; });
    auto from2 = from1;

    BOOST_REQUIRE_EQUAL(from1.Sum(), 15);
    // это не копипаст, это такой тест.
    BOOST_REQUIRE_EQUAL(from1.Sum(), 15);

    BOOST_REQUIRE_EQUAL(from2.Sum(), 15);

    std::vector<int> v2(5);
    count = 0;
    for (auto& it : v2) {
        it = ++count;
    }

    auto from3 = linq::From(std::move(v2));
    auto from4 = from3;

    BOOST_REQUIRE_EQUAL(from3.Sum(), 15);

    BOOST_REQUIRE_EQUAL(from4.Sum(), 15);

    BOOST_REQUIRE_EQUAL(
        linq::From(std::move(v))
            .Where([](const TestType&) { return true; })
            .Concat(linq::From(std::vector<TestType>()))
            .Select<int>([](const TestType& t) { return t._t; })
            .Concat(linq::From(std::vector<int>({ 2 })))
            .Exclude(linq::From(std::vector<int>()))
            .Exclude(linq::From(std::vector<int>({ 1 })))
            .Exclude(linq::From(std::vector<int>({ 100 })))
            .Sum(),
        16);
    BOOST_REQUIRE_EQUAL(v.size(), 0);
}

BOOST_AUTO_TEST_CASE(LinqCppSelectManyTest)
{
    BOOST_REQUIRE_EQUAL(
        linq::From(std::vector<int>())
            .SelectMany<int>([](int) {
                return std::vector<int>();
            })
            .Count(),
        0);

    BOOST_REQUIRE_EQUAL(
        linq::From({ std::vector<int>() })
            .SelectMany<int>([](const std::vector<int>& s) {
                return s;
            })
            .Sum(),
        0);

    auto v_select = linq::From({ std::vector<TestType2>({ TestType2(1) }), std::vector<TestType2>({ TestType2(1) }) })
                        .SelectMany<TestType2>([](const std::vector<TestType2>& s) {
                            return linq::From(std::vector<TestType2>(s)).Concat(std::vector<TestType2>({ TestType2(1) }));
                        })
                        .Select<int>([](const TestType2 t) { return t._t; })
                        .ToVector();

    BOOST_REQUIRE_EQUAL(linq::From(v_select).Sum(), 4);

    BOOST_REQUIRE_EQUAL(
        linq::From({ 1 })
            .SelectMany<TestType2>([](int) {
                return std::vector<TestType2>();
            })
            .Sum(),
        0);

    BOOST_REQUIRE_EQUAL(
        linq::From({ 1 })
            .SelectMany<TestType2>([](int i) {
                return std::vector<TestType2>({ TestType2(i) });
            })
            .Where([](const TestType2& t) { return t._t != -1; })
            // это не копипаст, это такой тест.
            .Where([](const TestType2& t) { return t._t != -1; })
            .Sum(),
        1);

    BOOST_REQUIRE_EQUAL(
        linq::From({ 1 })
            .SelectMany<TestType2>([](int) {
                struct R {
                    std::vector<TestType2> r;
                };
                auto l = [] {
                    R r;
                    r.r = std::vector<TestType2>({ TestType2(1) });
                    return r;
                };
                return std::move(l().r);
            })
            .Where([](const TestType2& t) { return t._t != -1; })
            .Select<TestType2>([](const TestType2& t) {
                TestType2 ret = t;
                return ret;
            })
            .Sum(),
        1);

    std::vector<TestType> v1(5);
    auto count = 0;
    for (auto& it : v1) {
        it._t = ++count;
    }

    std::vector<std::vector<TestType>> v2(1);
    *v2.begin() = std::move(v1);

    BOOST_REQUIRE_EQUAL(
        linq::From(std::move(v2))
            .SelectMany<int>([](const std::vector<TestType>& s) {
                return linq::From(s)
                    .Select<int>([](const TestType& s) { return s._t; })
                    .ToVector();
            })
            .Sum(),
        15);

    struct T1 {
        int v1;

        ~T1()
        {
            v1 = 0;
        }
    };

    struct T2 {
        std::vector<T1> _t1;
    };

    auto container
        = linq::From({ T2 { { T1 { 2 }, T1 { 2 }, T1 { 3 }, T1 { 4 }, T1 { 5 } } } })
              .SelectMany<const T1&>([](const T2& m) -> const std::vector<T1>& {
                  return m._t1;
              })
              .Select<const int&>([](const T1& m) -> const int& {
                  return m.v1;
              });

    const_cast<int&>(*container.begin()) = 1;

    BOOST_REQUIRE_EQUAL(container.Sum(), 15);
}

BOOST_AUTO_TEST_CASE(LinqCppReuseTest)
{
    auto from = linq::Move({ TestType(1), TestType(1) });
    BOOST_REQUIRE_EQUAL(from.Select<int>([](const TestType& t) { return t._t; }).Sum(), 2);
    // это не копипаст, это такой тест.
    BOOST_REQUIRE_EQUAL(from.Select<int>([](const TestType& t) { return t._t; }).Sum(), 2);
}

BOOST_AUTO_TEST_CASE(LinqCppThrottleTest)
{

    auto from = linq::From({ TestType2(1), TestType2(1) });
    auto count = 0;
    for (const auto& it : from.Throttle(1)) {
        BOOST_REQUIRE_EQUAL(linq::From(it).Count(), 1);
        ++count;
    }
    BOOST_REQUIRE_EQUAL(count, 2);

    auto throttle = linq::From({ 1, 2, 3, 4, 5 }).Throttle(2);
    for (auto i = 0; i < 2; ++i) {
        auto sums = throttle
                        .Select<int>([](const auto& m) {
                            return linq::From(m).Sum();
                        })
                        .ToVector();

        BOOST_REQUIRE_EQUAL(sums.size(), 3);

        BOOST_REQUIRE_EQUAL(sums[0], 3);
        BOOST_REQUIRE_EQUAL(sums[1], 7);
        BOOST_REQUIRE_EQUAL(sums[2], 5);
    }
}

BOOST_AUTO_TEST_CASE(LinqCppCapacityTest)
{
    struct CapacityTest {
        size_t* _copyCalled;

        CapacityTest(size_t* copyCalled)
            : _copyCalled(copyCalled)
        {
        }

        CapacityTest(const CapacityTest& k)
        {
            _copyCalled = k._copyCalled;
            ++(*_copyCalled);
        }

        CapacityTest(CapacityTest&& k)
        {
            _copyCalled = k._copyCalled;
            ++(*_copyCalled);
        }

        CapacityTest& operator=(const CapacityTest& k)
        {
            _copyCalled = k._copyCalled;
            ++(*_copyCalled);
            return *this;
        }

        CapacityTest& operator=(CapacityTest&& k)
        {
            _copyCalled = k._copyCalled;
            ++(*_copyCalled);
            return *this;
        }
    };

    std::vector<CapacityTest> data;

    size_t copyCalled = 0;

    data.push_back(CapacityTest(&copyCalled));

    auto capacity = data.capacity() * 10;

    data.clear();

    BOOST_REQUIRE_NE(capacity, 0);

    for (size_t i = 0; i < capacity; ++i) {
        data.push_back(CapacityTest(&copyCalled));
    }
    copyCalled = 0;

    auto data2 = linq::From(data)
                     .ToVector();

    BOOST_REQUIRE_EQUAL(copyCalled, data.size());

    copyCalled = 0;
    auto expectedCopyCalled = data.size() + data2.size();

    linq::From(data)
        .Concat(data2)
        .ToVector();

    BOOST_REQUIRE_EQUAL(copyCalled, expectedCopyCalled);
}

BOOST_AUTO_TEST_CASE(LinqCppComplexTest)
{
    struct T1 {
        struct T2 {
            int v;

            ~T2() { v = 0; }
        };

        struct T3 {
            struct T4 {
                T2 td2;
            };
            T2 td2;
            std::vector<T4> td4;
        };
        boost::optional<T3> result;
    };

    auto getData = [] {
        return std::vector<T1> {
            { boost::optional<T1::T3>(T1::T3 {
                T1::T2 { 2 },
                { T1::T3::T4 { T1::T2 { 2 } }, T1::T3::T4 { T1::T2 { 2 } } } }) },
            { boost::optional<T1::T3>(T1::T3 {
                T1::T2 { 2 },
                { T1::T3::T4 { T1::T2 { 2 } }, T1::T3::T4 { T1::T2 { 1 } } } }) }
        };
    };

    auto result = linq::From(getData())
                      .Where([](const T1& m) { return !!m.result; })
                      .Select<const T1::T3&>([](const T1& m) -> const T1::T3& { return *m.result; })
                      .SelectMany<T1::T2>([](const T1::T3& m) {
                          return linq::From({ m.td2 })
                              .Concat(
                                  linq::From(m.td4)
                                      .Select<T1::T2>([](const T1::T3::T4& m) {
                                          return m.td2;
                                      }));
                      })
                      .Where([](const T1::T2& m) { return m.v == 1; })
                      .Select<int64_t>([](const T1::T2& m) { return m.v; })
                      .FirstOrNone();

    BOOST_REQUIRE_EQUAL(!!result, true);
    BOOST_REQUIRE_EQUAL(*result, 1);
}

BOOST_AUTO_TEST_CASE(LinqCppSkipTest)
{

    BOOST_REQUIRE_EQUAL(
        linq::From({ 1, 2, 3, 4, 5 })
            .Skip(2)
            .Sum(),
        12);
}

BOOST_AUTO_TEST_CASE(LinqCppTakeTest)
{

    BOOST_REQUIRE_EQUAL(
        linq::From({ 1, 2, 3, 4, 5 })
            .Take(2)
            .Sum(),
        3);

    BOOST_REQUIRE_EQUAL(
        linq::From({ 1, 2, 3, 4, 5 })
            .Skip(2)
            .Take(2)
            .Sum(),
        7);
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

BOOST_AUTO_TEST_CASE(LinqCppDistinctTest)
{
    {
        auto col11 = linq::From({ 1, 2, 3, 4, 5 }).Distinct();
        auto col12 = std::vector<int> { 1, 2, 3, 4, 5 };
        BOOST_REQUIRE_EQUAL_COLLECTIONS(col11.begin(), col11.end(), col12.begin(), col12.end());
    }

    {
        auto col21 = linq::From({ 1, 2, 3, 3, 4, 5 }).Distinct();
        auto col22 = std::vector<int> { 1, 2, 3, 4, 5 };
        BOOST_REQUIRE_EQUAL_COLLECTIONS(col21.begin(), col21.end(), col22.begin(), col22.end());
    }

    {
        auto col31 = linq::From({ 1, 2, 3, 4, 5, 5 }).Distinct();
        auto col32 = std::vector<int> { 1, 2, 3, 4, 5 };
        BOOST_REQUIRE_EQUAL_COLLECTIONS(col31.begin(), col31.end(), col32.begin(), col32.end());
    }

    {
        auto col41 = linq::From({ 1, 1, 2, 3, 4, 5 }).Distinct();
        auto col42 = std::vector<int> { 1, 2, 3, 4, 5 };
        BOOST_REQUIRE_EQUAL_COLLECTIONS(col41.begin(), col41.end(), col42.begin(), col42.end());
    }

    {
        auto col51 = linq::From({ 1, 1, 1, 1, 1, 1 }).Distinct();
        auto col52 = std::vector<int> { 1 };
        BOOST_REQUIRE_EQUAL_COLLECTIONS(col51.begin(), col51.end(), col52.begin(), col52.end());
    }

    {
        struct S {
            int V1;
            int V2;

            bool operator==(const S& m) const
            {
                return V1 == m.V1 && V2 == m.V2;
            }
        };

        auto col61 = linq::From({ S { 1, 2 }, S { 2, 3 }, S { 2, 3 }, S { 3, 4 } })
                         .Distinct([](const S& m) { return m.V1; })
                         .ToVector();
        auto col62 = std::vector<S> { S { 1, 2 }, S { 2, 3 }, S { 3, 4 } };

        BOOST_TEST_REQUIRE(col61 == col62);
    }
}

BOOST_AUTO_TEST_CASE(LinqCppCastTest)
{
    const std::vector<int> container = { 1, 2, 3 };
    BOOST_TEST_REQUIRE(linq::From(container)
                           .Cast<char>()
                           .Sum()
        == 6);
}

BOOST_AUTO_TEST_CASE(LinqCppIntersectTest)
{
    const std::vector<int> container1 = { 1, 2, 3 };
    const std::vector<int> container2 = { 4, 2, 5 };

    BOOST_TEST_REQUIRE(linq::From(container1)
                           .Intersect(container2)
                           .Sum()
        == 2);
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()
}
