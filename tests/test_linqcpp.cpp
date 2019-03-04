#include <linqcpp/linqcpp.h>

namespace test {
    BOOST_AUTO_TEST_SUITE(__main__)
    BOOST_AUTO_TEST_SUITE(linqcpp)

    struct TestType {
        int _t;

        TestType(int t)
                : _t(t) {}

        TestType() = default;

        TestType(TestType &&) = default;

        TestType(const TestType &) = delete;

        ~TestType() {
            _t = -1;
        }

        TestType &operator=(TestType &&) = default;
    };

    struct TestType2 {
        int _t;

        TestType2(int t)
                : _t(t) {}

        TestType2() = default;

        TestType2(TestType2 &&) = default;

        TestType2(const TestType2 &) = default;

        ~TestType2() {
            _t = -1;
        }

        TestType2 &operator=(TestType2 &&) = default;

        TestType2 &operator=(const TestType2 &) = default;

        TestType2 operator+(const TestType2 &t) {
            return TestType2(_t + t._t);
        }

        operator int() const {
            return _t;
        }
    };

    BOOST_AUTO_TEST_CASE(LinqCppInitializerListTest) {
            auto from = linq::Move({ TestType2(1), TestType2(1) });
            BOOST_REQUIRE_EQUAL(from.Sum(), 2);
            BOOST_REQUIRE_EQUAL(linq::Move({ 1, 2, 3, 4, 5 }).Count(), 5);
    }

    BOOST_AUTO_TEST_CASE(LinqCppConcatTest) {
            BOOST_REQUIRE_EQUAL(linq::Move({1, 2, 3, 4, 5}).Concat(linq::Move({1, 2, 3, 4, 5})).Count(), 10);
            BOOST_REQUIRE_EQUAL(linq::Move({ 1, 2, 3, 4, 5 }).Concat(linq::From(std::vector<int>())).Count(), 5);
            BOOST_REQUIRE_EQUAL(linq::From(std::vector<int>()).Concat(linq::From(std::vector<int>({ 1, 2, 3, 4, 5 }))).Count(), 5);
    }

    BOOST_AUTO_TEST_CASE(LinqCppContainerTest) {
            BOOST_REQUIRE_EQUAL(linq::From(std::list<int>({1, 2, 3, 4, 5})).Count(), 5);
            BOOST_REQUIRE_EQUAL(linq::From(std::vector<int>({ 1, 2, 3, 4, 5 })).Count(), 5);
            BOOST_REQUIRE_EQUAL(linq::From(linq::From(std::vector<int>({ 1, 2, 3, 4, 5 }))).Count(), 5);
    }

    BOOST_AUTO_TEST_CASE(LinqCppWhereTest) {
            BOOST_REQUIRE_EQUAL(
                    linq::Move({1, 2, 3, 4, 5})
                            .Where([](int i) { return i > 3; }).Count(), 2);
            BOOST_REQUIRE_EQUAL(
            linq::Move({ 1, 2, 3, 4, 5 })
            .Where([](const int &i) { return i > 3; }).Count(), 2);

            BOOST_REQUIRE_EQUAL(
            linq::Move({ TestType2(1), TestType2(2) })
            .Where([](const TestType2 &) { return true; })
            .Where([](const TestType2 &) { return true; })
            .Sum(), 3);
    }

    BOOST_AUTO_TEST_CASE(LinqCppSelectTest) {
            BOOST_REQUIRE_EQUAL(
                    linq::Move({TestType(1), {2}, {3}, {4}, {5}})
                            .Select<int>([](const TestType &s) { return s._t; })
                            .Sum(), 15);
    }

    BOOST_AUTO_TEST_CASE(LinqCppFromMoveTest) {
            std::vector<TestType> v(5);
            auto count = 0;
            for (auto &it: v) {
                it._t = ++count;
            }

            BOOST_REQUIRE_EQUAL(
            linq::From(v)
            .Where([](const TestType &) { return true; })
            .Concat(linq::From(std::vector<TestType>()))
            .Select<int>([](const TestType &t) { return t._t; })
            .Sum(), 15);
            BOOST_REQUIRE_EQUAL(v.size(), 5);

            auto from1 = linq::From(v).Select<int>([](const TestType &t) { return t._t; });
            auto from2 = from1;

            BOOST_REQUIRE_EQUAL(from1.Sum(), 15);
            // это не копипаст, это такой тест.
            BOOST_REQUIRE_EQUAL(from1.Sum(), 15);

            BOOST_REQUIRE_EQUAL(from2.Sum(), 15);

            std::vector<int> v2(5);
            count = 0;
            for (auto &it: v2) {
                it = ++count;
            }

            auto from3 = linq::From(std::move(v2));
            auto from4 = from3;

            BOOST_REQUIRE_EQUAL(from3.Sum(), 15);

            BOOST_REQUIRE_EQUAL(from4.Sum(), 15);

            BOOST_REQUIRE_EQUAL(
            linq::From(std::move(v))
            .Where([](const TestType &) { return true; })
            .Concat(linq::From(std::vector<TestType>()))
            .Select<int>([](const TestType &t) { return t._t; })
            .Concat(linq::From(std::vector<int>({ 2 })))
            .Exclude(linq::From(std::vector<int>()))
            .Exclude(linq::From(std::vector<int>({ 1 })))
            .Exclude(linq::From(std::vector<int>({ 100 })))
            .Sum(), 16);
            BOOST_REQUIRE_EQUAL(v.size(), 0);
    }

