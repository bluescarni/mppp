#include <iostream>
#include <mp++/mp++.hpp>

using int_t = mppp::integer<1>;

int main()
{
    int_t n{42};
    std::cout << n << '\n';
}
