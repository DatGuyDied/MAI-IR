#ifndef INFIXNOTATION_H
#define INFIXNOTATION_H

#include <string>
#include <vector>
#include <stack>
#include <algorithm>
#include "index/inverted_index.h"
#include "booleansearch/operation.h"

using Docs_list = std::vector<int32_t>;

enum Op {
    OR,
    AND,
    NOT,
    BRACE,
};

namespace infix_notation {

class Parser {
 
    std::string query;
    inverted_index::InvertedIndex index;
    std::stack<Docs_list> docs_lists;
    std::stack<Op> ops;
    bool add_and;

public:
    bool is_delim(const char c) {
        return c == ' ' || c == '\t' || c == '\n';
    }

    bool is_operation(std::string s){
        return s == "&&" || s == "||" || s == "!";
    }

    int op_priority(Op op) {
        switch (op)
        {
        case OR:
            return 1;
            break;
        case AND:
            return 2;
            break;
        case NOT:
            return 3;
            break;
        default:
            return -1;
            break;
        }
    }

    bool op_on(size_t& i, Op& op) {
        if (query[i] == '!') {
            op = NOT;
            return true;
        }

        if (i >= query.length() - 1) {
            return false;
        }

        std::string str = query.substr(i, 2);
        if (str == "&&") {
            op = AND;
            i++;
            return true;
        }
        if (str == "||") {
            op = OR;
            i++;
            return true;
        }

        return false;
    }

    void calc_op(Op op) {
        if (docs_lists.empty()) {
            return;
        }
        Docs_list rhs = docs_lists.top();
        docs_lists.pop();

        if (op == OR) {
            if (docs_lists.empty()) {
                return;
            }

            Docs_list lhs = docs_lists.top();
            docs_lists.pop();

            docs_lists.push(boolean_or(lhs, rhs));
        } else if (op == AND) {
            if (docs_lists.empty()) {
                return;
            }

            Docs_list lhs = docs_lists.top();
            docs_lists.pop();

            docs_lists.push(boolean_and(lhs, rhs));
        } else if (op == NOT) {
            docs_lists.push(boolean_not(rhs, index.full_doc_list()));
        }
    }

    void open_brace() {
        if (add_and) {
            ops.push(AND);
        }

        ops.push(BRACE);
    }

    void close_brace() {
        while (!ops.empty() && ops.top() != BRACE) {
            calc_op(ops.top());
            ops.pop();
        }

        if (!ops.empty())
            ops.pop();
    }

    void operation(Op op) {
        while (!ops.empty() && op_priority(ops.top()) >= op_priority(op)) {
            calc_op(ops.top());
            ops.pop();
        }

        if (op == NOT && add_and)
            ops.push(AND);

        ops.push(op);
    }

    std::string read_term(size_t& i) {
        std::string term = "";
        for (;i < query.length() && isalnum(query[i]); i++) {
            term += query[i];
        }

        i--;
        return term;
    }

    Docs_list docs_by_quote(std::vector<std::string> terms, int32_t k) {
        Docs_list docs = index.full_doc_list();
        for (auto term : terms) {
            docs = boolean_and(docs, index.doc_ids(term));
        }

        Docs_list ret;
        for (auto doc : docs) {
            std::vector<std::vector<int32_t>> coords;
            coords.reserve(terms.size());

            for (auto term : terms) {
                coords.push_back(index.doc_coords(term, doc));
            }

            bool ok = false;

            for (size_t i = 0; i < coords[0].size(); i++) {
                std::vector<int32_t> pos(coords.size(), INT32_MAX);
                pos[0] = coords[0][i];

                for (size_t j = 1; j < pos.size(); j++) {
                    auto it = std::upper_bound(coords[j].begin(), coords[j].end(), pos[j-1]);

                    if (it == coords[j].end()) {
                        break;
                    }

                    pos[j] = *it;

                    if (pos[j] - pos[0] > k) {
                        break;
                    }
                }

                if (pos.back() - pos.front() <= k) {
                    ok = true;
                    break;
                }
            }

            if (ok) {
                ret.push_back(doc);
            }
        }

        return ret;
    }

    void quote(size_t& i) {
        std::vector<std::string> terms;

        for (;i < query.length() && query[i] != '"'; i++) {
            if (is_delim(query[i])) continue;
            std::string term = read_term(i);

            if (term != "") {
                terms.push_back(term);
            }
        }

        i++;
        size_t k = 0;

        for (;i < query.length() && isspace(query[i]); i++){}

        if (i < query.length() && query[i] == '/') {
            i++;
            for (;i < query.length() && isspace(query[i]); i++){}

            for (; i < query.length() && isdigit(query[i]); i++) {
                k *= 10;
                k += query[i] - '0';
            }
        }

        i--;
        k = std::max(k, terms.size() - 1);

        docs_lists.push(docs_by_quote(terms, k));
    }

    Parser(const std::string& query, const std::filesystem::path& index_path) : 
        query(query), 
        index(index_path), 
        add_and(false) 
    {};

    Docs_list calc() {
        for (size_t i = 0; i < query.length(); i++) {
            char c = query[i];
            
            if (is_delim(c)) {
                continue;
            }

            if (c == '(') {
                open_brace();
                add_and = false;
                continue;
            }

            if (c == ')') {
                close_brace();
                add_and = true;
                continue;
            }

            Op op;
            if (op_on(i, op)) {
                operation(op);
                add_and = false;
                continue;
            }

            if (add_and) {
                ops.push(AND);
            }

            if (c == '"') {
                quote(++i);
                add_and = true;
                continue;
            }

            std::string term = read_term(i);

            if (term == "") {
                continue;
            }

            docs_lists.push(index.doc_ids(term));
            add_and = true;
        }

        while (!ops.empty()) {
            calc_op(ops.top());
            ops.pop();
        }

        if (!docs_lists.empty()) {
            return docs_lists.top();
        }

        return {};
    }
};
} //namespace infix_notation

#endif