#pragma once
#include <iostream>
namespace lib { 

/* tribool
 * 初期値は undefined
 */
struct tribool {
    enum tribool_t { false_value, true_value, undefined_value } value;  
    constexpr tribool() : value(undefined_value) {}
    constexpr tribool(bool initial) : value(initial ? true_value : false_value) {}
    constexpr explicit operator bool() const { return value == true_value; }    
    constexpr tribool operator ! () const { return value == false_value ? tribool(true) : value == true_value ? tribool(false) : tribool(); }
    constexpr tribool operator || (const tribool &l) const {
        return (value == true_value || l.value == true_value) ? tribool(true) :
                (value == undefined_value || l.value == undefined_value) ? tribool() : tribool(false);
    }
    constexpr tribool operator && (const tribool &l) const {
        return (value == false_value || l.value == false_value) ? tribool(false) :
                (value == undefined_value || l.value == undefined_value) ? tribool() : tribool(true);
    }
    constexpr bool operator == (const tribool &l) const { return value == l.value; }
    constexpr bool operator != (const tribool &l) const { return !(*this == l); }    
    static tribool undefined;
    friend std::ostream& operator<<(std::ostream &os, const tribool &v) {
        return os << (v.value == tribool::true_value ? "o" : v.value == tribool::false_value ? "x" : "?");
    }
};

tribool tribool::undefined;

}
