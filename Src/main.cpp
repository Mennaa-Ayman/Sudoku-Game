#include "SudokuGame.hpp"
#include "UserInterface.hpp"
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char *argv[]){
    bool launchGui = true;

    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        if (arg == "--gui") {
            launchGui = true;
        } else if (arg == "--cli") {
            launchGui = false;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [--gui|--cli]" << std::endl;
            std::cout << "Default mode launches the graphical interface." << std::endl;
            return 0;
        }
    }

    if (launchGui) {
        try {
            return run_gui(argc, argv);
        } catch (const std::exception &ex) {
            std::cerr << "Failed to launch GUI: " << ex.what() << std::endl;
            std::cerr << "Falling back to console mode." << std::endl;
        }
    }

    SudokuGame game;
    game.startGame();
    return 0;
}