//
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

namespace linq {
template <typename P>
struct StdShim;
template <typename P>
struct _shared_getter;
template <typename I>
struct ItShim;
template <typename P>
struct Shim;

template <typename P>
Shim<P> From(Shim<P>&& p) { return std::move(p); }
template <typename P>
StdShim<P> From(StdShim<P>&& p) { return std::move(p); }
template <typename P>
StdShim<P> From(P&& p) { return StdShim<P> { { _shared_getter<P> { std::forward<P>(p) } } }; }
template <typename I>
Shim<ItShim<I>> From(I&& b, I&& e, size_t capacity) { return Shim<ItShim<I>> { { std::forward<I>(b), std::forward<I>(e), capacity } }; }
template <typename P>
StdShim<const P&> From(const P& p) { return StdShim<const P&> { { _shared_getter<const P&> { p } } }; }

template <typename T>
StdShim<std::vector<T>> From(std::initializer_list<T>&& t)
{
    return From(std::vector<T>(std::forward<std::initializer_list<T>>(t)));
}

namespace details {
    template <class T>
    using optional = boost::optional<T>;

    template <typename P>
    size_t get_capacity(const P&) { return 0; }
    template <typename T>
    size_t get_capacity(const std::list<T>& l) { return l.size(); }
    template <typename T>
    size_t get_capacity(const std::vector<T>& v) { return v.size(); }
    template <typename T>
    size_t get_capacity(const std::unordered_set<T>& s) { return s.size(); }
    template <typename K, typename T>
    size_t get_capacity(const std::unordered_map<K, T>& m) { return m.size(); }

    template <class T>
    struct unwrap_reference {
        using type = T;
    };
    template <class U>
    struct unwrap_reference<std::reference_wrapper<U>> {
        using type = U&;
    };
    template <class T>
    using unwrap_reference_t = typename unwrap_reference<T>::type;
}

template <typename I>
class ITearOffContainer;

template <typename P>
struct Shim : public P {
    using value_type = typename P::value_type;
    using value_type_t = std::remove_const_t<std::remove_reference_t<std::remove_const_t<value_type>>>;

    template <typename I1, typename I2>
    struct LoIterator1 {
        using pointer = typename I2::pointer;
        using reference = typename I2::reference;
        using value_type = typename I2::value_type;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = typename I2::difference_type;
        mutable I2 _i;
        I1& operator++()
        {
            ++_i;
            return (I1&)*this;
        }
        I1 operator++(int)
        {
            auto ret = (I1&)*this;
            ++(I1&)*this;
            return ret;
        }
        decltype(*_i) operator*() { return *_i; }
        decltype(*_i) operator*() const { return *_i; }
        bool operator==(const I1& i) const { return _i == i._i; }
        bool operator!=(const I1& i) const { return !((const I1&)*this == i); }
    };

    template <typename I1, typename I2>
    struct LoIterator2 : public LoIterator1<I1, I2> {
        I2 _e;
    };

    struct LoShim {
        P _p;
        constexpr static bool shared = P::shared;
        using iterator = decltype(_p.begin());
        using value_type = typename P::value_type;
        using const_iterator = typename P::const_iterator;
        size_t get_capacity() const { return _p.get_capacity(); }
    };

    template <typename P2>
    struct HiShim1 : public P2 {
        using iterator = typename P2::iterator;
        using const_iterator = typename P2::const_iterator;
        iterator end() { return { { this->_p.end() } }; }
        iterator begin() { return { { this->_p.begin() } }; }
        const_iterator end() const { return { { this->_p.end() } }; }
        const_iterator begin() const { return { { this->_p.begin() } }; }
    };

    template <typename P2>
    struct HiShim11 : public P2 {
        using iterator = typename P2::iterator;
        using const_iterator = typename P2::const_iterator;
        iterator end() { return { { this->_p.end() }, this }; }
        iterator begin() { return { { this->_p.begin() }, this }; }
        const_iterator end() const { return { { this->_p.end() }, this }; }
        const_iterator begin() const { return { { this->_p.begin() }, this }; }
    };

