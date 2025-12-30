/*
Stores and manipulates the 9×9 grid. 
o Validates row/column/box constraints. 
o Prints the board. 
o (Optional) Loads and saves puzzle data to a file.
*/

#ifndef SUDOKU_BOARD
#define SUDOKU_BOARD

#include "SudokuException.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

class SudokuBoard{
    private:
        friend class SudokuSolver;
        friend class SudokuGame;
        std::vector<std::vector<int>> board; // 2D vector representing the Sudoku grid
        bool puzzleLoaded =false;
        std::vector<int> getBox(int box_row_num , int box_col_num) const; // Retrieves all values in a 3x3 box
        bool validBox(int row_num, int col_num, int num);
        bool validRow(int row_num, int num);
        bool validColumn (int col_num, int num);

    public:
        SudokuBoard();
        SudokuBoard(const std::string& fname);
        SudokuBoard(const std::vector<std::vector<int>>& other): board(other) {}

        const std::vector<std::vector<int>>& getBoard() const;
        void setBoard(const std::vector<std::vector<int>>& other);

        bool setValue(int row_num, int col_num, int num, bool backtrace=0);   
        int getValue(int row_num, int col_num) const { return board[row_num-1][col_num-1]; }

        void printBoard() const;
        int emptyCount() const; //number of empty places

        void save(const std::string& fname) const;
};

#endif