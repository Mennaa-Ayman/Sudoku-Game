#include "UserInterface.hpp"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QObject>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QStyledItemDelegate>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QAbstractItemView>
#include <QKeySequence>
#include <QColor>
#include <QFont>
#include <QFontDatabase>
#include <QPainter>
#include <QSize>

#include <exception>

namespace {
// UI constants
constexpr int kBoardSize = 9;
constexpr int kStatusTimeoutMs = 5000;

// Paints bold borders around each 3x3 block so the playing field is easier to read.
class SudokuCellDelegate final : public QStyledItemDelegate {
public:
	explicit SudokuCellDelegate(QObject *parent = nullptr)
		: QStyledItemDelegate(parent) {}

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
		QStyledItemDelegate::paint(painter, option, index);

		painter->save();
		const QRect rect = option.rect.adjusted(0, 0, -1, -1);
		QPen pen(QColor(120, 120, 120));
		pen.setWidth(3);
		painter->setPen(pen);

		const int row = index.row();
		const int column = index.column();

		if (row % 3 == 0) {
			painter->drawLine(rect.topLeft(), rect.topRight());
		}
		if (row == kBoardSize - 1) {
			painter->drawLine(rect.bottomLeft(), rect.bottomRight());
		}
		if (column % 3 == 0) {
			painter->drawLine(rect.topLeft(), rect.bottomLeft());
		}
		if (column == kBoardSize - 1) {
			painter->drawLine(rect.topRight(), rect.bottomRight());
		}
		painter->restore();
	}
};

// Palette helpers for locked vs editable cells.
QColor initialCellColor() {
	return QColor(242, 242, 242);
}

QColor editableCellColor() {
	return QColor(255, 255, 255);
}

// Pull puzzle data from disk while validating shape and values.
bool loadBoardFromFile(const QString &fileName, SudokuBoard &outBoard, QString &errorMessage) {
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		errorMessage = file.errorString();
		return false;
	}

	QTextStream in(&file);
	std::vector<std::vector<int>> grid(kBoardSize, std::vector<int>(kBoardSize, 0));

	for (int row = 0; row < kBoardSize; ++row) {
		for (int col = 0; col < kBoardSize; ++col) {
			if (in.atEnd()) {
				errorMessage = QObject::tr("File ended early while reading puzzle data.");
				return false;
			}

			int value = 0;
			in >> value;

			if (in.status() != QTextStream::Ok) {
				errorMessage = QObject::tr("Unable to parse value at row %1 column %2.").arg(row + 1).arg(col + 1);
				return false;
			}

			if (value < 0 || value > 9) {
				errorMessage = QObject::tr("Values must be between 0 and 9. Offending value at row %1 column %2.").arg(row + 1).arg(col + 1);
				return false;
			}

			grid[row][col] = value;
		}
	}

	SudokuBoard board(grid);
	outBoard.setBoard(board.getBoard());
	return true;
}
}

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent) {
	// Build widgets, connect actions, and populate with an empty puzzle.
	setupUi();
	setupMenus();
	clearBoard();
	setStatusMessage(tr("Select a difficulty and start a new puzzle."));
}