    template <typename P2>
    struct HiShim2 : public P2 {
        using iterator = typename P2::iterator;
        using const_iterator = typename P2::const_iterator;
        iterator end() { return { { { this->_p.end() }, this->_p.end() }, this }; }
        iterator begin() { return { { { this->_p.begin() }, this->_p.end() }, this }; }
        const_iterator end() const { return { { { this->_p.end() }, this->_p.end() }, this }; }
        const_iterator begin() const { return { { { this->_p.begin() }, this->_p.end() }, this }; }
    };

    template <typename P2>
    struct HiShim22 : public P2 {
        using iterator = typename P2::iterator;
        using const_iterator = typename P2::const_iterator;
        iterator end() { return { { this->_p.end(), this->_p.end(), this } }; }
        iterator begin() { return { { this->_p.begin(), this->_p.end(), this } }; }
        const_iterator end() const { return { { this->_p.end(), this->_p.end(), this } }; }
        const_iterator begin() const { return { { this->_p.begin(), this->_p.end(), this } }; }
    };

    template <typename F>
    struct WhereShim : public LoShim {
        F _f;
        using base = LoShim;
        template <typename I>
        struct Iterator : public LoIterator2<Iterator<I>, I> {
            const WhereShim* _o = nullptr;
            using base = LoIterator2<Iterator<I>, I>;
            bool _init = _chk();
            Iterator& operator++()
            {
                base::operator++();
                _chk();
                return *this;
            }
            bool _chk()
            {
                for (; base::_i != base::_e && !_o->_f(*base::_i); ++base::_i) {
                }
                return true;
            }
        };
        using iterator = Iterator<typename base::iterator>;
        using const_iterator = Iterator<typename base::const_iterator>;
    };
    template <typename F>
    Shim<HiShim2<WhereShim<F>>> Where(F&& f) { return Shim<HiShim2<WhereShim<F>>> { { { { *this }, std::forward<F>(f) } } }; }
    template <typename F>
    const Shim<HiShim2<WhereShim<F>>> Where(F&& f) const { return Shim<HiShim2<WhereShim<F>>> { { { { *this }, std::forward<F>(f) } } }; }

    template <typename V2, typename F>
    struct SelectShim : public LoShim {
        F _f;
        using base = LoShim;
        template <typename I>
        struct Iterator : public LoIterator1<Iterator<I>, I> {
            const SelectShim* _o = nullptr;
            using value_type = V2;
            using pointer = std::add_pointer_t<value_type>;
            using reference = std::add_lvalue_reference_t<value_type>;
            V2 operator*() { return (details::unwrap_reference_t<decltype(_o->_f(*this->_i))>)_o->_f(*this->_i); }
            V2 operator*() const { return (details::unwrap_reference_t<decltype(_o->_f(*this->_i))>)_o->_f(*this->_i); }
        };
        using iterator = Iterator<typename base::iterator>;
        using value_type = typename iterator::value_type;
        using const_iterator = Iterator<typename base::const_iterator>;
    };
    template <typename V2, typename F>
    Shim<HiShim11<SelectShim<V2, F>>> Select(F&& f) { return Shim<HiShim11<SelectShim<V2, F>>> { { { { *this }, std::forward<F>(f) } } }; }
    template <typename V2, typename F>
    const Shim<HiShim11<SelectShim<V2, F>>> Select(F&& f) const { return Shim<HiShim11<SelectShim<V2, F>>> { { { { *this }, std::forward<F>(f) } } }; }

