//
// Copyright (C) 2019 Kapitonov Maxim
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

#include "linqcpp.h"

namespace linq {
    template<typename V>
    class ITearOffIterator {
    public:
        virtual ~ITearOffIterator() = default;

        virtual std::shared_ptr<ITearOffIterator> PrefInc() = 0;

        virtual std::shared_ptr<ITearOffIterator> PostInc() = 0;

        virtual V operator*() = 0;

        virtual bool operator!=(const ITearOffIterator<V> &) const = 0;
    };

    template<typename V>
    class SharedTearOffIterator {
        std::shared_ptr<ITearOffIterator<V>> _i;
    public:
        SharedTearOffIterator(std::shared_ptr<ITearOffIterator<V>> i)
                : _i(std::move(i)) {
        }

        bool operator!=(const SharedTearOffIterator &i) const {
            return *_i != *i._i;
        }

        SharedTearOffIterator operator++() {
            return SharedTearOffIterator(_i->PrefInc());
        }

        SharedTearOffIterator operator++(int) {
            return SharedTearOffIterator(_i->PostInc());
        }

        V operator*() {
            return *(*_i);
        }
    };

    template<typename V>
    class ITearOffContainer {
    public:
        virtual ~ITearOffContainer() = default;

        virtual size_t get_capacity() const {
            return 0;
        }

        virtual SharedTearOffIterator<V> end() = 0;

        virtual SharedTearOffIterator<V> begin() = 0;
    };

    namespace details {
        template<typename T>
        size_t get_capacity(const ITearOffContainer<T> &c) {
            return c.get_capacity();
        }

        template<typename T>
        struct Container<std::shared_ptr<T>> {
            template<typename T2>
            static constexpr auto Get(T2 &t) -> decltype(*t) {
                return *t;
            }
        };
    }

    template<typename P, typename T, typename V>
    std::shared_ptr<ITearOffContainer<typename Shim<P, T, V>::value_type>> Shim<P, T, V>::AsTearOffContainer() {

        if constexpr(Shim<P, T, V>::shared) {
            class TearOffContainer : public ITearOffContainer<value_type> {
                T _t;

                class TearOffIterator : public ITearOffIterator<value_type> {
                    mutable_iterator _i;
                public:
                    TearOffIterator(mutable_iterator i)
                            : _i(i) {
                    }

                    std::shared_ptr<ITearOffIterator<value_type>> PrefInc() override {
                        return std::make_shared<TearOffIterator>(++_i);
                    }

                    std::shared_ptr<ITearOffIterator<value_type>> PostInc() override {
                        return std::make_shared<TearOffIterator>(_i++);
                    }

                    value_type operator*() override {
                        return *_i;
                    }

                    bool operator!=(const ITearOffIterator<value_type> &i) const override {
                        return _i != dynamic_cast<const TearOffIterator *>(&i)->_i;
                    }
                };

            public:

                TearOffContainer(const T &t)
                        : _t(t) {
                }

                SharedTearOffIterator<value_type> end() override {
                    return SharedTearOffIterator<value_type>(std::make_shared<TearOffIterator>(_t.end()));
                }

                SharedTearOffIterator<value_type> begin() override {
                    return SharedTearOffIterator<value_type>(std::make_shared<TearOffIterator>(_t.begin()));
                }
            };
            auto _this = (T *) this;
            return std::make_shared<TearOffContainer>(*_this);
        } else {
            return "You cannot use a container by reference as a tear-off container. It's not safe.";
        }
    }
}
