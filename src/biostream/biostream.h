#ifndef BIOSTREAM_H
#define BIOSTREAM_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace biostream {

void write_string(std::ostream& os, const std::string& s) {
    os.write(reinterpret_cast<const char*>(s.data()), s.size());
}

std::string read_string(std::istream& is, int32_t n) {
    std::string s(n, '$');
    is.read(reinterpret_cast<char*>(s.data()), n);

    return s;
}

void write_int(std::ostream& os, const int32_t& n) {
    os.write(reinterpret_cast<const char*>(&n), sizeof(n));
}

int32_t read_int(std::istream& is) {
    int32_t n;
    is.read(reinterpret_cast<char*>(&n), sizeof(n));

    return n;
}

void write_vector(std::ostream& os, const std::vector<int32_t>& a) {
    for (auto it : a) {
        write_int(os, it);
    }
}

std::vector<int32_t> read_vector(std::istream& is, int32_t n) {
    std::vector<int32_t> a;
    a.reserve(n);

    for (auto &it : a) {
        a.push_back(read_int(is));
    }

    return a;
}

void write_map_keys(std::ostream& os, const std::map<int32_t, std::vector<int32_t>>& mp) {
    for (auto [k, v] : mp) {
        write_int(os, k);
    }
}

} //namespace biostream


#endif