    template <typename V2, typename F>
    struct SelectManyShim : public LoShim {
        F _f;
        using base = LoShim;
        template <typename I>
        struct Iterator : public LoIterator2<Iterator<I>, I> {
            using P2 = decltype((details::unwrap_reference_t<decltype(_f(*std::declval<I>()))>)_f(*std::declval<I>()));
            const SelectManyShim* _o = nullptr;
            details::optional<P2> _p2;
            using I2 = decltype(_p2->begin());
            using value_type = V2;
            using pointer = std::add_pointer_t<value_type>;
            using reference = std::add_lvalue_reference_t<value_type>;
            I2 _i2;
            I2 _e2;
            bool _f = true;
            void _ensh() const
            {
                if (_f) {
                    const_cast<Iterator*>(this)->_chk();
                }
            }
            Iterator& operator++()
            {
                _ensh();
                ++_i2;
                _chk();
                return *this;
            }
            decltype(*_i2) operator*()
            {
                _ensh();
                return *_i2;
            }
            decltype(*_i2) operator*() const
            {
                _ensh();
                return *_i2;
            }
            bool operator==(const Iterator& i) const
            {
                _ensh();
                return this->_i == i._i;
            }
            void _chk()
            {
                for (; this->_i != this->_e && (_f || _i2 == _e2);) {
                    if (!_f) {
                        ++this->_i;
                        if (this->_i == this->_e) {
                            break;
                        }
                    }
                    _p2.emplace((details::unwrap_reference_t<decltype(_o->_f(*this->_i))>)_o->_f(*this->_i));
                    _i2 = _p2->begin();
                    _e2 = _p2->end();
                    _f = false;
                }
            }
        };
        using iterator = Iterator<typename base::iterator>;
        using value_type = typename iterator::value_type;
        using const_iterator = Iterator<typename base::const_iterator>;
    };
    template <typename V2, typename F>
    Shim<HiShim2<SelectManyShim<V2, F>>> SelectMany(F&& f) { return Shim<HiShim2<SelectManyShim<V2, F>>> { { { { *this }, std::forward<F>(f) } } }; }
    template <typename V2, typename F>
    const Shim<HiShim2<SelectManyShim<V2, F>>> SelectMany(F&& f) const { return Shim<HiShim2<SelectManyShim<V2, F>>> { { { { *this }, std::forward<F>(f) } } }; }

    template <typename P2>
    struct ConcatShim : public LoShim {
        P2 _p2;
        using P2Type = std::remove_reference_t<P2>;
        using base = LoShim;
        template <typename I1, typename I2>
        struct Iterator : public LoIterator2<Iterator<I1, I2>, I1> {
            using base = LoIterator2<Iterator<I1, I2>, I1>;
            I2 _i2;
            bool _f = base::_i != base::_e;
            bool operator!=(const Iterator& i) const { return !(*this == i); }
            bool operator==(const Iterator& i) const { return _f == i._f && (_f ? base::_i == i._i : _i2 == i._i2); }
            Iterator& operator++()
            {
                if (_f && ++base::_i != base::_e) {
                    return *this;
                }
                if (!_f) {
                    ++_i2;
                }
                _f = false;
                return *this;
            }

            using V1 = decltype(*base::_i);
            using V2 = decltype(*_i2);

            using RV = std::conditional_t<!std::is_reference<V1>::value && std::is_reference<V2>::value, V1,
                std::conditional_t<std::is_const<std::remove_reference_t<V1>>::value, V1, V2>>;
            RV operator*()
            {
                if (_f) {
                    return *base::_i;
                }
                return *_i2;
            }
            RV operator*() const
            {
                if (_f) {
                    return *base::_i;
                }
                return *_i2;
            }
        };

        using iterator = Iterator<typename base::iterator, typename P2Type::iterator>;
        using const_iterator = Iterator<typename base::const_iterator, typename P2Type::const_iterator>;
        size_t get_capacity() const { return base::get_capacity() + From(_p2).get_capacity(); }
        iterator begin() { return { { { base::_p.begin() }, base::_p.end() }, _p2.begin() }; }
        iterator end() { return { { { base::_p.end() }, base::_p.end() }, _p2.end(), false }; }
        const_iterator begin() const { return { { { base::_p.begin() }, base::_p.end() }, _p2.begin() }; }
        const_iterator end() const { return { { { base::_p.end() }, base::_p.end() }, _p2.end(), false }; }
    };
    template <typename P2>
    Shim<ConcatShim<P2>> Concat(P2&& p) { return Shim<ConcatShim<P2>> { { { *this }, std::forward<P2>(p) } }; }
    template <typename P2>
    const Shim<ConcatShim<P2>> Concat(P2&& p) const { return Shim<ConcatShim<P2>> { { { *this }, std::forward<P2>(p) } }; }

