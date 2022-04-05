#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <string>
#include <filesystem>
#include <iostream>
#include "index/files/files.h"

namespace inverted_index {
class InvertedIndex {
    files::DocsCountFile docs_count;
    files::TermsCountFile terms_count;
    files::TermsFile terms;
    files::DocsFile docs;
    files::CoordsFile coords;

    int32_t get_term_id(const std::string& term) {
        int32_t l = 0, r = terms_count.get_terms_count();

        while(l < r) {
            int32_t m = (l + r) / 2;
            std::string str = terms.get_term(m);

            if (term <= str) {
                r = m;
            } else {
                l = m + 1;
            }
        }

        if (term != terms.get_term(l)) {
            return -1;
        }

        return l;
    }

public:
    InvertedIndex(const std::filesystem::path& dir) : 
        docs_count(dir), 
        terms_count(dir), 
        terms(dir), 
        docs(dir), 
        coords(dir) 
    {};

    std::vector<int32_t> doc_ids(const std::string& term) {
        int32_t term_id = get_term_id(term);
        return docs.get_docs(term_id);
    }

    std::vector<int32_t> full_doc_list() {
        int32_t n = docs_count.get_docs_count();
        std::vector<int32_t> ids(n, 0);

        for (int32_t i = 0; i < n; i++) {
            ids[i] = i;
        }

        return ids;
    }

    std::vector<int32_t> doc_coords(const std::string& term, int32_t doc_id) {
        int32_t term_id = get_term_id(term);
        std::vector<int32_t> ids = doc_ids(term);
        
        auto it = std::lower_bound(ids.begin(), ids.end(), doc_id);
        if (it == ids.end() || doc_id != *it) {
            return {};
        }

        int32_t serial_num = it - ids.begin(); 

        return coords.get_coords(term_id, serial_num);
    }
};


} //namespace inverted_index

#endif