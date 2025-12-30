#include "../Include/SudokuException.hpp"

const char* SudokuException:: what() const noexcept{
    return message.c_str();
}      

SudokuException::SudokuException(const std::string& m):message(m){}

WrongInput::WrongInput(const std::string& m):SudokuException(m){}

WrongFileName::WrongFileName(const std::string& m):SudokuException(m){}

SudokuUnsolvable::SudokuUnsolvable(const std::string& m):SudokuException(m){}

SudokuRulesException::SudokuRulesException(const std::string& m):SudokuException(m){}