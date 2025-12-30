/*
o Presents console menu, reads/writes user input. 
o Coordinates SudokuBoard and SudokuSolver. 
o Handles error-checking for moves and file operations. 
*/

#ifndef SUDOOKU_GAME
#define SUDOOKU_GAME

#include "SudokuBoard.hpp"
#include "SudokuSolver.hpp"
#include "SudokuGenerator.hpp"
#include "SudokuException.hpp"
#include <iostream>
#include <string>

class SudokuGame: public SudokuBoard{
    private:
        SudokuSolver solver;
        int getChoice() const;
        
    public:
        SudokuGame();
        SudokuGame(const std::string& fname);
        SudokuGame(const std::vector<std::vector<int>>& other);
        void startGame();
        void welcomeSudoku();
};

#endif