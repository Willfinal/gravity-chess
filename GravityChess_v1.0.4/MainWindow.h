#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QMouseEvent>
#include <QPainter>
#include <QVector>
#include <QSoundEffect>
#include <random>
#include <cmath>
#include <QElapsedTimer>
#include <QTime>

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

    void on_volumeSlider_valueChanged(int value);
    void on_muteButton_clicked();

private:
    // 基础 UI 和 音效
    Ui::MainWindow *ui;
    QSoundEffect *moveSoundEffect;
    bool isMuted;
    int lastVolume;

    // 棋盘常量与坐标
    const int ROWS = 6;
    const int COLS = 7;
    QPoint m_boardPoints[7]; // 存储每一列的中心点坐标 (x, y)
    int m_pieceRadius;       // 棋子半径

    // 游戏状态变量
    bool gameStarted;        // 游戏是否开始
    bool gameOver;           // 标记游戏是否已结束
    bool isYellowTurn;       // 是否轮到黄棋
    int boardState[7];       // 记录每列当前的落子高度 (-1 到 5)
    int boardPieces[7][6];   // 记录每个位置的颜色：0=空, 1=黄, 2=蓝

    // 交互变量
    int hoverCol;            // 鼠标当前悬停的列号 (-1为无)

    // 历史记录 (用于撤回)
    struct Move { int col; int row; };
    QVector<Move> moveHistory;

    // 胜利判定相关
    void checkWin();
    QVector<QPoint> winningPieces;

    // 1. 定义粒子结构体
    struct Particle {
        double x, y;          // 位置
        double vx, vy;        // 速度
        double life;          // 当前生命值 (1.0 -> 0.0)
        double initialLife;   // 初始生命值 (用于计算透明度)
    };

    // 2. 成员变量
    QVector<Particle> allParticles;             // 存储粒子的容器
    std::mt19937 rng;                           // 随机数引擎
    std::uniform_real_distribution<double> distAngle; // 角度分布 [0, 2*PI)
    std::uniform_real_distribution<double> distSpeed; //速度大小随机分布
    // std::uniform_real_distribution<double> distSpeed; // 如果需要随机速度大小，可以取消注释

    bool particlesSpawnedForWinningPieces;      // 标记粒子是否已生成 (初始化在构造函数里做)

    // 3. 函数声明
    void spawnParticlesAt(int centerX, int centerY); // 生成粒子

    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    int getColumnFromX(int x);



    // 冲击波和闪光效果
    struct ShockwaveInfo {
        double centerX;
        double centerY;
        QTime startTime;
        bool isActive;

        // 冲击波参数
        double shockwaveMaxRadius = 50.0;  // 冲击波最大半径
        double shockwaveInitialWidth = 5.0; // 冲击波初始宽度
        int shockwaveDurationMs = 1000;    // 冲击波动画持续时间 (毫秒)

        // 闪光参数
        bool glowEnabled = true;           // 是否启用中心闪光（开关）
        double glowMaxRadius = 32.0;      // 闪光最大半径
        double glowDurationMs = 300.0;    // 闪光持续时间 (毫秒)
    };

    std::vector<ShockwaveInfo> activeShockwaves; // 存储所有活跃的冲击波

    bool hasSpawnedEffects = false;

};

#endif // MAINWINDOW_H