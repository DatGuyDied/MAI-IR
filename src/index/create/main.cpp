#include <iostream>
#include <vector>
#include <map>
#include <cctype>
#include <filesystem>
#include <fstream>
#include "biostream/biostream.h"

using Inverted_index = std::map<std::string, std::map<int32_t, std::vector<int32_t>>>;

struct entry {
    std::string id;
    std::string title;
    std::string description;
    std::string content;
};

std::map<std::string, std::vector<int32_t>> split(const std::string& s) {
    std::map<std::string, std::vector<int32_t> > ret;

    std::string token = "";
    int32_t pos = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        if (std::isspace(s[i])) {
            if (token == "") 
                continue;

            ret[token].push_back(pos);
            pos++;
            token = "";
            continue;
        }

        token += s[i];
    }

    return ret;
}

entry get_entry(const std::string& str) {
    entry e;

    std::vector<size_t> tab_pos;
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == '\t') {
            tab_pos.push_back(i);
        }
    }

    e.id = str.substr(0, tab_pos[0]);
    e.title = str.substr(tab_pos[0] + 1, tab_pos[1] - tab_pos[0] - 1);
    e.description = str.substr(tab_pos[1] + 1, str.size() - tab_pos[1] - 1);
    e.content = e.title + " " + e.description;

    return e;
}

void write_terms(const std::filesystem::path& output_path, Inverted_index index) {
    std::ofstream
        terms_pos_file(output_path / "terms_pos"),
        terms_file(output_path / "terms");

    biostream::write_int(terms_pos_file, (int32_t) 0);
    for (auto [term, mp] : index) {
        biostream::write_string(terms_file, term);
        biostream::write_int(terms_pos_file, (int32_t) terms_file.tellp());
    }
}

void write_docs(const std::filesystem::path& output_path, Inverted_index index) {
    std::ofstream
        docs_pos_file(output_path / "docs_pos"),
        docs_file(output_path / "docs");

    biostream::write_int(docs_pos_file, (int32_t) 0);
    for (auto [term, mp] : index) {
        biostream::write_map_keys(docs_file, mp);
        biostream::write_int(docs_pos_file, (int32_t) docs_file.tellp());
    }
}

void write_coords(const std::filesystem::path& output_path, Inverted_index index) {
    std::ofstream
        docs_coords_pos_file(output_path / "docs_coords_pos"),
        coords_pos_file(output_path / "coords_pos"),
        coords_file(output_path / "coords");

    biostream::write_int(docs_coords_pos_file, (int32_t) 0);
    biostream::write_int(coords_file, (int32_t) 0);
    for (auto [term, docs_map] : index) {
        for (auto [doc_id, coords] : docs_map) {
            biostream::write_vector(coords_file, coords);
            biostream::write_int(coords_pos_file, (int32_t) coords_file.tellp());
        }
        biostream::write_int(docs_coords_pos_file, (int32_t) coords_pos_file.tellp());
    }
}

void create_inverted_index(const std::filesystem::path& input_path, const std::filesystem::path& output_path) {
    std::filesystem::create_directory(output_path);
    std::ifstream input_file(input_path);
    std::ofstream 
        doc_count_file(output_path / "doc_count"),
        term_count_file(output_path / "term_count");

    Inverted_index inverted_index;

    std::string s;
    int32_t doc_count = 0;
    for(int32_t doc_id = 0; std::getline(input_file, s); doc_id++, doc_count++) {
        entry e = get_entry(s);
        auto splited_content = split(e.content);

        for (auto [term, coords] : splited_content) {
            inverted_index[term][doc_id] = coords;
        }
    }

    biostream::write_int(doc_count_file, doc_count);
    biostream::write_int(term_count_file, inverted_index.size());

    write_terms(output_path, inverted_index);
    write_docs(output_path, inverted_index);
    write_coords(output_path, inverted_index);
}

signed main() {
    std::filesystem::path input("data/db");
    std::filesystem::path output("src/index/create/index");

    std::cerr << "[INFO]\tcreating inverted index..." << std::endl; 
    create_inverted_index(input, output);
    std::cerr << "[INFO]\tdone!" << std::endl;
}