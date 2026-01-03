#ifndef USER_INTERFACE_HPP
#define USER_INTERFACE_HPP

#include <QMainWindow>
#include <QString>
#include <vector>

#include "SudokuBoard.hpp"
#include "SudokuGenerator.hpp"
#include "SudokuSolver.hpp"
#include "SudokuException.hpp"

class QAction;
class QLabel;
class QComboBox;
class QPushButton;
class QTableWidget;
class QToolBar;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void handleNewGame();
    void handleSolve();
    void handleLoad();
    void handleSave();
    void handleReset();
    void handleCellChanged(int row, int column);

private:
    void setupUi();
    void setupMenus();
    void populateBoard(const SudokuBoard &board);
    void clearBoard();
    void updateCellAppearance(int row, int column, bool isInitial);
    void setStatusMessage(const QString &message, bool isError = false);
    SudokuGenerator::Difficulity selectedDifficulty() const;
    bool isInitialCell(int row, int column) const;
    bool puzzleLoaded() const;
    void updateActionStates();

    QTableWidget *boardTable_ = nullptr;
    QComboBox *difficultyCombo_ = nullptr;
    QPushButton *newGameButton_ = nullptr;
    QPushButton *solveButton_ = nullptr;
    QPushButton *loadButton_ = nullptr;
    QPushButton *saveButton_ = nullptr;
    QPushButton *resetButton_ = nullptr;
    QLabel *statusLabel_ = nullptr;
    QToolBar *toolbar_ = nullptr;
    QAction *newGameAction_ = nullptr;
    QAction *solveAction_ = nullptr;
    QAction *loadAction_ = nullptr;
    QAction *saveAction_ = nullptr;
    QAction *resetAction_ = nullptr;
    QString defaultStatusStyle_;

    SudokuBoard currentBoard_;
    SudokuBoard initialBoard_;
    bool hasInitialBoard_ = false;
    bool populating_ = false;
};

int run_gui(int argc, char *argv[]);

#endif