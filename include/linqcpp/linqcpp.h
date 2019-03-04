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

#include <list>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace linq {
    ////////////////////////////////////////////////////////////////
    // Enumerator template
    ////////////////////////////////////////////////////////////////
    template<typename P, typename T, typename V = decltype(*((typename std::remove_reference<P>::type *) nullptr)->begin())>
    class Shim {
    protected:
        P _c;
    public:
        typedef V value_type;
        typedef decltype(((typename std::remove_reference<P>::type *) nullptr)->begin()) mutable_iterator;

        typedef mutable_iterator iterator;
        typedef mutable_iterator const_iterator;

        typedef typename std::remove_const<typename std::remove_reference<value_type>::type>::type value_type_t;

        Shim() = default;

        Shim(Shim &&) = default;

        Shim(const Shim &) = default;

        Shim &operator=(Shim &&) = default;

        Shim &operator=(const Shim &) = default;

        explicit Shim(P &&p)
                : _c(std::forward<P>(p)) {
        }

        explicit Shim(const P &p)
                : _c(p) {
        }

        auto end() -> decltype(this->_c.end()) {
            return this->_c.end();
        }

        auto begin() -> decltype(this->_c.begin()) {
            return this->_c.begin();
        }

        auto end() const -> decltype(this->_c.end()) {
            return this->_c.end();
        }

        auto begin() const -> decltype(this->_c.begin()) {
            return this->_c.begin();
        }

        template<typename TI, typename I, typename IV = decltype(**(I *) nullptr)>
        struct IteratorBase : public std::iterator<std::forward_iterator_tag, typename std::remove_reference<IV>::type> {
            I _i;
            I _e;

            IteratorBase(const I &i, const I &e)
                    : _i(i), _e(e) {
            }

            IteratorBase() = default;

            IteratorBase(IteratorBase &&) = default;

            IteratorBase(const IteratorBase &) = default;

            IteratorBase &operator=(IteratorBase &&) = default;

            IteratorBase &operator=(const IteratorBase &) = default;

            bool operator==(const TI &i) const {
                return _i == i._i;
            }

            bool operator!=(const TI &i) const {
                return !((TI &) *this == i);
            }

            TI operator++() {
                ++_i;
                return (TI &) *this;
            }

            TI operator++(int) {
                TI ret = (TI &) *this;
                ++(TI & ) * this;
                return ret;
            }

            IV operator*() {
                return *_i;
            }
        };

        template<typename F>
        class WhereShim : public Shim<T, WhereShim<F>> {
            using base = Shim<T, WhereShim<F>>;
            F _f;
        public:
            template<typename I>
            struct Iterator : public IteratorBase<Iterator<I>, I> {
                const WhereShim &_o;

                Iterator(const I &i, const I &e, const WhereShim &o)
                        : IteratorBase<Iterator<I>, I>(i, e), _o(o) {
                    _chk();
                }

                Iterator() = default;

                Iterator(Iterator &&) = default;

                Iterator(const Iterator &) = default;

                Iterator &operator=(Iterator &&) = default;

                Iterator &operator=(const Iterator &) = default;

                Iterator operator++() {
                    ++this->_i;
                    _chk();
                    return *this;
                }

                void _chk() {
                    for (; this->_i != this->_e && !_o._f(*this->_i); ++this->_i) {}
                }
            };

            typedef Iterator<typename base::mutable_iterator> mutable_iterator;

            WhereShim() = default;

            WhereShim(WhereShim &&) = default;

            WhereShim(const WhereShim &) = default;

            WhereShim(const T &c, const F &f)
                    : base(c),
                      _f(f) {
            }

            WhereShim &operator=(WhereShim &&) = default;

            WhereShim &operator=(const WhereShim &) = default;

            mutable_iterator end() {
                return mutable_iterator(this->_c.end(), this->_c.end(), *this);
            }

            mutable_iterator begin() {
                return mutable_iterator(this->_c.begin(), this->_c.end(), *this);
            }

            mutable_iterator end() const {
                return mutable_iterator(this->_c.end(), this->_c.end(), *this);
            }

            mutable_iterator begin() const {
                return mutable_iterator(this->_c.begin(), this->_c.end(), *this);
            }
        };

        template<typename F>
        WhereShim<F> Where(const F &f) {
            return WhereShim<F>((T &) *this, f);
        }

        template<typename V2, typename F>
        class SelectShim : public Shim<T, SelectShim<V2, F>, V2> {
            using base = Shim<T, SelectShim<V2, F>, V2>;
            F _f;
        public:

            template<typename I>
            struct Iterator : public IteratorBase<Iterator<I>, I, V2> {
                const SelectShim &_o;

                Iterator(const I &i, const I &e, const SelectShim &o)
                        : IteratorBase<Iterator<I>, I, V2>(i, e), _o(o) {
                }

                Iterator() = default;

                Iterator(Iterator &&) = default;

                Iterator(const Iterator &) = default;

                Iterator &operator=(Iterator &&) = default;

                Iterator &operator=(const Iterator &) = default;

                V2 operator*() {
                    return _o._f(*this->_i);
                }
            };

            typedef V2 value_type;
            typedef Iterator<typename base::mutable_iterator> mutable_iterator;

            SelectShim() = default;

            SelectShim(SelectShim &&) = default;

            SelectShim(const SelectShim &) = default;

            SelectShim(T &c, const F &f)
                    : base(c),
                      _f(f) {
            }

            SelectShim &operator=(SelectShim &&) = default;

            SelectShim &operator=(const SelectShim &) = default;

            mutable_iterator end() {
                return mutable_iterator(this->_c.end(), this->_c.end(), *this);
            }

            mutable_iterator begin() {
                return mutable_iterator(this->_c.begin(), this->_c.end(), *this);
            }

            mutable_iterator end() const {
                return mutable_iterator(this->_c.end(), this->_c.end(), *this);
            }

            mutable_iterator begin() const {
                return mutable_iterator(this->_c.begin(), this->_c.end(), *this);
            }
        };

        template<typename V2, typename F>
        SelectShim<V2, F> Select(F f) {
            return SelectShim<V2, F>(*(T *) this, f);
        }

        template<typename V2, typename F>
        class SelectManyShim : public Shim<T, SelectManyShim<V2, F>, V2> {
            using base = Shim<T, SelectManyShim<V2, F>, V2>;
            F _f;
        public:

            template<typename I>
            struct Iterator : public IteratorBase<Iterator<I>, I> {
                const SelectManyShim &_o;

                typedef decltype(_o._f(*(value_type_t *) nullptr)) T2;

                std::shared_ptr<Shim<T2, Iterator>> _s;

                typedef decltype(_s->begin()) I2;

                I2 _i2;
                I2 _e2;
                bool _f = true;

                Iterator(const I &i, const I &e, const SelectManyShim &o)
                        : IteratorBase<Iterator<I>, I>(i, e), _o(o) {
                }

                Iterator() = default;

                Iterator(Iterator &&) = default;

                Iterator(const Iterator &) = default;

                Iterator &operator=(Iterator &&) = default;

                Iterator &operator=(const Iterator &) = delete;

                void _ensh() const {
                    if (_f) {
                        const_cast<Iterator *>(this)->_chk();
                    }
                }

                bool operator==(const Iterator &i) const {
                    _ensh();
                    return this->_i == i._i;
                }

                Iterator operator++() {
                    _ensh();
                    ++_i2;
                    _chk();
                    return *this;
                }

                V2 operator*() {
                    _ensh();
                    return *_i2;
                }

                void _chk() {
                    for (; this->_i != this->_e && (_f || _i2 == _e2);) {
                        if (!_f) {
                            ++this->_i;
                            if (this->_i == this->_e) {
                                break;
                            }
                        }
                        _s = std::make_shared<Shim<T2, Iterator>>(_o._f(*this->_i));
                        _i2 = _s->begin();
                        _e2 = _s->end();
                        _f = false;
                    }
                }
            };

            typedef Iterator<typename base::mutable_iterator> mutable_iterator;

            SelectManyShim() = default;

            SelectManyShim(SelectManyShim &&) = default;

            SelectManyShim(const SelectManyShim &) = default;

            SelectManyShim(T c, const F &f)
                    : base(c),
                      _f(f) {
            }

            SelectManyShim &operator=(SelectManyShim &&) = default;

            SelectManyShim &operator=(const SelectManyShim &) = default;

            mutable_iterator end() {
                return mutable_iterator(this->_c.end(), this->_c.end(), *this);
            }

            mutable_iterator begin() {
                return mutable_iterator(this->_c.begin(), this->_c.end(), *this);
            }

            mutable_iterator end() const {
                return mutable_iterator(this->_c.end(), this->_c.end(), *this);
            }

            mutable_iterator begin() const {
                return mutable_iterator(this->_c.begin(), this->_c.end(), *this);
            }
        };

        template<typename V2, typename F>
        SelectManyShim<V2, F> SelectMany(F f) {
            return SelectManyShim<V2, F>(*(T *) this, f);
        }

        template<typename T2>
        class ConcatShim : public Shim<T, ConcatShim<T2>> {
            using base = Shim<T, ConcatShim<T2>>;
            T2 _c2;
        public:

            template<typename I1, typename I2>
            struct Iterator : public std::iterator<std::forward_iterator_tag, typename std::remove_reference<decltype(**(I1 *) nullptr)>::type> {
                I1 _i1;
                I1 _e1;
                I2 _i2;

                bool _f;

                Iterator(const I1 &i1, const I1 &e1, const I2 &i2, bool f = true)
                        : _i1(i1), _e1(e1), _i2(i2), _f(f) {
                    if (_i1 == _e1) {
                        _f = false;
                    }
                }

                Iterator() = default;

                Iterator(Iterator &&) = default;

                Iterator(const Iterator &) = default;

                Iterator &operator=(Iterator &&) = default;

                Iterator &operator=(const Iterator &) = default;

                bool operator==(const Iterator &i) const {
                    return _f == i._f && (_f ? _i1 == i._i1 : _i2 == i._i2);
                }

                bool operator!=(const Iterator &i) const {
                    return !(*this == i);
                }

                Iterator operator++() {
                    if (_f && ++_i1 != _e1) {
                        return *this;
                    }

                    if (!_f) {
                        ++_i2;
                    }

                    _f = false;
                    return *this;
                }

                decltype(*_i1) operator*() {
                    if (_f) {
                        return *_i1;
                    }
                    return *_i2;
                }
            };

            typedef Iterator<typename base::mutable_iterator, decltype(_c2.begin())> mutable_iterator;

            ConcatShim() = default;

            ConcatShim(ConcatShim &&) = default;

            ConcatShim(const ConcatShim &) = default;

            ConcatShim(T c1, T2 &&c2)
                    : base(c1),
                      _c2(std::forward<T2>(c2)) {
            }

            ConcatShim &operator=(ConcatShim &&) = default;

            ConcatShim &operator=(const ConcatShim &) = default;

            mutable_iterator begin() {
                return mutable_iterator(this->_c.begin(), this->_c.end(), _c2.begin());
            }

            mutable_iterator end() {
                return mutable_iterator(this->_c.end(), this->_c.end(), _c2.end(), false);
            }

            mutable_iterator begin() const {
                return mutable_iterator(this->_c.begin(), this->_c.end(), _c2.begin());
            }

            mutable_iterator end() const {
                return mutable_iterator(this->_c.end(), this->_c.end(), _c2.end(), false);
            }
        };

        template<typename T2>
        ConcatShim<T2> Concat(T2 &&e) {
            return ConcatShim<typename std::remove_const<T2>::type>(*(T *) this, std::forward<T2>(e));
        }

        class ExcludeShim : public Shim<T, ExcludeShim> {
            using base = Shim<T, ExcludeShim>;
            std::unordered_set<value_type_t> _set;
        public:

            template<typename I>
            struct Iterator : public IteratorBase<Iterator<I>, I> {
                const ExcludeShim &_o;

                Iterator(const I &i, const I &e, const ExcludeShim &o)
                        : IteratorBase<Iterator<I>, I>(i, e), _o(o) {
                    _chk();
                }

                Iterator() = default;

                Iterator(Iterator &&) = default;

                Iterator(const Iterator &) = default;

                Iterator &operator=(Iterator &&) = default;

                Iterator &operator=(const Iterator &) = default;

                Iterator operator++() {
                    ++this->_i;
                    _chk();
                    return *this;
                }

                void _chk() {
                    for (; this->_i != this->_e && _o._set.find(*this->_i) != _o._set.end(); ++this->_i) {}
                }
            };

            typedef Iterator<typename base::mutable_iterator> mutable_iterator;

            ExcludeShim() = default;

            ExcludeShim(ExcludeShim &&) = default;

            ExcludeShim(const ExcludeShim &) = default;

            template<typename T2>
            ExcludeShim(T c1, const T2 &c2)
                    : base(c1) {
                for (const auto &it: c2) {
                    _set.insert(it);
                }
            }

            ExcludeShim &operator=(ExcludeShim &&) = default;

            ExcludeShim &operator=(const ExcludeShim &) = default;

            mutable_iterator end() {
                return mutable_iterator(this->_c.end(), this->_c.end(), *this);
            }

            mutable_iterator begin() {
                return mutable_iterator(this->_c.begin(), this->_c.end(), *this);
            }

            mutable_iterator end() const {
                return mutable_iterator(this->_c.end(), this->_c.end(), *this);
            }

            mutable_iterator begin() const {
                return mutable_iterator(this->_c.begin(), this->_c.end(), *this);
            }
        };

        template<typename T2>
        ExcludeShim Exclude(const T2 &e) {
            return ExcludeShim(*(T *) this, e);
        }

        template<typename V2>
        class CastShim : public Shim<T, CastShim<V2>, V2> {
            using base = Shim<T, CastShim<V2>, V2>;
        public:

            template<typename I>
            struct Iterator : public IteratorBase<Iterator<I>, I, V2> {
                Iterator(const I &i, const I &e)
                        : IteratorBase<Iterator<I>, I, V2>(i, e) {
                }

                Iterator() = default;

                Iterator(Iterator &&) = default;

                Iterator(const Iterator &) = default;

                Iterator &operator=(Iterator &&) = default;

                Iterator &operator=(const Iterator &) = default;

                V2 operator*() {
                    return static_cast<V2>(*this->_i);
                }
            };

            typedef V2 value_type;
            typedef Iterator<typename base::mutable_iterator> mutable_iterator;

            CastShim() = default;

            CastShim(CastShim &&) = default;

            CastShim(const CastShim &) = default;

            CastShim(T c)
                    : base(c) {
            }

            CastShim &operator=(CastShim &&) = default;

            CastShim &operator=(const CastShim &) = default;

            mutable_iterator end() {
                return mutable_iterator(this->_c.end(), this->_c.end());
            }

            mutable_iterator begin() {
                return mutable_iterator(this->_c.begin(), this->_c.end());
            }

            mutable_iterator end() const {
                return mutable_iterator(this->_c.end(), this->_c.end());
            }

            mutable_iterator begin() const {
                return mutable_iterator(this->_c.begin(), this->_c.end());
            }
        };

        class IntersectShim : public Shim<T, IntersectShim> {
            using base = Shim<T, IntersectShim>;
            std::unordered_set<value_type_t> _set;
        public:

            template<typename I>
            struct Iterator : public IteratorBase<Iterator<I>, I> {
                const IntersectShim &_o;

                Iterator(const I &i, const I &e, const IntersectShim &o)
                        : IteratorBase<Iterator<I>, I>(i, e), _o(o) {
                    _chk();
                }

                Iterator() = default;

                Iterator(Iterator &&) = default;

                Iterator(const Iterator &) = default;

                Iterator &operator=(Iterator &&) = default;

                Iterator &operator=(const Iterator &) = default;

                Iterator operator++() {
                    ++this->_i;
                    _chk();
                    return *this;
                }

                void _chk() {
                    for (; this->_i != this->_e && _o._set.find(*this->_i) == _o._set.end(); ++this->_i) {}
                }
            };

            typedef Iterator<typename base::mutable_iterator> mutable_iterator;

            IntersectShim() = default;

            IntersectShim(IntersectShim &&) = default;

            IntersectShim(const IntersectShim &) = default;

            template<typename T2>
            IntersectShim(T c1, const T2 &c2)
                    : base(c1) {
                for (const auto &it: c2) {
                    _set.insert(it);
                }
            }

            IntersectShim &operator=(IntersectShim &&) = default;

            IntersectShim &operator=(const IntersectShim &) = default;

            mutable_iterator end() {
                return mutable_iterator(this->_c.end(), this->_c.end(), *this);
            }

            mutable_iterator begin() {
                return mutable_iterator(this->_c.begin(), this->_c.end(), *this);
            }

            mutable_iterator end() const {
                return mutable_iterator(this->_c.end(), this->_c.end(), *this);
            }

            mutable_iterator begin() const {
                return mutable_iterator(this->_c.begin(), this->_c.end(), *this);
            }
        };

        template<typename T2>
        IntersectShim Intersect(const T2 &e) {
            return IntersectShim(*(T *) this, e);
        }

        template<typename V2>
        CastShim<V2> Cast() {
            return CastShim<V2>(*(T *) this);
        }

        class ThrottleShim : public Shim<T, ThrottleShim> {
            using base = Shim<T, ThrottleShim>;
            size_t _v = 0;
        public:
            template<typename I>
            struct Iterator : public IteratorBase<Iterator<I>, I> {
                using base = IteratorBase<Iterator<I>, I>;

                const ThrottleShim &_o;

//                struct ThrottleItem {
//                    struct IteratorItem : public IteratorBase<IteratorItem, I> {
//                        using base = IteratorBase<IteratorItem, I>;
//
//                        size_t _v;
//
//                        IteratorItem(I b, I e, size_t v)
//                                : base(b, e), _v(v) {}
//
//                        bool operator==(const IteratorItem &i) const {
//                            if (i._i == base::_e && _v == 0) {
//                                return true;
//                            }
//                            return base::operator==(i);
//                        }
//
//                        IteratorItem operator++() {
//                            --_v;
//                            return base::operator++();
//                        }
//                    };
//
//                    I _b;
//                    I _e;
//                    size_t _v;
//
//                    typedef IteratorItem mutable_iterator;
//
//                    ThrottleItem(I b, I e, size_t v)
//                            : _b(b), _e(e), _v(v) {
//                    }
//
//                    ThrottleItem(ThrottleItem &&) = default;
//
//                    ThrottleItem(const ThrottleItem &) = default;
//
//                    ThrottleItem &operator=(ThrottleItem &&) = default;
//
//                    ThrottleItem &operator=(const ThrottleItem &) = default;
//
//                    mutable_iterator end() {
//                        return mutable_iterator(_e, _e, _v);
//                    }
//
//                    mutable_iterator begin() {
//                        return mutable_iterator(_b, _e, _v);
//                    }
//
//                    mutable_iterator end() const {
//                        return mutable_iterator(_e, _e, _v);
//                    }
//
//                    mutable_iterator begin() const {
//                        return mutable_iterator(_b, _e, _v);
//                    }
//                };

                Iterator(const I &i, const I &e, const ThrottleShim &o)
                        : base(i, e), _o(o) {
                }

                Iterator() = default;

                Iterator(Iterator &&) = default;

                Iterator(const Iterator &) = default;

                Iterator &operator=(Iterator &&) = default;

                Iterator &operator=(const Iterator &) = default;

                Iterator operator++() {
                    for (auto v = _o._v; base::_i != base::_e && v > 0; ++base::_i, --v) {}
                    return *this;
                }

                std::vector<value_type_t> operator*() {
                    std::vector<value_type_t> ret;
                    ret.reserve(_o._v);
                    auto i = base::_i;
                    for (auto v = _o._v; i != base::_e && v > 0; ++i, --v) {
                        ret.push_back(*i);
                    }
                    return ret;
                }
            };

            typedef Iterator<typename base::mutable_iterator> mutable_iterator;

            ThrottleShim() = default;

            ThrottleShim(ThrottleShim &&) = default;

            ThrottleShim(const ThrottleShim &) = default;

            ThrottleShim(const T &c, size_t v)
                    : base(c),
                      _v(v) {
            }

            ThrottleShim &operator=(ThrottleShim &&) = default;

            ThrottleShim &operator=(const ThrottleShim &) = default;

            mutable_iterator end() {
                return mutable_iterator(this->_c.end(), this->_c.end(), *this);
            }

            mutable_iterator begin() {
                return mutable_iterator(this->_c.begin(), this->_c.end(), *this);
            }

            mutable_iterator end() const {
                return mutable_iterator(this->_c.end(), this->_c.end(), *this);
            }

            mutable_iterator begin() const {
                return mutable_iterator(this->_c.begin(), this->_c.end(), *this);
            }
        };

        ThrottleShim Throttle(size_t value) {
            return ThrottleShim(*(T *) this, value);
        }

        template<typename VT = value_type_t>
        boost::optional<VT> FirstOrNone() {
            auto _this = (T *) this;
            auto ret = _this->begin();
            if (ret == _this->end()) {
                return boost::optional<VT>();
            }
            return *ret;
        }

        template<typename F>
        boost::optional<value_type_t> FirstOrNone(const F &f) {
            auto _this = (T *) this;
            return _this->template Where(f)
                    .template FirstOrNone<value_type_t>();
        }

        template<typename VT = value_type_t>
        boost::optional<VT> LastOrNone() {
            auto _this = (T *) this;
            auto ret = _this->begin();
            for (auto it = ret; it != _this->end(); ++it) {
                ret = it;
            }
            if (ret == _this->end()) {
                return boost::optional<VT>();
            }
            return *ret;
        }

        template<typename F>
        boost::optional<value_type_t> LastOrNone(const F &f) {
            auto _this = (T *) this;
            return _this->template Where(f)
                    .template LastOrNone<value_type_t>();
        }

        template<typename VT = value_type_t>
        VT First() {
            auto ret = FirstOrNone<VT>();
            if (!ret) {
                throw std::out_of_range("The element isn't found.");
            }
            return *ret;
        }

        template<typename VT = value_type_t, typename F>
        VT First(const F &f) {
            auto _this = (T *) this;
            return _this->template Where(f)
                    .template First<VT>();
        }

        template<typename VT = value_type_t>
        VT Last() {
            auto ret = LastOrNone<VT>();
            if (!ret) {
                throw std::out_of_range("The element isn't found.");
            }
            return *ret;
        }

        template<typename F>
        value_type_t Last(const F &f) {
            auto _this = (T *) this;
            return _this->template Where(f)
                    .template Last<value_type_t>();
        }

        size_t Count() {
            size_t ret = 0;
            auto _this = (T *) this;
            for (const auto &it: *_this) {
                (void) it;
                ++ret;
            }
            return ret;
        }

        value_type_t Sum() {
            value_type_t ret = {};
            auto _this = (T *) this;
            for (const auto &it: *_this) {
                ret = (value_type_t) (ret + it);
            }
            return ret;
        }

        bool Any() {
            auto _this = (T *) this;
            for (const auto &it: *_this) {
                (void) it;
                return true;
            }
            return false;
        }

        template<typename F>
        bool Any(F f) {
            auto _this = (T *) this;
            return _this->Where(f).Any();
        }

        template<typename F>
        bool All(F f) {
            auto _this = (T *) this;
            for (const auto &it: *_this) {
                if (!f(it)) {
                    return false;
                }
            }
            return true;
        }

        std::list<value_type_t> ToList() {
            std::list<value_type_t> ret;
            auto _this = (T *) this;
            for (const auto &it: *_this) {
                ret.push_back(it);
            }
            return ret;
        }

        std::vector<value_type_t> ToVector(size_t n = 0) {
            std::vector<value_type_t> ret;
            ret.reserve(n);
            auto _this = (T *) this;
            for (const auto &it: *_this) {
                ret.push_back(it);
            }
            return ret;
        }

        std::vector<value_type_t> ToOrderedVector(size_t n = 0) {
            auto _this = (T *) this;
            auto ret = _this->ToVector(n);
            std::sort(ret.begin(), ret.end());
            return ret;
        }

        template<typename F>
        std::vector<value_type_t> ToOrderedVector(F f, size_t n = 0) {
            auto _this = (T *) this;
            auto ret = _this->ToVector(n);
            std::sort(ret.begin(), ret.end(), f);
            return ret;
        }

        template<typename K, typename V2, typename KS, typename VS>
        std::unordered_map<K, V2> ToUnorderedMap(KS keySelector, VS valueSelector) {
            std::unordered_map<K, V2> ret;
            auto _this = (T *) this;
            for (const auto &it: *_this) {
                valueSelector(it, ret[keySelector(it)]);
            }
            return ret;
        }

        template<typename K, typename KS>
        std::unordered_map<K, value_type_t> ToUnorderedMap(KS keySelector) {
            return ToUnorderedMap<K, value_type_t>(keySelector, [](const value_type_t &v1, value_type_t &v2) { v2 = v1; });
        }

        std::unordered_set<value_type_t> ToUnorderedSet() {
            std::unordered_set<value_type_t> ret;
            auto _this = (T *) this;
            for (const auto &it: *_this) {
                ret.insert(it);
            }
            return ret;
        }

        template<typename K, typename F>
        std::unordered_set<K> ToUnorderedSet(const F &f) {
            auto _this = (T *) this;
            return _this->template Select<K>(f).ToUnorderedSet();
        }
    };

    template<typename P>
    struct _shared_getter {
        std::shared_ptr<P> _p;

        explicit _shared_getter(P &&p)
                : _p(std::make_shared<P>(std::forward<P>(p))) {
        }

        _shared_getter(_shared_getter &&) = default;

        _shared_getter(const _shared_getter &) = default;

        _shared_getter &operator=(_shared_getter &&) = default;

        _shared_getter &operator=(const _shared_getter &) = default;

        auto end() -> decltype(_p->end()) {
            return _p->end();
        }

        auto begin() -> decltype(_p->begin()) {
            return _p->begin();
        }

        auto end() const -> decltype(_p->end()) {
            return _p->end();
        }

        auto begin() const -> decltype(_p->begin()) {
            return _p->begin();
        }
    };

    template<typename P>
    struct _shared_getter<P &> {
        P &_p;

        explicit _shared_getter(P &p)
                : _p(p) {
        }

        _shared_getter(_shared_getter &&) = default;

        _shared_getter(const _shared_getter &) = default;

        _shared_getter &operator=(_shared_getter &&) = default;

        _shared_getter &operator=(const _shared_getter &) = default;

        auto end() -> decltype(_p.end()) {
            return _p.end();
        }

        auto begin() -> decltype(_p.begin()) {
            return _p.begin();
        }

        auto end() const -> decltype(_p.end()) {
            return _p.end();
        }

        auto begin() const -> decltype(_p.begin()) {
            return _p.begin();
        }
    };

    template<typename P>
    class StdShim : public Shim<_shared_getter<P>, StdShim<P>> {
        using base = Shim<_shared_getter<P>, StdShim<P>>;
    public:
        StdShim() = default;

        explicit StdShim(P &&p)
                : base(std::move(_shared_getter<P>(std::forward<P>(p)))) {
        }

        StdShim(StdShim &&) = default;

        StdShim(const StdShim &) = default;

        StdShim &operator=(StdShim &&) = default;

        StdShim &operator=(const StdShim &) = default;
    };

    template<typename I>
    struct ItShim {
        typedef I iterator;
        typedef I const_iterator;
        typedef typename I::value_type value_type;

        I _b;
        I _e;

        iterator end() {
            return _e;
        }

        iterator begin() {
            return _b;
        }

        const_iterator end() const {
            return _e;
        }

        const_iterator begin() const {
            return _b;
        }
    };

    template<typename P>
    StdShim<P> From(P &&p) {
        return StdShim<P>(std::forward<P>(p));
    }

    template<typename I>
    StdShim<ItShim<I>> From(const I &b, const I &e) {
        ItShim<I> s;
        s._b = b;
        s._e = e;
        return From(std::forward<ItShim<I>>(s));
    }

    template<typename T>
    StdShim<std::vector<T>> Move(std::initializer_list<T> &&t) {
        std::vector<T> vec(t.size());
        std::move(const_cast<T *>(t.begin()), const_cast<T *>(t.end()), vec.begin());
        return linq::From(std::move(vec));
    }

    class Enumerable {
    public:
        template<typename P>
        static auto From(P &&p) -> decltype(linq::From(std::forward<P>(p))) {
            return linq::From(std::forward<P>(p));
        }

        template<typename I>
        static auto From(const I &b, const I &e) -> decltype(linq::From(b, e)) {
            return linq::From(b, e);
        }
    };
}