    template <typename F>
    struct ExcludeShim : public LoShim {
        using base = LoShim;
        std::unordered_set<std::result_of_t<F(value_type)>> _set;
        F _f;
        template <typename I>
        struct Iterator : public LoIterator2<Iterator<I>, I> {
            using base = LoIterator2<Iterator, I>;
            const ExcludeShim* _o = nullptr;
            bool _init = _chk();
            Iterator& operator++()
            {
                ++base::_i;
                _chk();
                return *this;
            }
            bool _chk()
            {
                for (; base::_i != base::_e && _o->_set.find(_o->_f(*base::_i)) != _o->_set.end(); ++base::_i) {
                }
                return true;
            }
        };
        using iterator = Iterator<typename base::iterator>;
        using const_iterator = Iterator<typename base::const_iterator>;
    };
    template <typename P2, typename F>
    Shim<HiShim2<ExcludeShim<F>>> Exclude(P2&& p, F&& f)
    {
        return Shim<HiShim2<ExcludeShim<F>>> { { { { *this }, From(p).ToUnorderedSet(), std::forward<F>(f) } } };
    }
    template <typename P2, typename F>
    const Shim<HiShim2<ExcludeShim<F>>> Exclude(P2&& p, F&& f) const
    {
        return Shim<HiShim2<ExcludeShim<F>>> { { { { *this }, From(p).ToUnorderedSet(), std::forward<F>(f) } } };
    }
    struct _ExcludeFunctor {
        value_type_t operator()(const value_type& v) const { return v; }
    };
    template <typename P2>
    Shim<HiShim2<ExcludeShim<_ExcludeFunctor>>> Exclude(P2&& p)
    {
        return Exclude(p, _ExcludeFunctor { _ExcludeFunctor {} });
    }
    template <typename P2>
    const Shim<HiShim2<ExcludeShim<_ExcludeFunctor>>> Exclude(P2&& p) const
    {
        return Exclude(p, _ExcludeFunctor { _ExcludeFunctor {} });
    }

    template <typename F>
    struct IntersectShim : public LoShim {
        using base = LoShim;
        std::unordered_set<value_type> _set;
        F _f;
        template <typename I>
        struct Iterator : public LoIterator2<Iterator<I>, I> {
            using base = LoIterator2<Iterator<I>, I>;
            const IntersectShim* _o = nullptr;
            bool _init = _chk();
            Iterator& operator++()
            {
                base::operator++();
                _chk();
                return *this;
            }
            bool _chk()
            {
                for (; base::_i != base::_e && _o->_set.find(_o->_f(*base::_i)) == _o->_set.end(); ++base::_i) {
                }
                return true;
            }
        };
        using iterator = Iterator<typename base::iterator>;
        using const_iterator = Iterator<typename base::const_iterator>;
    };
    template <typename P2, typename F>
    Shim<HiShim2<IntersectShim<F>>> Intersect(P2&& p, F&& f)
    {
        return Shim<HiShim2<IntersectShim<F>>> { { { { *this }, From(p).ToUnorderedSet(), std::forward<F>(f) } } };
    }
    template <typename P2, typename F>
    const Shim<HiShim2<IntersectShim<F>>> Intersect(P2&& p, F&& f) const
    {
        return Shim<HiShim2<IntersectShim<F>>> { { { { *this }, From(p).ToUnorderedSet(), std::forward<F>(f) } } };
    }
    struct _IntersectFunctor {
        value_type_t operator()(const value_type& v) const { return v; }
    };
    template <typename P2>
    Shim<HiShim2<IntersectShim<_IntersectFunctor>>> Intersect(P2&& p)
    {
        return Intersect(p, _IntersectFunctor { _IntersectFunctor {} });
    }
    template <typename P2>
    const Shim<HiShim2<IntersectShim<_IntersectFunctor>>> Intersect(P2&& p) const
    {
        return Intersect(p, _IntersectFunctor { _IntersectFunctor {} });
    }

    template <typename V2>
    struct CastShim : public LoShim {
        using base = LoShim;

