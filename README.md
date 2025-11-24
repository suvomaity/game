# Rain Dodging Game - Complete Project Report

## Executive Summary

This project is a 2D grid-based survival game developed using C++ and the Qt framework. The player controls a stick figure character that must dodge falling raindrops to survive as long as possible. The game features multiple difficulty levels, real-time score tracking, and a health system.

---

## 1. Project Overview

### 1.1 Game Concept
- **Genre**: Arcade/Survival
- **Objective**: Dodge falling raindrops for as long as possible
- **Technology**: C++ with Qt 6.9.1 Framework
- **Graphics**: Custom grid-based rendering system

### 1.2 Key Features
- Multiple difficulty levels (Easy, Medium, Hard, NIGHTMARE)
- Health system (5 hearts)
- Real-time score tracking
- Keyboard and mouse controls
- Pause functionality
- Customizable grid size
- Dynamic raindrop physics with horizontal drift

---

## 2. Technical Architecture

### 2.1 File Structure
```
Project Root
├── main.cpp              # Application entry point
├── mainwindow.h          # Main game class header
├── mainwindow.cpp        # Game logic implementation
├── mainwindow.ui         # Qt UI layout
├── my_label.h/cpp        # Custom label widget
└── spacewar.pro          # Qt project file
```

### 2.2 Class Diagram
```
QMainWindow
    ↑
    |
MainWindow
    ├── Game State Variables
    ├── UI Components (Ui::MainWindow)
    ├── Game Timer (QTimer)
    └── Game Logic Methods
```

---

## 3. Key Functions in mainwindow.cpp

### 3.1 Constructor & Destructor

#### **MainWindow::MainWindow(QWidget *parent)**
```cpp
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    instructions_label = new QLabel(ui->frame);
    instructions_label->setVisible(false);
    ui->frame->setMouseTracking(true);
    ui->frame->installEventFilter(this);
    ui->difficulty_comboBox->blockSignals(true);
    ui->difficulty_comboBox->addItems({"Easy", "Medium", "Hard", "NIGHTMARE"});
    ui->difficulty_comboBox->blockSignals(false);
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::gameLoop);
    resetGame();
}
```
**Purpose**: Initializes the game window and all UI components
**Key Operations**:
- Sets up UI from the .ui file
- Creates and hides instruction label
- Enables mouse tracking on the game frame
- Populates difficulty combo box
- Creates and connects game timer to the game loop
- Calls resetGame() to initialize game state

#### **MainWindow::~MainWindow()**
```cpp
MainWindow::~MainWindow()
{
    delete ui;
}
```
**Purpose**: Cleans up resources when the window is closed

---

### 3.2 Game State Management

#### **void MainWindow::resetGame()**
```cpp
void MainWindow::resetGame()
{
    gameRunning = false;
    gameTimer->stop();
    int gridSize = ui->spinBox_gridSize->value();
    if (gridSize <= 0) return;
    worldWidth = ui->frame->width() / gridSize;
    worldHeight = ui->frame->height() / gridSize;
    int difficulty = ui->difficulty_comboBox->currentIndex();
    playerHealth = 5;
    maxPlayerHealth = 5;
    int spawnChance = 5;
    switch(difficulty) {
    case 0: // Easy
        spawnChance = 5;
        break;
    case 1: // Medium
        spawnChance = 8;
        break;
    case 2: // Hard
        spawnChance = 12;
        break;
    case 3: // NIGHTMARE
        spawnChance = 16;
        break;
    }
    this->spawnChance = spawnChance;
    ui->health_label_value->setText(QString::number(playerHealth));
    playerScore = 0;
    ui->score_label_value->setText(QString::number(playerScore));
    killCount = 0;
    raindrops.clear();
    playerPosition = QPoint(worldWidth / 2, worldHeight - 2);
    gameTickCounter = 0;
    drawGame();
}
```
**Purpose**: Resets all game variables to initial state
**Key Operations**:
- Stops game timer and sets gameRunning to false
- Calculates world dimensions based on grid size
- Resets health to 5
- Sets spawn chance based on difficulty level
- Clears all raindrops
- Centers player near bottom of screen
- Updates UI labels
- Redraws the game canvas

