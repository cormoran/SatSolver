/**
 * file   : dpll-sat.cpp
 * date   : 2017-01-06
 */

#include<cstdio>
#include<cstdlib>
#include<iostream>
#include<vector>
#include<string>
#include<sstream>
#include<map>
#include<set>
#include<cassert>
#include<queue>
#include <tuple>

#include "../lib/clause.hpp"
#include "../lib/dimacs.hpp"

using namespace std;

using lib::tribool;
using lib::Variable;
using Literal = lib::Literal<>;
using Clause = lib::Clause<>;
using DIMACS = lib::DIMACS<>;


using VariableID = int;
using ClauseID = int;


/*    
// 連言標準形 : and で結ばれた節
struct ConjunctiveNormalForm {
    
};
*/

class Solver {
  public:
    DIMACS problem;
    
    tribool status() {
        tribool res(true);
        for(Clause &cl : problem.clauses) {
            res = res && cl.value();
        }
        return res;
    }

    void print_status() {
        cerr << "--" << endl;
        for(Clause &cl : problem.clauses) {
            cerr << cl << endl;
        }
    }
    
    tribool search(int i = 0) {
        cerr << i << endl;
        tribool res = status();
        if(res != tribool::undefined || i == problem.variables.size())
            return res;        
        problem.variables[i].value = tribool(false);
        print_status();
        res = search(i + 1);
        if(res)
            return res;
        problem.variables[i].value = tribool(true);
        print_status();
        res = search(i + 1);
        return res;
    }

    bool solve(ostream & os = cout) {
        tribool satisfiable = search();

        if(satisfiable) {
            os << "SAT" << endl;
            int max_index = -1;
            for(auto p : problem.variable_map) {
                max_index = max(max_index, p.first);
            }
            for(int i = 1; i <= max_index; i++) {
                if(problem.variable_map.count(i)) {
                    os << (static_cast<bool>(problem.variables[i].value) ? 1 : -1) * i;
                } else {
                    os << -1 * i;
                }
                os << " ";
            }
            os << "0" << endl;
        } else {
            os << "UNSAT" << endl;
        }
        return static_cast<bool>(satisfiable);
    }
};



int main(int argc, const char* argv[]) {
    Solver solver;
    
    solver.problem.load();
    solver.solve();

    return 0;
}
