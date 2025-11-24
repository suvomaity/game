#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QMessageBox>
#include <QMouseEvent>
#include <QEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    instructions_label = new QLabel(ui->frame); // It needs a parent for proper memory management
    instructions_label->setVisible(false); // Keep it hidden
    ui->frame->setMouseTracking(true);
    ui->frame->installEventFilter(this);
    ui->difficulty_comboBox->blockSignals(true);
    ui->difficulty_comboBox->addItems({"Easy", "Medium", "Hard", "NIGHTMARE"});
    ui->difficulty_comboBox->blockSignals(false);
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::gameLoop);
    resetGame();
}

MainWindow::~MainWindow()
{
    delete ui; // instructions_label is deleted automatically as a child of ui->frame
}

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
    killCount = 0; // Now represents dodged drops
    raindrops.clear();
    playerPosition = QPoint(worldWidth / 2, worldHeight - 2);
    gameTickCounter = 0;
    drawGame();
}

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

void MainWindow::on_reset_button_clicked() { resetGame(); }

void MainWindow::on_spinBox_gridSize_valueChanged(int arg1) { Q_UNUSED(arg1); resetGame(); }

void MainWindow::on_difficulty_comboBox_currentIndexChanged(int index) { Q_UNUSED(index); resetGame(); }

void MainWindow::on_leaderboard_button_clicked()
{
}

