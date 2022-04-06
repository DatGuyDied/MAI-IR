#include <iostream>
#include <vector>
#include "index/files/files.h"
#include "booleansearch/infixnotation.h"

int main() {
    std::string str;
    std::getline(std::cin, str);

    infix_notation::Parser p(str, "src/index/create/index");
    files::DBFile db_file("data");


    auto v = p.calc();

    for (auto it : v) {
        std::cout << it << ' ' << std::endl;
        std::cout << db_file.get_line(it);
    }
}
