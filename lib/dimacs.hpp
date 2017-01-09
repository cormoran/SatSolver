#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>

#include "clause.hpp"

namespace lib {

template<class V = Variable,class C = Clause<>>
struct DIMACS {
    int n_of_variable, n_of_clause; // cnf format のはじめで宣言した数
    std::vector<V> variables;
    std::vector<C> clauses;
    std::map<int, int> variable_map;

    bool load(std::istream &is = std::cin) {
        variables.clear();
        clauses.clear();
        variable_map.clear();
        std::string line;
        bool started = false, ended = false;;
        while(!ended && std::getline(is, line)) {
            if(line.size() == 0 || line[0] == 'c') {
                continue;
            } else if(line[0] == 'p') {
                std::stringstream ss(line);
                std::string s;
                ss >> s;
                if(s != "p" || ss.eof()) return false;
                ss >> s;
                if(s != "cnf" || ss.eof()) return false;
                ss >> n_of_variable;
                if(ss.eof()) return false;
                ss >> n_of_clause;
                started = true;
            } else {
                if(!started) return false;
                std::stringstream ss(line);
                C clause;
                while(ss) {
                    int n; ss >> n;
                    if(n == 0) break;
                    bool is_not = n < 0;
                    n = std::abs(n);
                    if(!variable_map.count(n)) {
                        variable_map[n] = variables.size();
                        variables.emplace_back(); // undefined                        
                    }
                    clause.emplace_back(variable_map[n], is_not, &variables);
                }
                if(clause.size() == 0) break;
                clauses.push_back(std::move(clause));
            }
        }
        return true;
    }
};
} 