        template <typename I>
        struct Iterator : public LoIterator1<Iterator<I>, I> {
            using base = LoIterator1<Iterator, I>;
            using value_type = V2;
            using pointer = std::add_pointer_t<value_type>;
            using reference = std::add_lvalue_reference_t<value_type>;
            V2 operator*() { return static_cast<V2>(base::operator*()); }
            V2 operator*() const { return static_cast<V2>(base::operator*()); }
        };
        using iterator = Iterator<typename base::iterator>;
        using value_type = typename iterator::value_type;
        using const_iterator = Iterator<typename base::const_iterator>;
    };
    template <typename V2>
    Shim<HiShim1<CastShim<V2>>> Cast() { return Shim<HiShim1<CastShim<V2>>> { { { { *this } } } }; }
    template <typename V2>
    const Shim<HiShim1<CastShim<V2>>> Cast() const { return Shim<HiShim1<CastShim<V2>>> { { { { *this } } } }; }

    struct TakeShim : public LoShim {
        using base = LoShim;
        size_t _v = 0;
        template <typename I>
        struct Iterator : public LoIterator2<Iterator<I>, I> {
            const TakeShim* _o = nullptr;
            size_t _v = _o->_v;
            using base = LoIterator2<Iterator<I>, I>;
            Iterator& operator++()
            {
                --_v;
                return base::operator++();
            }
            bool operator==(const Iterator& i) const { return (_v <= 0 && i._i == base::_e) ? true : base::operator==(i); }
        };
        using iterator = Iterator<typename base::iterator>;
        using const_iterator = Iterator<typename base::const_iterator>;
    };
    Shim<HiShim2<TakeShim>> Take(size_t value) { return Shim<HiShim2<TakeShim>> { { { { *this }, value } } }; }
    const Shim<HiShim2<TakeShim>> Take(size_t value) const { return Shim<HiShim2<TakeShim>> { { { { *this }, value } } }; }

    struct ThrottleShim : public LoShim {
        using base = LoShim;
        size_t _v = 0;

        template <typename I>
        struct Iterator : public LoIterator2<Iterator<I>, I> {
            using base = LoIterator2<Iterator<I>, I>;
            const ThrottleShim* _o = nullptr;
            Iterator& operator++()
            {
                for (auto v = _o->_v; base::_i != base::_e && v > 0; ++base::_i, --v) {
                }
                return *this;
            }
            decltype(From(base::_i, base::_e, 0).Take(0)) operator*() { return From(base::_i, base::_e, _o->_v).Take(_o->_v); }
            decltype(From(base::_i, base::_e, 0).Take(0)) operator*() const { return From(base::_i, base::_e, _o->_v).Take(_o->_v); }
        };

        using iterator = Iterator<typename base::iterator>;
        using const_iterator = Iterator<typename base::const_iterator>;
    };
    Shim<HiShim2<ThrottleShim>> Throttle(size_t value) { return Shim<HiShim2<ThrottleShim>> { { { { *this }, value } } }; }
    const Shim<HiShim2<ThrottleShim>> Throttle(size_t value) const { return Shim<HiShim2<ThrottleShim>> { { { { *this }, value } } }; }

    template <typename F>
    struct DistinctShim : public LoShim {
        using base = LoShim;
        F _f;
        mutable std::unordered_set<std::result_of_t<F(value_type)>> _set;
        template <typename I>
        struct Iterator : public LoIterator2<Iterator<I>, I> {
            const DistinctShim* _o = nullptr;
            using base = LoIterator2<Iterator<I>, I>;
            Iterator& operator++()
            {
                _o->_set.insert(_o->_f(*base::_i));
                base::operator++();
                _chk();
                return *this;
            }
            void _chk()
            {
                for (; base::_i != base::_e && _o->_set.find(_o->_f(*base::_i)) != _o->_set.end(); ++base::_i) {
                }
            }
        };
        using iterator = Iterator<typename base::iterator>;
        using const_iterator = Iterator<typename base::const_iterator>;
    };
    template <typename F>
    Shim<HiShim2<DistinctShim<F>>> Distinct(F&& f) { return Shim<HiShim2<DistinctShim<F>>> { { { { *this }, std::forward<F>(f) } } }; }
    template <typename F>
    const Shim<HiShim2<DistinctShim<F>>> Distinct(F&& f) const { return Shim<HiShim2<DistinctShim<F>>> { { { { *this }, std::forward<F>(f) } } }; }
    struct _DistinctFunctor {
        value_type_t operator()(const value_type& v) const { return v; }
    };
    Shim<HiShim2<DistinctShim<_DistinctFunctor>>> Distinct()
    {
        return Distinct(_DistinctFunctor { _DistinctFunctor {} });
    }
    const Shim<HiShim2<DistinctShim<_DistinctFunctor>>> Distinct() const
    {
        return Distinct(_DistinctFunctor { _DistinctFunctor {} });
    }