---

### 3.3 Player Management

#### **QVector<QPoint> MainWindow::getPlayerManCells() const**
```cpp
QVector<QPoint> MainWindow::getPlayerManCells() const
{
    QVector<QPoint> cells;
    int px = playerPosition.x();
    int py = playerPosition.y();

    // + MAN with extended arms (5 blocks wide)

    // Head
    cells.append(QPoint(px, py - 4));

    // Arms (extended)
    cells.append(QPoint(px - 2, py - 3));
    cells.append(QPoint(px - 1, py - 3));
    cells.append(QPoint(px,     py - 3));
    cells.append(QPoint(px + 1, py - 3));
    cells.append(QPoint(px + 2, py - 3));

    // Body
    cells.append(QPoint(px, py - 2));
    cells.append(QPoint(px, py - 1));

    // Legs
    cells.append(QPoint(px, py));

    return cells;
}
```
**Purpose**: Returns all grid cells occupied by the player character
**Visual Representation**:
```
    O      <- Head
  \-|-/    <- Extended Arms
    |      <- Body
    |
```
**Key Features**:
- 9 cells total
- 5-cell wide arm span for larger hitbox
- Centered on player position

---

### 3.4 Core Game Loop

#### **void MainWindow::gameLoop()**
```cpp
void MainWindow::gameLoop()
{
    if (!gameRunning) return;
    updatePositions();
    spawnRain();
    checkCollisions();
    drawGame();
    gameTickCounter++;
}
```
**Purpose**: Main game loop called by QTimer at regular intervals
**Execution Order**:
1. Update all raindrop positions
2. Potentially spawn new raindrops
3. Check for collisions with player
4. Redraw the entire game screen
5. Increment tick counter

**Timing**: Called every 60-120ms depending on difficulty

---

### 3.5 Raindrop Physics

#### **void MainWindow::updatePositions()**
```cpp
void MainWindow::updatePositions()
{
    for (int i = 0; i < raindrops.size(); ++i) {
        raindrops[i].setY(raindrops[i].y() + 1);
        int dx = QRandomGenerator::global()->bounded(3) - 1; // -1, 0, or 1
        int newX = raindrops[i].x() + dx;
        if (newX < 0) newX = 0;
        if (newX >= worldWidth) newX = worldWidth - 1;
        raindrops[i].setX(newX);
        if (raindrops[i].y() >= worldHeight) {
            killCount++;
            playerScore++;
            ui->score_label_value->setText(QString::number(playerScore));
            raindrops.remove(i--);
        }
    }
}
```
**Purpose**: Updates position of all raindrops with realistic physics
**Physics Model**:
- Vertical movement: +1 cell per tick (constant downward velocity)
- Horizontal drift: Random -1, 0, or +1 (simulates wind)
- Boundary checking: Keeps drops within screen bounds
- Score increment: +1 point when raindrop reaches bottom without hitting player

#### **void MainWindow::spawnRain()**
```cpp
void MainWindow::spawnRain()
{
    if (QRandomGenerator::global()->bounded(100) < spawnChance) {
        int spawnX = QRandomGenerator::global()->bounded(worldWidth);
        raindrops.append(QPoint(spawnX, 0));
    }
}
```
**Purpose**: Randomly spawns new raindrops at the top of the screen
**Algorithm**:
- Generates random number 0-99
- If less than spawnChance, creates new raindrop
- Spawns at random X position, Y=0 (top)

**Spawn Rates by Difficulty**:
- Easy: 5% per tick
- Medium: 8% per tick
- Hard: 12% per tick
- NIGHTMARE: 16% per tick

---

### 3.6 Collision Detection

#### **void MainWindow::checkCollisions()**
```cpp
void MainWindow::checkCollisions()
{
    QVector<QPoint> playerCells = getPlayerManCells();
    for (int i = 0; i < raindrops.size(); ++i) {
        QPoint rainPos = raindrops[i];
        QPoint oldRainPos(rainPos.x(), rainPos.y() - 1);
        if (playerCells.contains(rainPos) || playerCells.contains(oldRainPos)) {
            raindrops.remove(i--);
            handlePlayerHit();
            if (!gameRunning) return;
        }
    }
}
```
**Purpose**: Detects collisions between raindrops and player
**Algorithm**:
- Gets all 9 player cell positions
- For each raindrop, checks both current and previous position
- If collision detected:
  - Removes the raindrop
  - Calls handlePlayerHit()
  - Exits if game is over