void MainWindow::setupUi() {
	setWindowTitle(tr("Sudoku"));
	resize(680, 780);

	// Root container and gradient background establish the overall look.
	auto *central = new QWidget(this);
	auto *mainLayout = new QVBoxLayout(central);
	mainLayout->setContentsMargins(12, 12, 12, 12);
	mainLayout->setSpacing(12);
	central->setStyleSheet("background: #ffffff; color: #1a1a1a;");

	// Choose typography for the title, controls, and accent buttons.
	QFont titleFont(QStringLiteral("Poppins"), 30, QFont::Bold);
	if (!QFontDatabase::hasFamily(titleFont.family())) {
		titleFont.setFamily(QStringLiteral("Segoe UI Semibold"));
	}
	QFont controlFont(QStringLiteral("Segoe UI"), 11);
	QFont buttonFont(QStringLiteral("Segoe UI"), 12, QFont::DemiBold);

	// Hero header with drop shadow to pop against the gradient.
	auto *titleLabel = new QLabel(tr("SUDOKU"), this);
	titleLabel->setAlignment(Qt::AlignCenter);
	titleLabel->setFont(titleFont);
	titleLabel->setStyleSheet("padding: 12px 0; letter-spacing: 2px; color: #1a1a1a;");
	auto *titleShadow = new QGraphicsDropShadowEffect(this);
	titleShadow->setBlurRadius(24.0);
	titleShadow->setOffset(0.0, 4.0);
	titleShadow->setColor(QColor(0, 0, 0, 120));
	titleLabel->setGraphicsEffect(titleShadow);
	mainLayout->addWidget(titleLabel);

	// Difficulty picker and action buttons share a horizontal strip.
	auto *controlsLayout = new QHBoxLayout();
	controlsLayout->setSpacing(8);

	difficultyCombo_ = new QComboBox(this);
	difficultyCombo_->addItem(tr("Easy"), static_cast<int>(SudokuGenerator::EASY));
	difficultyCombo_->addItem(tr("Medium"), static_cast<int>(SudokuGenerator::MEDIUM));
	difficultyCombo_->addItem(tr("Hard"), static_cast<int>(SudokuGenerator::HARD));
	difficultyCombo_->setFont(controlFont);
	difficultyCombo_->setStyleSheet("QComboBox { background-color: #ffffff; border: 1px solid #d0d0d0; border-radius: 8px; padding: 6px 12px; font-weight: 600; color: #1a1a1a; } QComboBox::drop-down { border: none; } QComboBox QAbstractItemView { background-color: #ffffff; selection-background-color: #f0f0f0; }");

	auto *difficultyLabel = new QLabel(tr("Difficulty:"), this);
	difficultyLabel->setFont(controlFont);
	difficultyLabel->setStyleSheet("color: #4a4a4a; font-weight: 600;");
	controlsLayout->addWidget(difficultyLabel);
	controlsLayout->addWidget(difficultyCombo_);

	newGameButton_ = new QPushButton(tr("New Game"), this);
	solveButton_ = new QPushButton(tr("Solve"), this);
	loadButton_ = new QPushButton(tr("Load"), this);
	saveButton_ = new QPushButton(tr("Save"), this);
	resetButton_ = new QPushButton(tr("Reset"), this);

	// Centralized styling drives consistent hover/pressed states without duplicating style sheets.
	const auto buttonStyle = [](const QString &baseColor, const QString &hoverColor, const QString &textColor) {
		return QStringLiteral(
			"QPushButton { background-color: %1; border: none; border-radius: 16px; padding: 10px 18px; font-size: 15px; font-weight: 700; color: %3; }"
			"QPushButton:hover { background-color: %2; }"
			"QPushButton:pressed { background-color: %2; }"
			"QPushButton:disabled { background-color: #e0e0e0; color: #9e9e9e; }")
	    .arg(baseColor, hoverColor, textColor);
	};

	newGameButton_->setStyleSheet(buttonStyle("#ffb300", "#ffc046", "#263238"));
	solveButton_->setStyleSheet(buttonStyle("#8bc34a", "#9ccc65", "#0d5302"));
	loadButton_->setStyleSheet(buttonStyle("#29b6f6", "#4fc3f7", "#0d47a1"));
	saveButton_->setStyleSheet(buttonStyle("#26a69a", "#4db6ac", "#004d40"));
	resetButton_->setStyleSheet(buttonStyle("#ef5350", "#e57373", "#ffffff"));

	newGameButton_->setFont(buttonFont);
	solveButton_->setFont(buttonFont);
	loadButton_->setFont(buttonFont);
	saveButton_->setFont(buttonFont);
	resetButton_->setFont(buttonFont);

	controlsLayout->addWidget(newGameButton_);
	controlsLayout->addWidget(solveButton_);
	controlsLayout->addWidget(loadButton_);
	controlsLayout->addWidget(saveButton_);
	controlsLayout->addWidget(resetButton_);
	controlsLayout->addStretch();

	// Primary 9x9 grid with stretchy cells and custom delegate styling.
	boardTable_ = new QTableWidget(kBoardSize, kBoardSize, this);
	boardTable_->setSelectionMode(QAbstractItemView::SingleSelection);
	boardTable_->setSelectionBehavior(QAbstractItemView::SelectItems);
	boardTable_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);
	boardTable_->horizontalHeader()->setVisible(false);
	boardTable_->verticalHeader()->setVisible(false);
	boardTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	boardTable_->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	boardTable_->setShowGrid(true);
	boardTable_->setStyleSheet(
		"QTableWidget {"
		" background-color: #ffffff;"
		" gridline-color: #bdbdbd;"
		" font-size: 24px;"
		" selection-background-color: #e0e0e0;"
		" selection-color: #1a1a1a;"
		" border-radius: 12px;"
		" border: 1px solid #d0d0d0;"
		" }"
		"QTableWidget::item { padding: 0; }");
	boardTable_->setItemDelegate(new SudokuCellDelegate(boardTable_));

	QFont cellFont;
	cellFont.setPointSize(18);
	cellFont.setBold(true);

	// Pre-create table items so later updates only tweak content and flags.
	for (int row = 0; row < kBoardSize; ++row) {
		for (int col = 0; col < kBoardSize; ++col) {
			auto *item = new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignCenter);
			item->setFont(cellFont);
			boardTable_->setItem(row, col, item);
		}
	}

	mainLayout->addLayout(controlsLayout);
	mainLayout->addWidget(boardTable_);

	setCentralWidget(central);

	statusLabel_ = new QLabel(this);
	statusBar()->addPermanentWidget(statusLabel_, 1);
	statusBar()->setStyleSheet("QStatusBar { border: none; color: #4a4a4a; font-size: 14px; font-weight: 600; }");
	statusLabel_->setStyleSheet("color: #4a4a4a; font-weight: 600;");
	defaultStatusStyle_ = statusLabel_->styleSheet();
	toolbar_ = addToolBar(tr("Game"));
	toolbar_->setMovable(false);
	toolbar_->setStyleSheet("QToolBar { background: #ffffff; border: none; spacing: 6px; } QToolButton { color: #1a1a1a; font-weight: 600; }");
	toolbar_->setIconSize(QSize(24, 24));
	toolbar_->setToolButtonStyle(Qt::ToolButtonTextOnly);

	// Wire up buttons and table edits to their handlers.
	connect(newGameButton_, &QPushButton::clicked, this, &MainWindow::handleNewGame);
	connect(solveButton_, &QPushButton::clicked, this, &MainWindow::handleSolve);
	connect(loadButton_, &QPushButton::clicked, this, &MainWindow::handleLoad);
	connect(saveButton_, &QPushButton::clicked, this, &MainWindow::handleSave);
	connect(resetButton_, &QPushButton::clicked, this, &MainWindow::handleReset);
	connect(boardTable_, &QTableWidget::cellChanged, this, &MainWindow::handleCellChanged);
}