    struct _SkipFunctor {
        mutable size_t _c;
        bool operator()(const value_type&) const
        {
            if (_c == 0) {
                return true;
            }
            --_c;
            return false;
        }
    };
    Shim<HiShim2<WhereShim<_SkipFunctor>>> Skip(size_t c) const
    {
        return this->Where(_SkipFunctor { c });
    }

    template <typename VT = value_type>
    details::optional<VT> FirstOrNone()
    {
        auto _this = this;
        auto ret = _this->begin();
        if (ret == _this->end()) {
            return details::optional<VT>();
        }
        return *ret;
    }

    template <typename VT = value_type>
    details::optional<VT> FirstOrNone() const
    {
        auto _this = this;
        auto ret = _this->begin();
        if (ret == _this->end()) {
            return details::optional<VT>();
        }
        return *ret;
    }

    template <typename VT = value_type, typename F>
    details::optional<VT> FirstOrNone(const F& f)
    {
        auto _this = this;
        return _this->template Where(f).template FirstOrNone<VT>();
    }

    template <typename VT = value_type>
    details::optional<VT> LastOrNone()
    {
        auto _this = this;
        auto ret = _this->begin();
        for (auto it = ret; it != _this->end(); ++it) {
            ret = it;
        }
        if (ret == _this->end()) {
            return details::optional<VT>();
        }
        return *ret;
    }

    template <typename VT = value_type, typename F>
    details::optional<VT> LastOrNone(const F& f)
    {
        auto _this = this;
        return _this->template Where(f).template LastOrNone<VT>();
    }

    template <typename VT = value_type>
    VT First() noexcept(false)
    {
        auto ret = FirstOrNone<VT>();
        if (!ret) {
            throw std::out_of_range("The element isn't found.");
        }
        return *ret;
    }

    template <typename VT = value_type>
    VT First() const noexcept(false)
    {
        auto ret = FirstOrNone<VT>();
        if (!ret) {
            throw std::out_of_range("The element isn't found.");
        }
        return *ret;
    }

    template <typename VT = value_type, typename F>
    VT First(const F& f)
    {
        auto _this = this;
        return _this->template Where(f).template First<VT>();
    }

    template <typename VT = value_type>
    VT Last() noexcept(false)
    {
        auto ret = LastOrNone<VT>();
        if (!ret) {
            throw std::out_of_range("The element isn't found.");
        }
        return *ret;
    }

    template <typename VT = value_type, typename F>
    VT Last(const F& f)
    {
        auto _this = this;
        return _this->template Where(f).template Last<VT>();
    }

    template <typename T>
    static size_t Count(T* _this)
    {
        size_t ret = 0;
        for (const auto& it : *_this) {
            (void)it;
            ++ret;
        }
        return ret;
    }
    size_t Count() { return Count(this); }
    size_t Count() const { return Count(this); }

    template <typename T>
    static value_type_t Sum(T* _this)
    {
        value_type_t ret = {};
        for (const auto& it : *_this) {
            ret = (value_type_t)(ret + it);
        }
        return ret;
    }
    value_type_t Sum() { return Sum(this); }
    value_type_t Sum() const { return Sum(this); }

    details::optional<value_type> MinOrNone()
    {
        auto _this = this;
        details::optional<value_type> ret;

        for (const auto& it : *_this) {
            if (!ret || *ret > it) {
                ret = it;
            }
        }
        return ret;
    }

