/*
Backtracking method to find a solution or detect unsolvability. 
*/

#ifndef SUDOOKU_SOLVER
#define SUDOOKU_SOLVER

#include "SudokuBoard.hpp"
#include <vector>
#include <iostream>
#include <algorithm>

class SudokuSolver{
    private:
        SudokuBoard board; 

    public:
        SudokuSolver(const SudokuBoard& b): board(b) {}
        
        // Attempts to solve the Sudoku puzzle; returns true if solved, false if unsolvable
        bool solve();
        const SudokuBoard& getBoard() const;
        void setBoard(const std::vector<std::vector<int>>& boardObject);
        void printBoard() const { board.printBoard(); }
};

#endif