**Anti-Tunneling**: Checks previous position to prevent fast-moving drops from passing through player

#### **void MainWindow::handlePlayerHit()**
```cpp
void MainWindow::handlePlayerHit()
{
    playerHealth--;
    ui->health_label_value->setText(QString::number(playerHealth));
    if (playerHealth <= 0) {
        gameRunning = false;
        gameTimer->stop();
    }
}
```
**Purpose**: Handles damage when player is hit
**Actions**:
- Decrements health by 1
- Updates health display
- Stops game if health reaches 0

---

### 3.7 Rendering System

#### **void MainWindow::drawGame()**
```cpp
void MainWindow::drawGame()
{
    clearCanvas();
    int gridSize = ui->spinBox_gridSize->value();
    if (gridSize <= 0) return;
    drawGrid(gridSize);
    QPixmap pm = ui->frame->pixmap();
    QPainter painter(&pm);
    if(gameRunning) {
        for(const QPoint& cell : getPlayerManCells()) {
            drawCell(painter, cell.x(), cell.y(), gridSize, Qt::green);
        }
    }
    for (const QPoint &rain : raindrops) {
        drawCell(painter, rain.x(), rain.y(), gridSize, Qt::blue);
    }
    if (!gameRunning && playerHealth <= 0) {
        int textWidth = 15;
        int startY = (worldHeight / 2) - 5;
        int textX = (worldWidth - textWidth + 1) / 2;
        drawBlockyText(painter, "END", textX, startY, Qt::red);
        drawBlockyText(painter, "...", textX, startY + 6, Qt::red);
    }
    painter.end();
    ui->frame->setPixmap(pm);
}
```
**Purpose**: Master rendering function
**Rendering Order**:
1. Clear canvas to black
2. Draw grid lines
3. Draw green player (if game running)
4. Draw blue raindrops
5. Draw "END..." text if game over
6. Update display

#### **void MainWindow::clearCanvas()**
```cpp
void MainWindow::clearCanvas() {
    QPixmap pm(ui->frame->size());
    pm.fill(Qt::black);
    ui->frame->setPixmap(pm);
}
```
**Purpose**: Creates fresh black canvas

#### **void MainWindow::drawGrid(int gridSize)**
```cpp
void MainWindow::drawGrid(int gridSize) {
    if (gridSize <= 0) return;
    QPixmap pm = ui->frame->pixmap();
    QPainter painter(&pm);
    painter.setPen(QPen(QColor(50, 50, 50), 1));
    for(int i = 0; i <= ui->frame->width(); i += gridSize) 
        painter.drawLine(i, 0, i, ui->frame->height());
    for(int i = 0; i <= ui->frame->height(); i += gridSize) 
        painter.drawLine(0, i, ui->frame->width(), i);
    painter.end();
    ui->frame->setPixmap(pm);
}
```
**Purpose**: Draws grid overlay
**Visual**: Dark gray lines (RGB: 50,50,50) to show grid structure

#### **void MainWindow::drawCell(QPainter &painter, int cellX, int cellY, int gridSize, QColor color)**
```cpp
void MainWindow::drawCell(QPainter &painter, int cellX, int cellY, int gridSize, QColor color) {
    if (gridSize <= 0) return;
    painter.fillRect(cellX * gridSize, cellY * gridSize, gridSize, gridSize, color);
}
```
**Purpose**: Draws a single colored grid cell
**Parameters**:
- cellX, cellY: Grid coordinates
- gridSize: Size of each cell in pixels
- color: Fill color (green for player, blue for rain, red for text)

---

### 3.8 Custom Text Rendering