    value_type Min() noexcept(false)
    {
        auto ret = MinOrNone();
        if (!ret) {
            throw std::range_error("Sequence contains no elements.");
        }
        return *ret;
    }

    details::optional<value_type> MaxOrNone()
    {
        auto _this = this;
        details::optional<value_type> ret;

        for (const auto& it : *_this) {
            if (!ret || *ret < it) {
                ret = it;
            }
        }
        return ret;
    }

    value_type Max() noexcept(false)
    {
        auto ret = MaxOrNone();
        if (!ret) {
            throw std::range_error("Sequence contains no elements.");
        }
        return *ret;
    }

    bool Any() const
    {
        auto _this = this;
        for (const auto& it : *_this) {
            (void)it;
            return true;
        }
        return false;
    }

    template <typename F>
    bool Any(F f) const
    {
        auto _this = this;
        return _this->Where(f).Any();
    }

    bool Contains(const value_type& t) const
    {
        auto _this = this;
        for (const auto& it : *_this) {
            if (it == t) {
                return true;
            }
        }
        return false;
    }

    template <typename C>
    bool IsIntersect(C&& c) const
    {
        auto _this = this;
        for (const auto& it1 : *_this) {
            for (const auto& it2 : c) {
                if (it1 == it2) {
                    return true;
                }
            }
        }
        return false;
    }

    template <typename F>
    bool All(F f) const
    {
        auto _this = this;
        for (const auto& it : *_this) {
            if (!f(it)) {
                return false;
            }
        }
        return true;
    }

    std::list<value_type> ToList()
    {
        std::list<value_type> ret;
        auto _this = this;
        for (auto&& it : *_this) {
            ret.emplace_back(it);
        }
        return ret;
    }

    void ToVector(std::vector<value_type>& v)
    {
        auto _this = this;
        for (auto&& it : *_this) {
            v.emplace_back(it);
        }
    }

    std::vector<value_type> ToVector()
    {
        std::vector<value_type> ret;
        ret.reserve(this->get_capacity());
        ToVector(ret);
        return ret;
    }

    void ToVector(std::vector<value_type>& v) const
    {
        auto _this = this;
        for (auto&& it : *_this) {
            v.emplace_back(it);
        }
    }

    std::vector<value_type> ToVector() const
    {
        std::vector<value_type> ret;
        ret.reserve(this->get_capacity());
        ToVector(ret);
        return ret;
    }

    std::vector<value_type> ToVector(size_t n)
    {
        std::vector<value_type> ret;
        ret.reserve(n);
        ToVector(ret);
        return ret;
    }

    std::vector<value_type> ToOrderedVector()
    {
        auto _this = this;
        auto ret = _this->ToVector();
        std::sort(ret.begin(), ret.end());
        return ret;
    }

    std::vector<value_type> ToOrderedVector(size_t n)
    {
        auto _this = this;
        auto ret = _this->ToVector(n);
        std::sort(ret.begin(), ret.end());
        return ret;
    }

    template <typename F>
    std::vector<value_type> ToOrderedVector(F f)
    {
        auto _this = this;
        auto ret = _this->ToVector();
        std::sort(ret.begin(), ret.end(), f);
        return ret;
    }

    template <typename F>
    std::vector<value_type> ToOrderedVector(F f, size_t n)
    {
        auto _this = this;
        auto ret = _this->ToVector(n);
        std::sort(ret.begin(), ret.end(), f);
        return ret;
    }

    template <typename K, typename V2, typename KS, typename VS>
    std::unordered_map<K, V2> ToUnorderedMap(KS keySelector, VS valueSelector)
    {
        std::unordered_map<K, V2> ret;
        auto _this = this;
        for (auto&& it : *_this) {
            valueSelector(it, ret[keySelector(it)]);
        }
        return ret;
    }

    template <typename K, typename KS>
    std::unordered_map<K, value_type> ToUnorderedMap(KS keySelector)
    {
        return ToUnorderedMap<K, value_type>(keySelector, [](const value_type& v1, value_type& v2) { v2 = v1; });
    }

