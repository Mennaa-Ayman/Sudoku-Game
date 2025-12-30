#ifndef SUDOKU_EXCEPTION
#define SUDOKU_EXCEPTION

#include <iostream>
#include <stdexcept>

class SudokuException : public std::exception{
    private:
        std::string message; 
    public: 
    SudokuException(const std::string& m); 
    const char* what() const noexcept override;        
};

class WrongInput : public SudokuException{
    public: 
        WrongInput(const std::string& m);
        using SudokuException::what;
};

class WrongFileName : public SudokuException{
    public:
        WrongFileName(const std:: string& m);
        using SudokuException::what;
};

class SudokuUnsolvable: public SudokuException{
    public:
        SudokuUnsolvable(const std::string& m);
        using SudokuException::what;
};

class SudokuRulesException: public SudokuException{
    public:
        SudokuRulesException(const std::string& m);
        using SudokuException::what;
};
#endif