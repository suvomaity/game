#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QPoint>
#include <QVector>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startGame_button_clicked();
    void on_reset_button_clicked();
    void on_spinBox_gridSize_valueChanged(int arg1);
    void on_difficulty_comboBox_currentIndexChanged(int index);
    void on_leaderboard_button_clicked();
    void on_instructions_button_clicked();
    void gameLoop();

private:
    Ui::MainWindow *ui;
    QLabel *instructions_label;
    QTimer *gameTimer;

    bool gameRunning;
    int worldWidth;
    int worldHeight;
    int playerHealth;
    int maxPlayerHealth;
    int playerScore;
    int killCount;
    int spawnChance;
    int gameTickCounter;
    QPoint playerPosition;
    QVector<QPoint> raindrops;

    void resetGame();
    QVector<QPoint> getPlayerManCells() const;
    void updatePositions();
    void spawnRain();
    void updateScore(int points, bool countsAsKill);
    void handlePlayerHit();
    void checkCollisions();
    void drawGame();
    void drawCell(QPainter &painter, int cellX, int cellY, int gridSize, QColor color);
    void clearCanvas();
    void drawGrid(int gridSize);
    void drawBlockyText(QPainter &painter, const QString& text, int startX, int startY, QColor color);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
};
#endif // MAINWINDOW_H
