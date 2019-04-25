#pragma once

namespace griha {

template <unsigned V>
struct factorial {
    static constexpr unsigned value = V * factorial<V - 1>::value;
};

template <>
struct factorial<1> {
    static constexpr unsigned value = 1;
};

template <>
struct factorial<0> {
    static constexpr unsigned value = 1;
};

template <unsigned V>
constexpr unsigned factorial_v = factorial<V>::value;

constexpr unsigned fact(unsigned value) {
    if (value == 0 || value == 1)
        return 1;
    return value * fact(value - 1);
}

} // namespace griha