void MainWindow::setupMenus() {
	// Menu bar mirrors toolbar actions for keyboard-friendly access.
	menuBar()->setStyleSheet(QStringLiteral(
		"QMenuBar { background: #ffffff; color: #1a1a1a; font-weight: 600; }"
		"QMenuBar::item:selected { background: #f0f0f0; border-radius: 6px; }"
		"QMenu { background-color: #ffffff; color: #1a1a1a; border: 1px solid #d0d0d0; border-radius: 6px; }"
		"QMenu::item:selected { background-color: #f5f5f5; }"));
	auto *gameMenu = menuBar()->addMenu(tr("&Game"));

	newGameAction_ = new QAction(tr("New Game"), this);
	newGameAction_->setShortcut(QKeySequence::New);
	connect(newGameAction_, &QAction::triggered, this, &MainWindow::handleNewGame);

	solveAction_ = new QAction(tr("Solve"), this);
	solveAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
	connect(solveAction_, &QAction::triggered, this, &MainWindow::handleSolve);

	loadAction_ = new QAction(tr("Load"), this);
	loadAction_->setShortcut(QKeySequence::Open);
	connect(loadAction_, &QAction::triggered, this, &MainWindow::handleLoad);

	saveAction_ = new QAction(tr("Save"), this);
	saveAction_->setShortcut(QKeySequence::Save);
	connect(saveAction_, &QAction::triggered, this, &MainWindow::handleSave);

	resetAction_ = new QAction(tr("Reset"), this);
	resetAction_->setShortcut(QKeySequence::Refresh);
	connect(resetAction_, &QAction::triggered, this, &MainWindow::handleReset);

	gameMenu->addAction(newGameAction_);
	gameMenu->addAction(solveAction_);
	gameMenu->addSeparator();
	gameMenu->addAction(loadAction_);
	gameMenu->addAction(saveAction_);
	gameMenu->addSeparator();
	gameMenu->addAction(resetAction_);

	toolbar_->addAction(newGameAction_);
	toolbar_->addAction(solveAction_);
	toolbar_->addSeparator();
	toolbar_->addAction(loadAction_);
	toolbar_->addAction(saveAction_);
	toolbar_->addSeparator();
	toolbar_->addAction(resetAction_);
}