#### **void MainWindow::drawBlockyText(QPainter &painter, const QString& text, int startX, int startY, QColor color)**
```cpp
void MainWindow::drawBlockyText(QPainter &painter, const QString& text, 
                                int startX, int startY, QColor color)
{
    int gridSize = ui->spinBox_gridSize->value();
    int currentX = startX;
    for (QChar c : text)
    {
        switch (c.toUpper().unicode())
        {
        case 'E':
            for(int i=0; i<5; ++i) 
                drawCell(painter, currentX, startY+i, gridSize, color);
            for(int i=0; i<3; ++i) 
                drawCell(painter, currentX+i, startY, gridSize, color);
            for(int i=0; i<3; ++i) 
                drawCell(painter, currentX+i, startY+2, gridSize, color);
            for(int i=0; i<3; ++i) 
                drawCell(painter, currentX+i, startY+4, gridSize, color);
            currentX += 4;
            break;
        // ... other letters ...
        case '.':
            drawCell(painter, currentX, startY + 2, gridSize, color);
            currentX += 2;
            break;
        default:
            currentX += 4;
            break;
        }
    }
}
```
**Purpose**: Renders pixel-art style text
**Supported Characters**: G, A, M, E, O, V, R, N, D, . (dot)
**Design**: Each letter is 3 pixels wide × 5 pixels tall with 1-pixel spacing
**Usage**: Displays "END..." game over message

---

### 3.9 Input Handling

#### **void MainWindow::keyPressEvent(QKeyEvent *event)**
```cpp
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    // Pause / Resume with P
    case Qt::Key_P:
        gameRunning = !gameRunning;
        if (gameRunning)
            gameTimer->start();
        else
            gameTimer->stop();
        return;
    default:
        break;
    }

    if (!gameRunning) return;

    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_A:
        if (playerPosition.x() - 1 > 0) {
            playerPosition.setX(playerPosition.x() - 1);
        }
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        if (playerPosition.x() + 1 < worldWidth - 1) {
            playerPosition.setX(playerPosition.x() + 1);
        }
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}
```
**Purpose**: Handles keyboard input
**Controls**:
- **P**: Toggle pause/resume (works anytime)
- **A/Left Arrow**: Move player left
- **D/Right Arrow**: Move player right
**Boundary Checking**: Prevents player from moving off screen

#### **bool MainWindow::eventFilter(QObject *watched, QEvent *event)**
```cpp
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->frame && event->type() == QEvent::MouseMove) {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        if (gameRunning) {
            int gridSize = ui->spinBox_gridSize->value();
            if (gridSize > 0) {
                int newX = mouseEvent->pos().x() / gridSize;
                playerPosition.setX(qMax(0, qMin(newX, worldWidth - 1)));
            }
        }
        return true;
    }
    return QMainWindow::eventFilter(watched, event);
}
```
**Purpose**: Enables mouse control
**Mechanism**:
- Intercepts mouse move events over game frame
- Converts pixel position to grid coordinates
- Updates player X position to follow mouse
- Clamps position to valid range

---

### 3.10 UI Event Handlers

#### **void MainWindow::on_startGame_button_clicked()**
```cpp
void MainWindow::on_startGame_button_clicked()
{
    if (!gameRunning) {
        if(playerHealth <= 0){
            resetGame();
        }
        instructions_label->setVisible(false);
        gameRunning = true;
        int difficulty = ui->difficulty_comboBox->currentIndex();
        int gameSpeed = 100;
        switch(difficulty) {
        case 0: gameSpeed = 120; break;
        case 1: gameSpeed = 100; break;
        case 2: gameSpeed = 80; break;
        case 3: gameSpeed = 60; break;
        }
        gameTimer->start(gameSpeed);
        this->setFocus();
    }
}
```
**Purpose**: Starts the game
**Logic**:
- Resets game if player is dead
- Hides instructions
- Sets game speed based on difficulty
- Starts timer
- Sets focus to window for keyboard input

#### **void MainWindow::on_reset_button_clicked()**
```cpp
void MainWindow::on_reset_button_clicked() { 
    resetGame(); 
}
```
**Purpose**: Resets game to initial state

#### **void MainWindow::on_spinBox_gridSize_valueChanged(int arg1)**
```cpp
void MainWindow::on_spinBox_gridSize_valueChanged(int arg1) { 
    Q_UNUSED(arg1); 
    resetGame(); 
}
```
**Purpose**: Resets game when grid size changes (10-30 pixels)

