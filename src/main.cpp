#include "types.hpp"
#include "scan.hpp"

#include <iostream>

template <typename... Ts>
void print_tuple(const std::tuple<Ts...> &t) {
    std::cout << "(";

    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        ((std::cout << (Is == 0 ? "" : ", ") << std::get<Is>(t)), ...);
    }(std::index_sequence_for<Ts...>{});

    std::cout << ")\n";
}

int main() { 
    auto r = stdx::scan<std::uint16_t, double, std::string>("3 3.14 hello", "{%u} {%f} {%s}");
    if(r){
        std::cout << "Parsed\n";
        print_tuple(r->values);

    } else {
        std::cout << "Error: " << r.error().message << "\n";
    }

    return 0;
}