void MainWindow::clearBoard() {
	// Start from a blank grid and disable puzzle-dependent controls.
	currentBoard_ = SudokuBoard();
	initialBoard_ = SudokuBoard();
	hasInitialBoard_ = false;
	populateBoard(currentBoard_);
	updateActionStates();
}

void MainWindow::populateBoard(const SudokuBoard &board) {
	// Replicate the provided board snapshot without firing cellChanged.
	QSignalBlocker blocker(boardTable_);
	populating_ = true;
	const auto &data = board.getBoard();
	for (int row = 0; row < kBoardSize; ++row) {
		for (int col = 0; col < kBoardSize; ++col) {
			auto *item = boardTable_->item(row, col);
			if (!item) {
				item = new QTableWidgetItem();
				item->setTextAlignment(Qt::AlignCenter);
				boardTable_->setItem(row, col, item);
			}
			const int value = data[row][col];
			item->setText(value == 0 ? QString() : QString::number(value));
			updateCellAppearance(row, col, isInitialCell(row, col));
		}
	}
	populating_ = false;
	boardTable_->clearSelection();
}

void MainWindow::updateCellAppearance(int row, int column, bool isInitial) {
	auto *item = boardTable_->item(row, column);
	if (!item) {
		return;
	}

	// Lock clue cells while keeping player cells editable and visually distinct.
	Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	if (!isInitial) {
		flags |= Qt::ItemIsEditable;
		item->setBackground(editableCellColor());
	} else {
		item->setBackground(initialCellColor());
	}
	item->setTextAlignment(Qt::AlignCenter);
	item->setForeground(QColor(Qt::black));
	item->setFlags(flags);
}

void MainWindow::setStatusMessage(const QString &message, bool isError) {
	if (statusLabel_) {
		statusLabel_->setText(message);
		statusLabel_->setStyleSheet(isError ? QStringLiteral("color: #c62828;") : defaultStatusStyle_);
	}
	statusBar()->showMessage(message, kStatusTimeoutMs);
}

SudokuGenerator::Difficulity MainWindow::selectedDifficulty() const {
	if (!difficultyCombo_) {
		return SudokuGenerator::EASY;
	}
	return static_cast<SudokuGenerator::Difficulity>(difficultyCombo_->currentData().toInt());
}