    std::unordered_set<value_type> ToUnorderedSet()
    {
        std::unordered_set<value_type> ret;
        auto _this = this;
        for (auto&& it : *_this) {
            ret.emplace(it);
        }
        return ret;
    }

    template <typename K, typename F>
    std::unordered_set<K> ToUnorderedSet(F&& f)
    {
        auto _this = this;
        return _this->template Select<K>(f).ToUnorderedSet();
    }

    std::unordered_set<value_type> ToUnorderedSet() const
    {
        std::unordered_set<value_type> ret;
        auto _this = this;
        for (auto&& it : *_this) {
            ret.emplace(it);
        }
        return ret;
    }

    template <typename K, typename F>
    std::unordered_set<K> ToUnorderedSet(F&& f) const
    {
        auto _this = this;
        return _this->template Select<K>(f).ToUnorderedSet();
    }

    struct _MoveFunctor {
        value_type_t operator()(value_type& v) const { return std::move(v); }
        value_type_t operator()(value_type&& v) const { return std::move(v); }
    };
    Shim<HiShim11<SelectShim<value_type_t, _MoveFunctor>>> Move()
    {
        return Shim<HiShim11<SelectShim<value_type_t, _MoveFunctor>>> { { { { *this }, _MoveFunctor {} } } };
    }

    std::shared_ptr<ITearOffContainer<value_type>> AsTearOffContainer();
    std::shared_ptr<ITearOffContainer<value_type>> AsTearOffContainer() const;
};

template <typename P>
struct _shared_getter {
    std::shared_ptr<P> _p;

    using iterator = decltype(_p->begin());
    using value_type = typename P::value_type;
    using const_iterator = typename P::const_iterator;

    constexpr static bool shared = true;

    explicit _shared_getter(P&& p)
        : _p { std::make_shared<P>(std::forward<P>(p)) }
    {
    }
    _shared_getter(_shared_getter&&) = default;
    _shared_getter(const _shared_getter&) = default;
    _shared_getter& operator=(_shared_getter&&) = default;
    _shared_getter& operator=(const _shared_getter&) = default;

    size_t get_capacity() const { return details::get_capacity(*_p); }
    iterator end() { return _p->end(); }
    iterator begin() { return _p->begin(); }
    const_iterator end() const { return _p->end(); }
    const_iterator begin() const { return _p->begin(); }
};

template <typename P>
struct _shared_getter<P&> {
    P* _p = nullptr;

    using iterator = decltype(_p->begin());
    using value_type = typename P::value_type;
    using const_iterator = typename P::const_iterator;

    constexpr static bool shared = false;

    explicit _shared_getter(P& p)
        : _p(&p)
    {
    }

    ~_shared_getter()
    {
        _p = nullptr;
    }

    _shared_getter(_shared_getter&&) = default;
    _shared_getter(const _shared_getter&) = default;
    _shared_getter& operator=(_shared_getter&&) = default;
    _shared_getter& operator=(const _shared_getter&) = default;

    size_t get_capacity() const { return details::get_capacity(*_p); }
    iterator end() { return _p->end(); }
    iterator begin() { return _p->begin(); }
    const_iterator end() const { return _p->end(); }
    const_iterator begin() const { return _p->begin(); }
};

template <typename P>
struct StdShim : public Shim<_shared_getter<P>> {
    using base = Shim<_shared_getter<P>>;
};

template <typename I>
struct ItShim {
    using iterator = std::remove_reference_t<I>;
    using const_iterator = iterator;
    using value_type = typename iterator::value_type;

    iterator _b;
    iterator _e;
    size_t _capacity;

    iterator end() { return _e; }
    iterator begin() { return _b; }
    const_iterator end() const { return _e; }
    const_iterator begin() const { return _b; }
    size_t get_capacity() const { return _capacity; }
};

// used only for tests!!!
template <typename T>
StdShim<std::vector<T>> Move(std::initializer_list<T>&& t)
{
    std::vector<T> vec(t.size());
    std::move(const_cast<T*>(t.begin()), const_cast<T*>(t.end()), vec.begin());
    return linq::From(std::move(vec));
}
}
