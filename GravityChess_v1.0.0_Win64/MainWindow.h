#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QMouseEvent>
#include <QPainter>
#include <QVector>


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
    // 添加按钮槽函数声明
    void on_Play_clicked();
    void on_Retract_clicked();
    void on_Replay_clicked();

private:
    // 棋盘坐标参数
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool gameOver; // 标记游戏是否已结束

    int getColumnFromX(int x);

    Ui::MainWindow *ui;
    const int ROWS = 6;
    const int COLS = 7;

    QPoint m_boardPoints[7]; // 存储每一列的中心点坐标 (x, y)
    int m_pieceRadius;       // 棋子半径

    // 游戏状态变量
    bool gameStarted;        // 游戏是否开始
    bool isYellowTurn;        // 是否轮到黑棋
    int boardState[7];       // 记录每列当前的落子高度 (0-5)，-1表示该列已满
    int boardPieces[7][6];   // 记录每个位置的颜色：0=空, 1=黄, 2=蓝

    // 交互变量
    int hoverCol;            // 鼠标当前悬停的列号 (-1为无)

    // 历史记录 (用于撤回)
    struct Move { int col; int row; };
    QVector<Move> moveHistory;
    // 胜利判定相关
    void checkWin(); // 检查胜利的函数
    QVector<QPoint> winningPieces; // 存储所有胜利的棋子坐标 (col, row)
};

#endif // MAINWINDOW_H