bool MainWindow::isInitialCell(int row, int column) const {
	if (!hasInitialBoard_) {
		return false;
	}
	return initialBoard_.getBoard()[row][column] != 0;
}

bool MainWindow::puzzleLoaded() const {
	if (hasInitialBoard_) {
		return true;
	}
	const auto &grid = currentBoard_.getBoard();
	for (const auto &row : grid) {
		for (int value : row) {
			if (value != 0) {
				return true;
			}
		}
	}
	return false;
}

void MainWindow::updateActionStates() {
	const bool hasPuzzle = puzzleLoaded();
	if (solveButton_) solveButton_->setEnabled(hasPuzzle);
	if (resetButton_) resetButton_->setEnabled(hasPuzzle);
	if (saveButton_) saveButton_->setEnabled(hasPuzzle);
	if (solveAction_) solveAction_->setEnabled(hasPuzzle);
	if (resetAction_) resetAction_->setEnabled(hasPuzzle);
	if (saveAction_) saveAction_->setEnabled(hasPuzzle);
}

void MainWindow::handleNewGame() {
	try {
		// Generate a new puzzle of the requested difficulty and reset state caches.
		SudokuGenerator generator;
		generator.generate(selectedDifficulty());
		initialBoard_.setBoard(generator.getBoard());
		currentBoard_.setBoard(generator.getBoard());
		hasInitialBoard_ = true;
		populateBoard(currentBoard_);
		updateActionStates();
		setStatusMessage(tr("New puzzle generated."));
	} catch (const SudokuException &ex) {
		QMessageBox::critical(this, tr("Generation Failed"), QString::fromUtf8(ex.what()));
		setStatusMessage(tr("Unable to generate puzzle."), true);
	} catch (const std::exception &ex) {
		QMessageBox::critical(this, tr("Unexpected Error"), QString::fromUtf8(ex.what()));
		setStatusMessage(tr("Unexpected error during generation."), true);
	}
}

void MainWindow::handleSolve() {
	if (!puzzleLoaded()) {
		QMessageBox::information(this, tr("No Puzzle"), tr("Generate or load a puzzle first."));
		return;
	}

	try {
		// Run the backtracking solver on a copy of the current board.
		SudokuSolver solver(currentBoard_);
		if (solver.solve()) {
			currentBoard_.setBoard(solver.getBoard().getBoard());
			populateBoard(currentBoard_);
			setStatusMessage(tr("Puzzle solved automatically."));
		} else {
			QMessageBox::warning(this, tr("Unsolvable"), tr("The current puzzle configuration is unsolvable."));
			setStatusMessage(tr("Puzzle is unsolvable."), true);
		}
	} catch (const SudokuException &ex) {
		QMessageBox::warning(this, tr("Solver Error"), QString::fromUtf8(ex.what()));
		setStatusMessage(tr("Solver reported an error."), true);
	}
}

void MainWindow::handleLoad() {
	const QString fileName = QFileDialog::getOpenFileName(this, tr("Load Sudoku"), QString(), tr("Sudoku Files (*.txt);;All Files (*.*)"));
	if (fileName.isEmpty()) {
		return;
	}

	QString errorMessage;
	SudokuBoard loaded;
	if (!loadBoardFromFile(fileName, loaded, errorMessage)) {
		QMessageBox::critical(this, tr("Load Failed"), errorMessage);
		setStatusMessage(tr("Unable to load puzzle."), true);
		return;
	}

	initialBoard_.setBoard(loaded.getBoard());
	currentBoard_.setBoard(loaded.getBoard());
	hasInitialBoard_ = true;
	populateBoard(currentBoard_);
	updateActionStates();
	setStatusMessage(tr("Puzzle loaded from file."));
}