void MainWindow::on_instructions_button_clicked()
{
    QString instructionsText =
        "**Controls:**\n"
        " A / Left Arrow : Move Left\n"
        " D / Right Arrow : Move Right\n\n"
        "**Hazards:**\n"
        " - Rain Drops (Blue): Fall straight down. Dodge them to survive.\n\n"
        "**Goal:** Survive as long as possible in the rain and get a high score! Score increases over time survived.";
    QMessageBox::information(this, "Instructions", instructionsText);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {

    // Pause / Resume with P
    case Qt::Key_P:
        gameRunning = !gameRunning;   // toggle
        if (gameRunning)
            gameTimer->start();       // resume
        else
            gameTimer->stop();        // pause
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

void MainWindow::gameLoop()
{
    if (!gameRunning) return;
    updatePositions();
    spawnRain();
    checkCollisions();
    drawGame();
    gameTickCounter++;
}

void MainWindow::updatePositions()
{
    for (int i = 0; i < raindrops.size(); ++i) {
        raindrops[i].setY(raindrops[i].y() + 1);
        int dx = QRandomGenerator::global()->bounded(3) - 1; // -1, 0, or 1 for slight tilt
        int newX = raindrops[i].x() + dx;
        if (newX < 0) newX = 0;
        if (newX >= worldWidth) newX = worldWidth - 1;
        raindrops[i].setX(newX);
        if (raindrops[i].y() >= worldHeight) {
            killCount++; // Dodged a drop
            playerScore++; // Add to score for dodging
            ui->score_label_value->setText(QString::number(playerScore));
            raindrops.remove(i--);
        }
    }
}

void MainWindow::spawnRain()
{
    if (QRandomGenerator::global()->bounded(100) < spawnChance) {
        int spawnX = QRandomGenerator::global()->bounded(worldWidth);
        raindrops.append(QPoint(spawnX, 0));
    }
}

void MainWindow::updateScore(int points, bool countsAsKill)
{
    Q_UNUSED(countsAsKill);
    playerScore += points;
    ui->score_label_value->setText(QString::number(playerScore));
}

void MainWindow::handlePlayerHit()
{
    playerHealth--;
    ui->health_label_value->setText(QString::number(playerHealth));
    if (playerHealth <= 0) {
        gameRunning = false;
        gameTimer->stop();
    }
}

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

void MainWindow::drawCell(QPainter &painter, int cellX, int cellY, int gridSize, QColor color) {
    if (gridSize <= 0) return;
    painter.fillRect(cellX * gridSize, cellY * gridSize, gridSize, gridSize, color);
}

void MainWindow::clearCanvas() {
    QPixmap pm(ui->frame->size());
    pm.fill(Qt::black);
    ui->frame->setPixmap(pm);
}

void MainWindow::drawGrid(int gridSize) {
    if (gridSize <= 0) return;
    QPixmap pm = ui->frame->pixmap();
    QPainter painter(&pm);
    painter.setPen(QPen(QColor(50, 50, 50), 1));
    for(int i = 0; i <= ui->frame->width(); i += gridSize) painter.drawLine(i, 0, i, ui->frame->height());
    for(int i = 0; i <= ui->frame->height(); i += gridSize) painter.drawLine(0, i, ui->frame->width(), i);
    painter.end();
    ui->frame->setPixmap(pm);
}

void MainWindow::drawBlockyText(QPainter &painter, const QString& text, int startX, int startY, QColor color)
{
    int gridSize = ui->spinBox_gridSize->value();
    int currentX = startX;
    for (QChar c : text)
    {
        switch (c.toUpper().unicode())
        {
        case 'G':
            drawCell(painter, currentX + 1, startY, gridSize, color);
            drawCell(painter, currentX + 2, startY, gridSize, color);
            drawCell(painter, currentX, startY + 1, gridSize, color);
            drawCell(painter, currentX, startY + 2, gridSize, color);
            drawCell(painter, currentX, startY + 3, gridSize, color);
            drawCell(painter, currentX + 2, startY + 3, gridSize, color);
            drawCell(painter, currentX + 1, startY + 4, gridSize, color);
            drawCell(painter, currentX + 2, startY + 4, gridSize, color);
            currentX += 4;
            break;
        case 'A':
            for(int i=0; i<5; ++i) drawCell(painter, currentX, startY+i, gridSize, color);
            for(int i=0; i<5; ++i) drawCell(painter, currentX+2, startY+i, gridSize, color);
            drawCell(painter, currentX+1, startY, gridSize, color);
            drawCell(painter, currentX+1, startY+2, gridSize, color);
            currentX += 4;
            break;
        case 'M':
            for(int i=0; i<5; ++i) drawCell(painter, currentX, startY+i, gridSize, color);
            for(int i=0; i<5; ++i) drawCell(painter, currentX+2, startY+i, gridSize, color);
            drawCell(painter, currentX+1, startY+1, gridSize, color);
            currentX += 4;
            break;
        case 'E':
            for(int i=0; i<5; ++i) drawCell(painter, currentX, startY+i, gridSize, color);
            for(int i=0; i<3; ++i) drawCell(painter, currentX+i, startY, gridSize, color);
            for(int i=0; i<3; ++i) drawCell(painter, currentX+i, startY+2, gridSize, color);
            for(int i=0; i<3; ++i) drawCell(painter, currentX+i, startY+4, gridSize, color);
            currentX += 4;
            break;
        case 'O':
            for(int i=0; i<5; ++i) drawCell(painter, currentX, startY+i, gridSize, color);
            for(int i=0; i<5; ++i) drawCell(painter, currentX+2, startY+i, gridSize, color);
            drawCell(painter, currentX+1, startY, gridSize, color);
            drawCell(painter, currentX+1, startY+4, gridSize, color);
            currentX += 4;
            break;
        case 'V':
            for(int i=0; i<4; ++i) drawCell(painter, currentX, startY+i, gridSize, color);
            for(int i=0; i<4; ++i) drawCell(painter, currentX+2, startY+i, gridSize, color);
            drawCell(painter, currentX+1, startY+4, gridSize, color);
            currentX += 4;
            break;
        case 'R':
            for(int i=0; i<5; ++i) drawCell(painter, currentX, startY+i, gridSize, color);
            drawCell(painter, currentX+1, startY, gridSize, color);
            drawCell(painter, currentX+2, startY+1, gridSize, color);
            drawCell(painter, currentX+1, startY+2, gridSize, color);
            drawCell(painter, currentX+2, startY+3, gridSize, color);
            drawCell(painter, currentX+2, startY+4, gridSize, color);
            currentX += 4;
            break;
        case 'N':
            for(int i=0; i<5; ++i) drawCell(painter, currentX, startY+i, gridSize, color);
            for(int i=0; i<5; ++i) drawCell(painter, currentX+2, startY+i, gridSize, color);
            for(int i=0; i<5; ++i) drawCell(painter, currentX+1, startY+i, gridSize, color);
            currentX += 4;
            break;
        case 'D':
            for(int i=0; i<5; ++i) drawCell(painter, currentX, startY+i, gridSize, color);
            for(int i=0; i<5; ++i) drawCell(painter, currentX+2, startY+i, gridSize, color);
            drawCell(painter, currentX+1, startY, gridSize, color);
            drawCell(painter, currentX+1, startY+4, gridSize, color);
            currentX += 4;
            break;
        default:
            // For dots or other characters, simple representation
            if (c == '.') {
                drawCell(painter, currentX, startY + 2, gridSize, color);
                currentX += 2;
            } else {
                currentX += 4; // Skip unknown
            }
            break;
        }
    }
}

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
        return true; // Consume the event
    }
    return QMainWindow::eventFilter(watched, event);
}
