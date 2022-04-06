#ifndef OPERATION_H
#define OPERATION_H

#include <vector>

using Docs_list = std::vector<int32_t>;

Docs_list boolean_and(const Docs_list& a, const Docs_list b) {
    auto l = a.begin(), r = b.begin();
    Docs_list ret;
    ret.reserve(std::min(a.size(), b.size()));

    while (l != a.end() && r != b.end()) {
        int32_t lv = *l, rv = *r;

        if (lv == rv) {
            ret.push_back(lv);
            l++, r++;
        } else if (lv < rv) {
            l++;
        } else {
            r++;
        }
    }

    return ret;
}

Docs_list boolean_or(const Docs_list& a, const Docs_list& b) {
    auto l = a.begin(), r = b.begin();
    Docs_list ret;
    ret.reserve(a.size() + b.size());

    while (l != a.end() && r != b.end()) {
        int32_t lv = *l, rv = *r;

        if (lv == rv) {
            ret.push_back(lv);
            l++, r++;
        } else if (lv < rv) {
            ret.push_back(lv);
            l++;
        } else {
            ret.push_back(rv);
            r++;
        }
    }

    while (l != a.end()) ret.push_back(*l++);
    while (r != b.end()) ret.push_back(*r++);

    return ret;
}

Docs_list boolean_not(const Docs_list& a, const Docs_list& all) {
    auto l = a.begin(), r = all.begin();
    Docs_list ret;
    ret.reserve(all.size() - a.size());

    while (l != a.end() && r != all.end()) {
        int32_t lv = *l, rv = *r;

        if (lv == rv) {
            l++, r++;
        } else if (lv < rv) {
            l++;
        } else {
            ret.push_back(rv);
            r++;
        }
    }

    while (r != all.end()) ret.push_back(*r++);

    return ret;
}

#endif