void MainWindow::handleSave() {
	if (!puzzleLoaded()) {
		QMessageBox::information(this, tr("Nothing to Save"), tr("Generate or load a puzzle before saving."));
		return;
	}

	const QString fileName = QFileDialog::getSaveFileName(this, tr("Save Sudoku"), QString(), tr("Sudoku Files (*.txt);;All Files (*.*)"));
	if (fileName.isEmpty()) {
		return;
	}

	try {
		// Persist the player's current progress for later restoration.
		currentBoard_.save(fileName.toStdString());
		setStatusMessage(tr("Puzzle saved."));
	} catch (const SudokuException &ex) {
		QMessageBox::critical(this, tr("Save Failed"), QString::fromUtf8(ex.what()));
		setStatusMessage(tr("Unable to save puzzle."), true);
	} catch (const std::exception &ex) {
		QMessageBox::critical(this, tr("Unexpected Error"), QString::fromUtf8(ex.what()));
		setStatusMessage(tr("Unexpected error while saving."), true);
	}
}

void MainWindow::handleReset() {
	if (!puzzleLoaded()) {
		clearBoard();
		setStatusMessage(tr("Board cleared."));
		return;
	}

	// Restore either the generated clues or a blank grid based on what exists.
	if (hasInitialBoard_) {
		currentBoard_.setBoard(initialBoard_.getBoard());
	} else {
		currentBoard_ = SudokuBoard();
	}
	populateBoard(currentBoard_);
	updateActionStates();
	setStatusMessage(hasInitialBoard_ ? tr("Puzzle reset to its original state.") : tr("Board cleared."));
}

void MainWindow::handleCellChanged(int row, int column) {
	if (populating_) {
		return;
	}

	if (row < 0 || row >= kBoardSize || column < 0 || column >= kBoardSize) {
		return;
	}

	auto *item = boardTable_->item(row, column);
	if (!item) {
		return;
	}

    // Mirror core validation rules while keeping the visual feedback responsive.
    try {
        if (isInitialCell(row, column)) {
            QSignalBlocker blocker(boardTable_);
            const int originalValue = initialBoard_.getValue(row + 1, column + 1);
            item->setText(originalValue == 0 ? QString() : QString::number(originalValue));
            return;
        }

        const QString text = item->text().trimmed();
        if (text.isEmpty()) {
            currentBoard_.setValue(row + 1, column + 1, 0, true);
            setStatusMessage(tr("Cell cleared."));
            updateCellAppearance(row, column, false);
            updateActionStates();
            return;
        }

        bool ok = false;
        const int value = text.toInt(&ok);
        if (!ok || value < 1 || value > 9) {
            QSignalBlocker blocker(boardTable_);
            item->setText(QString());
            setStatusMessage(tr("Enter numbers between 1 and 9."), true);
            return;
        }

        currentBoard_.setValue(row + 1, column + 1, 0, true);
        if (currentBoard_.setValue(row + 1, column + 1, value)) {
            QSignalBlocker blocker(boardTable_);
            item->setText(QString::number(value));
            setStatusMessage(tr("Move accepted."));
            updateCellAppearance(row, column, false);
            updateActionStates();
            if (currentBoard_.emptyCount() == 0) {
                setStatusMessage(tr("Congratulations! Puzzle completed."));
            }
        } else {
            currentBoard_.setValue(row + 1, column + 1, 0, true);
            QSignalBlocker blocker(boardTable_);
            item->setText(QString());
            setStatusMessage(tr("Invalid move."), true);
        }
    } catch (const SudokuException &ex) {
        QSignalBlocker blocker(boardTable_);
        item->setText(QString());
        setStatusMessage(QString::fromUtf8(ex.what()), true);
    } catch (const std::exception &) {
        QSignalBlocker blocker(boardTable_);
        item->setText(QString());
        setStatusMessage(tr("Unexpected error while editing cell."), true);
    }
}

int run_gui(int argc, char *argv[]) {
	QApplication app(argc, argv);
	MainWindow window;
	window.show();
	return app.exec();
}