    BOOST_AUTO_TEST_CASE(LinqCppSelectManyTest) {
            BOOST_REQUIRE_EQUAL(
                    linq::From(std::vector<int>())
                            .SelectMany<int>([](int) {
                                return std::vector<int>();
                            }).Count(), 0);

            BOOST_REQUIRE_EQUAL(
            linq::Move({ std::vector<int>() })
            .SelectMany<int>([](const std::vector<int> &s) {
                return s;
            }).Sum(), 0);

            auto select = linq::Move({ std::vector<TestType2>({TestType2(1)}), std::vector<TestType2>({TestType2(1)}) })
            .SelectMany<TestType2>([](const std::vector<TestType2> &s) {
                return linq::From(std::vector<TestType2>(s)).Concat(std::vector<TestType2>({TestType2(1)}));
            }).Select<int>([](const TestType2 t) { return t._t; });

            std::vector<int> v_select;
            v_select.insert(v_select.begin(), select.begin(), select.end());

            BOOST_REQUIRE_EQUAL(
            linq::Move({ 1 })
            .SelectMany<TestType2>([](int) {
                return std::vector<TestType2>();
            }).Sum(), 0);

            BOOST_REQUIRE_EQUAL(
            linq::Move({ 1 })
            .SelectMany<TestType2>([](int i) {
                return std::vector<TestType2>({TestType2(i)});
            })
            .Where([](const TestType2 &t) { return t._t != -1; })
            // это не копипаст, это такой тест.
            .Where([](const TestType2 &t) { return t._t != -1; })
            .Sum(), 1);

            BOOST_REQUIRE_EQUAL(
            linq::Move({ 1 })
            .SelectMany<TestType2>([](int) {
                struct R {
                    std::vector<TestType2> r;
                };
                auto l = [] {
                    R r;
                    r.r = std::vector<TestType2>({TestType2(1)});
                    return r;
                };
                return std::move(l().r);
            })
            .Where([](const TestType2 &t) { return t._t != -1; })
            .Select<TestType2>([](const TestType2 &t) {
                TestType2 ret = t;
                return ret;
            })
            .Sum(), 1);

            BOOST_REQUIRE_EQUAL(linq::From(v_select).Sum(), 4);

            std::vector<TestType> v1(5);
            auto count = 0;
            for (auto &it: v1) {
                it._t = ++count;
            }

            std::vector<std::vector<TestType>> v2(1);
            *v2.begin() = std::move(v1);

            BOOST_REQUIRE_EQUAL(
            linq::From(std::move(v2))
            .SelectMany<int>([](const std::vector<TestType> &s) {
                return linq::From(s)
                        .Select<int>([](const TestType &s) { return s._t; }).ToVector();
            }).Sum(), 15);

    }

    BOOST_AUTO_TEST_CASE(LinqCppReuseTest) {

            auto from = linq::Move({ TestType(1), TestType(1) });
            BOOST_REQUIRE_EQUAL(from.Select<int>([](const TestType &t) { return t._t; }).Sum(), 2);
            // это не копипаст, это такой тест.
            BOOST_REQUIRE_EQUAL(from.Select<int>([](const TestType &t) { return t._t; }).Sum(), 2);
    }

    BOOST_AUTO_TEST_CASE(LinqCppThrottleTest) {

            auto from = linq::Move({ TestType2(1), TestType2(1) });
            auto count = 0;
            for (const auto &it : from.Throttle(1)) {
                BOOST_REQUIRE_EQUAL(linq::From(it).Count(), 1);
                ++count;
            }
            BOOST_REQUIRE_EQUAL(count, 2);

            auto throttle = linq::Move({ 1, 2, 3, 4, 5 }).Throttle(2);
            for (auto i = 0; i < 2; ++i){
                auto sums = throttle
                        .Select<size_t>([](const std::vector<int> &m) {
                            return linq::From(m).Sum();
                        }).ToVector();

                BOOST_REQUIRE_EQUAL(sums.size(), 3);

                BOOST_REQUIRE_EQUAL(sums[0], 3);
                BOOST_REQUIRE_EQUAL(sums[1], 7);
                BOOST_REQUIRE_EQUAL(sums[2], 5);
            }
    }

    BOOST_AUTO_TEST_SUITE_END()

    BOOST_AUTO_TEST_SUITE_END()
}
