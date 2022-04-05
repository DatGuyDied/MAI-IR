#ifndef FILES_H
#define FILES_H

#include <vector>
#include <filesystem>
#include <fstream>
#include "biostream/biostream.h"

namespace files
{
class TermsCountFile {
    std::filesystem::path index_dir;

public:
    TermsCountFile(const std::filesystem::path& dir) : index_dir(dir){};

    int32_t get_terms_count() {
        std::ifstream terms_count_file(index_dir / "terms_count");
        return biostream::read_int(terms_count_file);
    }
};

class TermsPosFile {
    std::filesystem::path index_dir;

public:
    TermsPosFile(const std::filesystem::path& dir) : index_dir(dir){};

    int32_t get_term_pos(int32_t i) {
        std::ifstream terms_pos_file(index_dir / "terms_pos");
        terms_pos_file.seekg(sizeof(int32_t) * i);
        return biostream::read_int(terms_pos_file);
    }

    int32_t get_term_size(int32_t i) {
        std::ifstream terms_pos_file(index_dir / "terms_pos");
        terms_pos_file.seekg(sizeof(int32_t) * i);

        int32_t curr = biostream::read_int(terms_pos_file),
                next = biostream::read_int(terms_pos_file);
        
        return next - curr;
    }
};

class TermsFile {
    std::filesystem::path index_dir;

public:
    TermsFile(const std::filesystem::path& dir) : index_dir(dir){};

    std::string get_term(int32_t i) {
        std::ifstream terms_file(index_dir / "terms");
        TermsPosFile terms_pos(index_dir);

        terms_file.seekg(terms_pos.get_term_pos(i));
        return biostream::read_string(terms_file, terms_pos.get_term_size(i));
    }
};

class DocsCountFile {
    std::filesystem::path index_dir;

public: 
    DocsCountFile(const std::filesystem::path& dir) : index_dir(dir){};

    int32_t get_docs_count() {
        std::ifstream docs_count_file(index_dir / "docs_count");
        return biostream::read_int(docs_count_file);
    }
};

class DocsPosFile {
    std::filesystem::path index_dir;

public:
    DocsPosFile(const std::filesystem::path& dir) : index_dir(dir){};

    int32_t get_docs_pos(int32_t i) {
        std::ifstream docs_pos_file(index_dir / "docs_pos");
        docs_pos_file.seekg(sizeof(int32_t) * i);
        return biostream::read_int(docs_pos_file);
    }

    int32_t get_docs_size(int32_t i) {
        std::ifstream docs_pos_file(index_dir / "docs_pos");
        docs_pos_file.seekg(sizeof(int32_t) * i);

        int32_t curr = biostream::read_int(docs_pos_file),
                next = biostream::read_int(docs_pos_file);

        return next - curr;
    }
};

class DocsFile {
    std::filesystem::path index_dir;

public:
    DocsFile(const std::filesystem::path& dir) : index_dir(dir){};

    std::vector<int32_t> get_docs(int32_t i) {
        std::ifstream docs_file(index_dir / "docs");
        DocsPosFile docs_pos(index_dir);

        docs_file.seekg(docs_pos.get_docs_pos(i));
        return biostream::read_vector(docs_file, docs_pos.get_docs_size(i));
    }
};

class DocsCoordsPosFile {
    std::filesystem::path index_dir;

public:
    DocsCoordsPosFile(const std::filesystem::path& dir) : index_dir(dir){};

    int32_t get_docs_coords_pos(int32_t i) {
        std::ifstream docs_coords_pos_file(index_dir / "docs_coords_pos");
        docs_coords_pos_file.seekg(sizeof(int32_t) * i);
        return biostream::read_int(docs_coords_pos_file);
    }

    int32_t get_docs_coords_size(int32_t i) {
        std::ifstream docs_coords_pos_file(index_dir / "docs_coords_pos");
        docs_coords_pos_file.seekg(sizeof(int32_t) * i);

        int32_t curr = biostream::read_int(docs_coords_pos_file),
                next = biostream::read_int(docs_coords_pos_file);

        return next - curr;
    }
};

class CoordsPosFile {
    std::filesystem::path index_dir;

public:
    CoordsPosFile(const std::filesystem::path& dir) : index_dir(dir){};

    std::vector<int32_t> get_coords_pos(int32_t i) {
        std::ifstream coords_pos_file(index_dir / "coords_pos");
        DocsCoordsPosFile docs_coords_pos(index_dir);

        coords_pos_file.seekg(docs_coords_pos.get_docs_coords_pos(i));
        auto ret =  biostream::read_vector(coords_pos_file, docs_coords_pos.get_docs_coords_size(i));
        ret.pop_back();

        return ret;
    }

    std::vector<int32_t> get_coords_size(int32_t i) {
        std::ifstream coords_pos_file(index_dir / "coords_pos");
        DocsCoordsPosFile docs_coords_pos(index_dir);

        coords_pos_file.seekg(docs_coords_pos.get_docs_coords_pos(i));
        auto pos =  biostream::read_vector(coords_pos_file, docs_coords_pos.get_docs_coords_size(i));

        std::vector<int32_t> size;
        size.reserve(pos.size());
        for (int32_t i = 1; i < pos.size(); i++) {
            size.push_back(pos[i] - pos[i - 1]);
        }

        return size;
    }

    int32_t get_coords_pos(int32_t i, int32_t j) {
        std::vector<int32_t> coords_pos = get_coords_pos(i);
        return coords_pos[j];
    }

    int32_t get_coords_size(int32_t i, int32_t j) {
        std::vector<int32_t> coords_pos = get_coords_pos(i);
        return coords_pos[j + 1] - coords_pos[j];
    }
};

class CoordsFile {
    std::filesystem::path index_dir;

public:
    CoordsFile(const std::filesystem::path& dir) : index_dir(dir){};

    std::vector<std::vector<int32_t>> get_coords(int32_t i) {
        std::ifstream coords_file(index_dir / "coords");
        CoordsPosFile coords_pos(index_dir);

        std::vector<std::vector<int32_t>> coords;
        std::vector<int32_t>
            pos = coords_pos.get_coords_pos(i),
            size = coords_pos.get_coords_size(i);


        for (int32_t i = 0; i < pos.size(); i++) {
            coords_file.seekg(pos[i]);
            coords.push_back(biostream::read_vector(coords_file, size[i]));
        }

        return coords;
    }

    std::vector<int32_t> get_coords(int32_t i, int32_t j) {
        auto coords = get_coords(i);
        return coords[j];
    }
};

} // namespace files

#endif