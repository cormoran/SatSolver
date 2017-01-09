/**
 * file   : cdcl-sat.cpp
 * date   : 2017-01-09
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
#include<thread>
using namespace std;

#define DEBUG 0

#include "../lib/dimacs.hpp"

using VariableID = int;
using ClauseID = int;


using lib::tribool;

// 変数
struct Variable : lib::Variable {
    ClauseID reason_clause_id; // 値が決定される理由となったClause
    int decided_level;    
    Variable() : reason_clause_id(-1), decided_level(-1), lib::Variable() { }
};


using Literal = lib::Literal<Variable>;

const Literal contradict_literal(-1, false, nullptr);

// 節 : or で結ばれたリテラル
struct Clause : lib::Clause<Literal> {
    bool learnt; // 学習から得た節か?
};
using DIMACS = lib::DIMACS<Variable, Clause>;

class Solver {
  public:
    bool obvious_unsat; // cnf読み込み時にunsatとなった場合
    DIMACS problem;
    vector<Clause> clauses;
    vector<vector<ClauseID>> watches; // unit propagation用の変数割り当て監視
    vector<Literal> assign_trail; // リテラルの値割り当て履歴
    queue<Literal> prop_que; // propagate処理すべきやつら
    // 仮
    int decision_level;
    // --

    bool init() {
        bool ok = true;
        watches.resize(problem.variables.size() * 2);
        for(Clause cl : problem.clauses) { // copy
            ok &= add_clause(move(cl));
        }
        return ok;
    }

    void show_status() {
        if(DEBUG) {
            cerr << "--" << endl;
            for(auto &cl : clauses) cerr << cl << endl;
        }
    }

    bool add_clause(Clause &&clause) {
        if(obvious_unsat) return false;
        if(clause.size() == 0) {
            return false;
        } else if(clause.size() == 1) {
            return assign_true(clause[0]);
        } else {
            clauses.push_back(clause);
            watches[watch_index(~clause[0])].push_back(clauses.size() - 1);
            watches[watch_index(~clause[1])].push_back(clauses.size() - 1);
            return true;
        }
    }

    bool assign_true(const Literal &lit, ClauseID reason_clause_id = -1) {
        if(obvious_unsat) return false;
        cerr << "try assign true " << lit << endl;
        if(lit.value() == tribool::undefined) {
            Variable &var = lit.variable();
            var.reason_clause_id = reason_clause_id;
            var.decided_level = decision_level;
            var.value = lit.is_not ? false : true;
            prop_que.push(lit);
            assign_trail.emplace_back(lit);
            cerr << "assign variable " << lit.variable_id << " -> " << (lit.is_not ? "false" : "true") << endl;
            return true;
        } else {
            // check conflict
            if(lit.value() != tribool(true)) {
                cerr << "conflict try assign variable " << lit.variable_id << " -> " << (lit.is_not ? "false" : "true") << endl;
            }
            return lit.value() == tribool(true);
        }
    }

    
    int watch_index(const Literal &lit) {
        return lit.variable_id * 2 + lit.is_not;
    }

    // -> conflict clause if conflice else nullptr
        // -> conflict clause if conflice else nullptr
    ClauseID propagate() {
        if(obvious_unsat) return false;
        while(not prop_que.empty()) {
            Literal lit = prop_que.front(); prop_que.pop(); // true にされたリテラル
            cerr << "propagate variable " << lit.variable_id << endl;
            cerr << watches.size() << " " << watch_index(lit) << endl;
            vector<ClauseID> &watch = watches[watch_index(lit)];
            // とりあえず読みやすい実装にする
            vector<ClauseID> new_watch;
            for(ClauseID clause_id : watch) {
                Clause &clause = clauses[clause_id];
                assert(clause.size() >= 2);
                Literal false_lit = ~lit;
                if(clause[0] == false_lit) iter_swap(clause.begin(), clause.begin() + 1);
                assert(clause[1] == false_lit);

                if(clause[0].value()) { // false になったのと異なるやつがtrueなのでそのままでいい
                    new_watch.push_back(clause_id);
                    cerr << "ok propagate" << endl;
                    continue;
                }
                // watchが2つともtrueでないので, true or unidefinedなliteralを探してwatch
                for(int i = 2; i < clause.size(); i++) {
                    if(clause[i].value() != tribool::false_value) {
                        iter_swap(clause.begin() + 1, clause.begin() + i);
                        watches[watch_index(~clause[1])].push_back(clause_id);
                        cerr << "change watch target" << endl;
                        goto NEXT_PROPAGATE;
                    }
                }
                // true or undefined な literalが1つ(clause[0])しかない! 単位伝搬
                if(!assign_true(clause[0])) {
                    // conflict
                    prop_que = decltype(prop_que)();
                    return clause_id;
                }
                new_watch.push_back(clause_id);
                // for(auto &cl : clauses) cerr << cl << endl;
          NEXT_PROPAGATE:;                
            }
            swap(watch, new_watch);
        }
        // cerr << "propagate result" << endl;
        // for(auto &cl : clauses) cerr << cl << endl;
        return -1;
    }
    
    // -> (rollback level, learnt clause)
    tuple<int, Clause> analize(ClauseID conflict_clause) {
        set<int> visited; // TODO: 外部にseen置いて使う

        // Literal lit = Literal::contradict_literal;
        bool first = true;
        int path = 0;
        Clause learnt_clause;
        int roll_back_level = decision_level;
        int trail_pos = assign_trail.size() - 1;
        do {
            assert(conflict_clause >= 0);
            for(Literal &lit : clauses[conflict_clause]) {
                if(first && lit == clauses[conflict_clause].front()) {
                    first = false;
                    continue;
                }

                if(not visited.count(lit.variable_id) && lit.variable().decided_level > 0) {
                    visited.insert(lit.variable_id);
                    if(lit.variable().decided_level >= decision_level) {
                        path++;
                    } else {
                        learnt_clause.push_back(~lit);
                        roll_back_level = max(roll_back_level, lit.variable().decided_level);
                    }
                }
            }
            while(not visited.count(assign_trail[trail_pos--].variable_id));
            trail_pos++;
            conflict_clause = assign_trail[trail_pos].variable().reason_clause_id;
            visited.erase(assign_trail[trail_pos].variable_id);
            path--;
        } while(path > 0);
        learnt_clause.push_back(~assign_trail[trail_pos]);
        return make_tuple(roll_back_level, learnt_clause);
    }
    
    int rollback(int level) {
        if(obvious_unsat) return -1;
        int ret = -1;
        while(decision_level > level) {
            // cerr << "rollback" << endl;
            if(decision_level == 0) return -1;
            int i = assign_trail.size() - 1;
            while(i >= 1) {
                if(assign_trail[i - 1].variable().decided_level < decision_level) break;
                
                Variable &v = assign_trail[i].variable();
                assert(v.decided_level >= decision_level);
                v.value = tribool::undefined;
                assign_trail.pop_back();
                i--;
            }
            
            Variable &v = assign_trail[i].variable();
            v.value = tribool::undefined;       
            ret = assign_trail[i].variable_id;
            assign_trail.pop_back();
            decision_level--;
        }
        assert(ret >= 0);
        return ret;
    }
    
    tribool search() {
        if(obvious_unsat) return false;
        vector<int> visit_cnt(problem.variables.size());
        
        for(int i = 0; i < problem.variables.size(); i++) {
            if(visit_cnt[i] > 1) return false;
            if(problem.variables[i].value == tribool::undefined) {
                decision_level++;
                assign_true(Literal(i, visit_cnt[i], &problem.variables));
            } else {
                continue;
            }
            show_status();
            visit_cnt[i]++;
            int conflict = propagate();
            // cerr << "propagate result : " << conflict << endl;            
            if(conflict != -1) {
                Clause learnt;
                int rollback_level;
                tie(rollback_level, learnt) = analize(conflict);                
                i = rollback(rollback_level - 1) - 1;
                cerr << learnt << endl;
                assert(add_clause(move(learnt)));
                assign_true(learnt.back());
                if(i < -1) return false;
            }
        }
        return true;
    }

        bool solve(ostream & os = cout) {
        if(obvious_unsat) return false;
        tribool satisfiable;
        init();
        satisfiable = search();

        if(satisfiable) {
            os << "SAT" << endl;
            int max_index = -1;
            for(auto p : problem.variable_map) {
                max_index = max(max_index, p.first);
            }
            for(int i = 1; i <= max_index; i++) {
                if(problem.variable_map.count(i)) {
                    os << (static_cast<bool>(problem.variables[problem.variable_map[i]].value) ? 1 : -1) * i;
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