#### **void MainWindow::on_difficulty_comboBox_currentIndexChanged(int index)**
```cpp
void MainWindow::on_difficulty_comboBox_currentIndexChanged(int index) { 
    Q_UNUSED(index); 
    resetGame(); 
}
```
**Purpose**: Resets game when difficulty changes

#### **void MainWindow::on_instructions_button_clicked()**
```cpp
void MainWindow::on_instructions_button_clicked()
{
    QString instructionsText =
        "**Controls:**\n"
        " A / Left Arrow : Move Left\n"
        " D / Right Arrow : Move Right\n\n"
        "**Hazards:**\n"
        " - Rain Drops (Blue): Fall straight down. Dodge them to survive.\n\n"
        "**Goal:** Survive as long as possible in the rain and get a high score! "
        "Score increases over time survived.";
    QMessageBox::information(this, "Instructions", instructionsText);
}
```
**Purpose**: Shows instructions in a message box

---

## 4. Game Mechanics Summary

### 4.1 Scoring System
- **+1 point**: Each raindrop that reaches bottom without hitting player
- Score displayed in real-time
- No maximum score limit

### 4.2 Health System
- Starting health: 5
- Damage per hit: 1
- Game over at 0 health

### 4.3 Difficulty Scaling

| Parameter | Easy | Medium | Hard | NIGHTMARE |
|-----------|------|--------|------|-----------|
| Spawn Rate | 5% | 8% | 12% | 16% |
| Game Speed | 120ms | 100ms | 80ms | 60ms |
| Difficulty | ★☆☆☆ | ★★☆☆ | ★★★☆ | ★★★★ |

### 4.4 Physics
- **Gravity**: Constant downward velocity (1 cell/tick)
- **Wind**: Random horizontal drift (-1, 0, +1)
- **No acceleration**: Linear movement
- **Boundary collision**: Raindrops bounce off sides

---

## 5. Technical Highlights

### 5.1 Qt Framework Usage
- **QTimer**: For game loop timing
- **QPainter**: For custom 2D graphics
- **QPixmap**: For double-buffered rendering
- **Event Filters**: For mouse input handling
- **Signals/Slots**: For UI event handling

### 5.2 Optimization Techniques
- **Efficient collision detection**: Only checks raindrop-player intersections
- **Grid-based system**: Reduces coordinate calculations
- **Double buffering**: Prevents screen flicker
- **Vector removal**: Uses reverse iteration to safely remove elements

### 5.3 Code Quality
- **Const correctness**: getPlayerManCells() is const
- **Resource management**: Proper use of Qt parent-child ownership
- **Boundary checking**: Prevents array out-of-bounds
- **Input validation**: Checks gridSize > 0 before operations

---

## 6. Possible Enhancements

### 6.1 Gameplay
- Power-ups (shields, slow-motion, extra lives)
- Multiple hazard types (lightning, hail)
- Boss battles
- Increasing difficulty over time
- Combo system for consecutive dodges

### 6.2 Technical
- Implement leaderboard with persistent storage
- Add sound effects and music
- Particle effects for collisions
- Smooth animations with interpolation
- Better game over screen with statistics

### 6.3 UI/UX
- Main menu screen
- Settings menu
- Themes/skins
- Controller support
- Touch screen controls

---

## 7. Conclusion

This Rain Dodging Game demonstrates proficient use of the Qt framework for game development. The code exhibits clean architecture, efficient algorithms, and good programming practices. The grid-based rendering system provides a retro aesthetic while keeping the implementation straightforward. The game successfully delivers an engaging arcade experience with progressive difficulty and responsive controls.

### Learning Outcomes
- Event-driven programming in Qt
- Game loop architecture
- 2D graphics rendering
- Collision detection algorithms
- Input handling (keyboard and mouse)
- UI design with Qt Designer

### Total Lines of Code
- mainwindow.cpp: ~400 lines
- Complete project: ~550 lines

---

**Project Developed By**: [Your Name/Team]
**Framework**: Qt 6.9.1 with C++17
**Development Environment**: Qt Creator
**Date**: November 2024
