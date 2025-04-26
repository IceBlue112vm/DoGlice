#include <iostream>

#include "DoGlIce.hpp"

int main() {
    DoGlIce doglice;

    // init application
    if (!doglice.init()) {
        std::cout << "Initialization failed." << '\n';
        return -1;
    }

    // run application
    return doglice.run();
}