module;
#include <iostream>

module printing;

void print_hello() {
	std::cout << "Hello world\n";
}

void println(std::string_view msg) {
	std::cout << msg << '\n';
}