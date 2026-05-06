#include "Game.hpp"
#include <iostream>
#include <stdexcept>

int main()
{
    try {
        Game game;
        game.run();
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << '\n';
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "[ERROR] Unknown exception.\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
