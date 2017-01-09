#pragma once

#include <vector>
#include <iostream>

#include "literal.hpp"

namespace lib {

// 節 : or で結ばれたリテラル
template<class L = Literal<>>
struct Clause : std::vector<L> {
    bool learnt; // 学習から得た節か?
    tribool value() const {
        tribool res(false);
        for(auto &lit : *this) res = res || lit.value();
        return res;
    }

    friend std::ostream& operator<<(std::ostream &os, const Clause &cl) {
        os << cl.value() << ": ";
        for(const L &a : cl) os << a << (a == cl.back() ? "" : " v ");
        return os;
    }
};

}
