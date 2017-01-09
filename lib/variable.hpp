#pragma once
#include <iostream>
#include "tribool.hpp"

namespace lib {

using VariableID = int;

struct Variable {
    tribool value;
    Variable() : value() {}

    friend std::ostream& operator<<(std::ostream &os, const Variable &v) {
        return os << v.value;
    }
};


}
