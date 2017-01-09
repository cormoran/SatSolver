#pragma once

#include <iostream>
#include <vector>

#include "variable.hpp"

namespace  lib {

// リテラル : 変数 または 変数の否定
template<class V = Variable>
struct Literal {
    VariableID variable_id;
    bool is_not;    
    Literal(VariableID variable_id, bool is_not, std::vector<V> *variables)
            : variable_id(variable_id),
              is_not(is_not),
              variables(variables) {}
    bool operator == (const Literal &l) const {
        return variable_id == l.variable_id && is_not == l.is_not;
    }
    bool operator != (const Literal &l) const {
        return !(*this == l);
    }
    Literal operator ~() const {
        return Literal(variable_id, !is_not, variables);
    }
    tribool value() const {
        auto &val = (*variables)[variable_id].value;
        return is_not ? !val : val;
    }
    V& variable() const {
        return (*variables)[variable_id];
    }
  private:
    std::vector<V> *variables;
    friend std::ostream& operator<<(std::ostream &os, const Literal &lit) {
        return os << (lit.is_not ? "!" : " ") << lit.variable_id << "(" << lit.value() << ")";
    }
};

}
