/*
Creates a fully solved board, then removes cells while ensuring uniqueness. 
*/

#ifndef SUDOKU_GENERATOR
#define SUDOKU_GENERATOR

#include "SudokuBoard.hpp"
#include <cstdlib>
#include <ctime>
#include "SudokuSolver.hpp"

class SudokuGenerator: public SudokuBoard {
    void shuffle_vector(std::vector<int>& vec) const;

    bool generate_complete();

    void remove_elements(int empty);

    public:
    enum Difficulity
    {
        EASY = 40,
        MEDIUM = 48,
        HARD = 52
    };
    

    void generate(Difficulity d);

    using SudokuBoard::getBoard;
};

#endif