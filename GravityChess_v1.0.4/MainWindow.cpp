#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    this->setMouseTracking(true);
    ui->centralwidget->setMouseTracking(true);

    // 初始化游戏变量
    gameStarted = false;
    gameOver = false;
    isYellowTurn = true;
    hoverCol = -1;
    moveHistory.clear();

    // 初始化棋盘状态：全空
    for(int i=0; i<7; i++) {
        boardState[i] = -1; // 初始高度为-1，表示空
        for(int j=0; j<6; j++) {
            boardPieces[i][j] = 0;
        }
    }


    m_pieceRadius = 32; // 棋子半径

    int startY = 227;
    int startX = 61;    // 第一列中心X
    int stepX = 89;     // 间距

    // 只需要一个循环，因为 m_boardPoints 是一维数组 [7]
    for (int i = 0; i < 7; i++) {
        m_boardPoints[i] = QPoint(startX + i * stepX, startY);
    }

    // 锁定按钮
    ui->Retract->setEnabled(false);
    ui->Replay->setEnabled(false);

    // 用 new 实例化音效对象
    moveSoundEffect = new QSoundEffect(this);
    moveSoundEffect->setSource(QUrl("qrc:/new/prefix1/sounds/chessSound.wav"));
    moveSoundEffect->setVolume(0.5f);

    // 初始化其他变量
    lastVolume = 50;
    isMuted = false;

    ui->volumeSlider->setValue(50);

    rng.seed(std::random_device{}()); // 用当前时间种子初始化
    distAngle = std::uniform_real_distribution<double>(0.0, 2 * M_PI); // 角度 [0, 2π)
    distSpeed = std::uniform_real_distribution<double>(-0.1, 0.1);

}

void MainWindow::on_muteButton_clicked()
{
    isMuted = !isMuted; // 切换静音状态

    if (isMuted) {

        ui->volumeSlider->setValue(0);

    } else {

        ui->volumeSlider->setValue(lastVolume); // 恢复到静音前的音量

    }
}

void MainWindow::on_Play_clicked()
{
    // 标记游戏开始
    gameStarted = true;
    isYellowTurn = true; // 黑棋先手
    moveHistory.clear(); // 清空旧记录

    // 更新按钮状态
    ui->Retract->setEnabled(true);  // 启用撤回
    ui->Replay->setEnabled(true);   // 启用重玩
    ui->Play->setEnabled(false);    // 禁用开始（防止重复点击，可选）

    // 刷新界面
    update();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    // 游戏未开始不处理
    if (!gameStarted) return;

    // 获取鼠标X坐标
    int x = event->pos().x();

    // 调用辅助函数计算列号
    int newCol = getColumnFromX(x);

    // 如果列号发生变化，更新 hoverCol 并刷新界面
    if (newCol != hoverCol) {
        hoverCol = newCol;
        update(); // 触发 paintEvent 重绘预览
    }
}

// 辅助函数：根据鼠标X坐标计算列号
int MainWindow::getColumnFromX(int x)
{
    int col = -1;
    // 判定落子列数
    if (x >= (44 - 44))      col = 0;
    if (x >= (150 - 44))     col = 1;
    if (x >= (239 - 44))     col = 2;
    if (x >= (328 - 44))     col = 3;
    if (x >= (417 - 44))     col = 4;
    if (x >= (506 - 44))     col = 5;
    if (x >= (595 - 44))     col = 6;

    return col;
}

// 鼠标点击事件（落子）
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (!gameStarted || event->button() != Qt::LeftButton) return;
    if (event->button() != Qt::LeftButton) return;
    if (gameOver) return;

    // 获取列号
    int col = getColumnFromX(event->pos().x());
    if (col == -1) return; // 点击在无效区域

    // 检查该列是否已满
    if (boardState[col] >= 5) return;

    // 执行落子
    moveSoundEffect->play();// 落子音效

    boardState[col]++;
    int row = boardState[col];
    int color = isYellowTurn ? 1 : 2;
    boardPieces[col][row] = color;
    moveHistory.append({col, row});


    // 检查胜利 (此时还是当前玩家的回合)
    checkWin();

    // 切换回合
    isYellowTurn = !isYellowTurn;

    // 更新悬停预览为下一位玩家的棋子
    hoverCol = getColumnFromX(event->pos().x());

    // 刷新界面
    update();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // 先绘制背景
    QPixmap bgPixmap(":/new/prefix1/images/chessboard0.5.png");
    painter.drawPixmap(this->rect(), bgPixmap);

    // 加载图片资源
    QPixmap pixYellow(":/new/prefix1/images/yellowBall.png");
    QPixmap pixBlue(":/new/prefix1/images/blueBall.png");
    QPixmap pixYellowRing(":/new/prefix1/images/yellowRing.png");
    QPixmap pixBlueRing(":/new/prefix1/images/blueRing.png");
    QPixmap pixWhiteRing(":/new/prefix1/images/wightRing.png"); // 胜利白环

    // 统一缩放图片
    int imgSize = 64;
    pixYellow = pixYellow.scaled(imgSize, imgSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixBlue = pixBlue.scaled(imgSize, imgSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixYellowRing = pixYellowRing.scaled(imgSize, imgSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixBlueRing = pixBlueRing.scaled(imgSize, imgSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixWhiteRing = pixWhiteRing.scaled(imgSize, imgSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 绘制棋盘上的已有棋子
    for (int c = 0; c < 7; c++) {
        for (int r = 0; r <= 5; r++) {
            if (boardPieces[c][r] != 0) {
                int pieceX = 61 + c * 89;
                int pieceY = (227 + 5 * 89) - (r * 89);
                int drawX = pieceX - pixYellow.width() / 2;
                int drawY = pieceY - pixYellow.height() / 2;

                if (boardPieces[c][r] == 1) {
                    painter.drawPixmap(drawX, drawY, pixYellow);
                } else {
                    painter.drawPixmap(drawX, drawY, pixBlue);
                }
            }
        }
    }

    // 绘制顶部预览和落子圆环
    // 只有游戏进行中(!gameOver)、已开始、且鼠标悬停时才显示
    if (gameStarted && !gameOver && hoverCol != -1) {
        // 1. 绘制落子位置的圆环
        if (boardState[hoverCol] < 5) {
            int nextRow = boardState[hoverCol] + 1;
            int centerX = 61 + hoverCol * 89;
            int centerY = (227 + 5 * 89) - (nextRow * 89);
            int ringX = centerX - pixYellowRing.width() / 2;
            int ringY = centerY - pixYellowRing.height() / 2;

            if (isYellowTurn) {
                painter.drawPixmap(ringX, ringY, pixYellowRing);
            } else {
                painter.drawPixmap(ringX, ringY, pixBlueRing);
            }
        }

        // 绘制顶部悬浮预览
        int previewCenterX = 61 + hoverCol * 89;
        int previewDrawX = previewCenterX - pixYellow.width() / 2;
        int previewDrawY = 130 - pixYellow.height() / 2;

        if (isYellowTurn) {
            painter.drawPixmap(previewDrawX, previewDrawY, pixYellow);
        } else {
            painter.drawPixmap(previewDrawX, previewDrawY, pixBlue);
        }
    }

    // 无条件绘制胜利白环
    // 只要产生了胜利棋子，就要一直画出来，直到被清空
    for (const QPoint& pt : winningPieces) {
        int c = pt.x();
        int r = pt.y();
        int pieceX = 61 + c * 89;
        int pieceY = (227 + 5 * 89) - (r * 89);
        int drawX = pieceX - pixWhiteRing.width() / 2;
        int drawY = pieceY - pixWhiteRing.height() / 2;
        painter.drawPixmap(drawX, drawY, pixWhiteRing);
    }

    // --- 粒子和冲击波生成逻辑 ---
    // 检查是否游戏结束且胜利棋子存在，同时没有任何粒子或冲击波在活动时才生成
    if (!winningPieces.isEmpty() && gameOver && !hasSpawnedEffects && allParticles.empty() && activeShockwaves.empty()) {
        hasSpawnedEffects = true; // 标记特效已生成
        for (const QPoint& winPos : winningPieces) {
            int c = winPos.x();
            int r = winPos.y();
            int pieceX = 61 + c * 89;
            int pieceY = (227 + 5 * 89) - (r * 89);
            spawnParticlesAt(pieceX, pieceY); // 这会同时生成粒子和冲击波
        }
    }

    // --- 更新和绘制粒子 ---
    bool stillAlive = false; // 重置标志
    for (auto it = allParticles.begin(); it != allParticles.end();) {
        Particle& p = *it;

        // 1. 更新粒子状态
        p.x += p.vx;
        p.y += p.vy;
        p.life -= 0.001; // 假设 60 FPS，1秒减少 1.0 生命值

        // 2. 检查粒子是否存活
        if (p.life <= 0.0) {
            it = allParticles.erase(it); // 从容器中删除死亡粒子
            continue; // 跳过本次迭代的剩余部分，继续下一个
        }

        stillAlive = true; // 至少有一个粒子还活着

        // 3. 计算粒子颜色和大小 (保持不变)
        double alphaFactor = p.life / p.initialLife; // 计算当前透明度因子
        int alpha = static_cast<int>(alphaFactor * 255); // 转换为 0-255 的 alpha 值
        double brightness = pow(alphaFactor, 9);
        int baseR = 255; int baseG = 215; int baseB = 0;
        int highR = 255; int highG = 255; int highB = 255;
        int r = static_cast<int>(baseR + (highR - baseR) * brightness);
        int g = static_cast<int>(baseG + (highG - baseG) * brightness);
        int b = static_cast<int>(baseB + (highB - baseB) * brightness);
        if (alpha < 0) alpha = 0; // 确保 alpha 不小于 0
        QColor particleColor(r, g, b, alpha);

        // 4. 绘制粒子 (保持不变)
        painter.setBrush(particleColor);
        painter.setPen(Qt::NoPen); // 无边框
        double size = 2 * cbrt(p.life); // 粒子大小随生命值变化
        painter.drawEllipse(QPointF(p.x, p.y), size, size);

        ++it; // 移动到下一个粒子
    }

    // --- 绘制和更新冲击波动画（包含闪光） ---
    bool anyShockwaveOrGlowStillActive = false; // 合并标志
    for (auto it = activeShockwaves.begin(); it != activeShockwaves.end();) {
        ShockwaveInfo& sw = *it;
        if (!sw.isActive) {
            it = activeShockwaves.erase(it);
            continue;
        }

        int elapsed = sw.startTime.msecsTo(QTime::currentTime());
        bool stillActiveThisFrame = false; // 标记当前这个冲击波/闪光实例是否仍在活动

        // --- 更新和绘制冲击波 ---
        if (elapsed < sw.shockwaveDurationMs) { // 冲击波动画持续时间
            painter.setRenderHint(QPainter::Antialiasing);

            double progress = static_cast<double>(elapsed) / sw.shockwaveDurationMs;
            double radius = 32 + (0.5 * sw.shockwaveMaxRadius * progress); // 从32开始增长
            double width = sw.shockwaveInitialWidth * (1.0 - progress);
            int alpha = static_cast<int>(200 * (1.0 - progress));

            if (width > 0 && alpha > 0) {
                QPen pen(QColor(255, 255, 255, alpha), width);
                painter.setPen(pen);
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(QPointF(sw.centerX, sw.centerY), radius, radius);
            }
            stillActiveThisFrame = true; // 冲击波还在活动
        }

        // --- 更新和绘制中心闪光 ---
        if (sw.glowEnabled && elapsed < sw.glowDurationMs) { // 闪光持续时间
            painter.setRenderHint(QPainter::Antialiasing);

            double glowProgress = static_cast<double>(elapsed) / sw.glowDurationMs;
            double glowRadius = sw.glowMaxRadius * glowProgress; // 从0增长到最大

            // 修改 Alpha 计算方式，使用更小的幂次使其初始更亮，衰减更慢
            int glowAlpha = static_cast<int>(pow((1.0 - glowProgress), 0.6) * 255);

            if (glowAlpha > 0) { // 只有当透明度大于0时才绘制
                QRadialGradient gradient(sw.centerX, sw.centerY, glowRadius);
                // 中心颜色保持纯白，但Alpha由新公式计算
                gradient.setColorAt(0, QColor(255, 255, 255, glowAlpha));
                gradient.setColorAt(1, QColor(255, 255, 255, 0));       // 边缘

                painter.setBrush(gradient);
                painter.setPen(Qt::NoPen); // 无边框
                painter.drawEllipse(QPointF(sw.centerX, sw.centerY), glowRadius, glowRadius);
            }
            stillActiveThisFrame = true; // 闪光还在活动
        }

        if (stillActiveThisFrame) {
            anyShockwaveOrGlowStillActive = true; // 至少还有一个在活动
            ++it;
        } else {
            // 冲击波和闪光动画都结束了，移除该实例
            it = activeShockwaves.erase(it);
        }
    }

    // 统一的更新调用：只要有粒子、冲击波或闪光在活动，就刷新
    // 修改：变量名更改为 anyShockwaveOrGlowStillActive
    if (stillAlive || anyShockwaveOrGlowStillActive) {
        update();
    }
}

void MainWindow::on_Retract_clicked()
{
    if (moveHistory.isEmpty()) return;

    const Move& lastMove = moveHistory.last();
    int col = lastMove.col;
    int row = lastMove.row;

    boardPieces[col][row] = 0;
    boardState[col]--;
    moveHistory.removeLast();

    // 撤回后，游戏不再结束，清空胜利白环的记录
    gameOver = false;

    hasSpawnedEffects = false;

    winningPieces.clear(); // 撤回一步，胜利状态就解除了，白环要消失

    isYellowTurn = !isYellowTurn;

    allParticles.clear();                     // 2. 清除当前屏幕上可能存在的旧粒子
    activeShockwaves.clear();

    update();
}

void MainWindow::on_Replay_clicked()
{
    // 核心状态重置
    gameStarted = true;  // 确保游戏处于“已开始”状态
    gameOver = false;    // 清除胜利状态
    isYellowTurn = true;  // 恢复默认黑棋先手

    // 彻底清空棋盘数据
    for (int i = 0; i < 7; i++) {
        boardState[i] = -1; // 恢复每列高度为 -1（空）
        for (int j = 0; j < 6; j++) {
            boardPieces[i][j] = 0; // 清空棋子
        }
    }

    // 清空辅助数据
    moveHistory.clear();      // 清空历史记录
    winningPieces.clear();    // 清空胜利白圈记录
    hoverCol = -1;            // 清除悬停预览

    // 重置粒子相关状态

    allParticles.clear(); // 清空粒子容器
    activeShockwaves.clear();

    // 刷新界面
    update();
}

void MainWindow::checkWin()
{
    winningPieces.clear();
    // 如果游戏已经结束，不再进行判定（防止重复触发）
    if (gameOver) return;

    // 遍历棋盘上的每一个物理点
    // c: 列 (0-6), r: 物理行 (0-5, 0为最底部)
    for (int c = 0; c < 7; c++) {
        for (int r = 0; r < 6; r++) {
            int color = boardPieces[c][r];
            if (color == 0) continue; // 空位跳过

            // 1. 检查横向 (向右)
            // 检查 c+3 是否在边界内 (0~6)
            if (c <= 3) {
                if (boardPieces[c+1][r] == color &&
                    boardPieces[c+2][r] == color &&
                    boardPieces[c+3][r] == color) {

                    winningPieces.append(QPoint(c, r));
                    winningPieces.append(QPoint(c+1, r));
                    winningPieces.append(QPoint(c+2, r));
                    winningPieces.append(QPoint(c+3, r));
                }
            }

            // 检查纵向 (向上)
            // 检查 r+3 是否在边界内 (0~5)
            // 只有当 r <= 2 时，上方才有足够的空间连成4子
            if (r <= 2) {
                if (boardPieces[c][r+1] == color &&
                    boardPieces[c][r+2] == color &&
                    boardPieces[c][r+3] == color) {

                    winningPieces.append(QPoint(c, r));
                    winningPieces.append(QPoint(c, r+1));
                    winningPieces.append(QPoint(c, r+2));
                    winningPieces.append(QPoint(c, r+3));
                }
            }

            // 检查 "\" 型 (右上)
            if (c <= 3 && r <= 2) {
                if (boardPieces[c+1][r+1] == color &&
                    boardPieces[c+2][r+2] == color &&
                    boardPieces[c+3][r+3] == color) {

                    winningPieces.append(QPoint(c, r));
                    winningPieces.append(QPoint(c+1, r+1));
                    winningPieces.append(QPoint(c+2, r+2));
                    winningPieces.append(QPoint(c+3, r+3));
                }
            }

            // 检查 "/" 型 (左上)
            // 注意：这里检查的是向左上方延伸
            // c >= 3 (左边有3列), r <= 2 (上面有3行)
            if (c >= 3 && r <= 2) {
                if (boardPieces[c-1][r+1] == color &&
                    boardPieces[c-2][r+2] == color &&
                    boardPieces[c-3][r+3] == color) {

                    winningPieces.append(QPoint(c, r));
                    winningPieces.append(QPoint(c-1, r+1));
                    winningPieces.append(QPoint(c-2, r+2));
                    winningPieces.append(QPoint(c-3, r+3));
                }
            }
        }
    }

    // 如果找到了胜利棋子，标记游戏结束
    if (!winningPieces.isEmpty()) {
        gameOver = true; // 触发游戏结束状态
        qDebug() << "游戏结束！胜利棋子数量：" << winningPieces.size();
    }

    // 去重逻辑
    std::sort(winningPieces.begin(), winningPieces.end(),
              [](const QPoint& a, const QPoint& b) {
                  if (a.x() != b.x()) return a.x() < b.x();
                  return a.y() < b.y();
              }
              );
    winningPieces.erase(std::unique(winningPieces.begin(), winningPieces.end()), winningPieces.end());
}

void MainWindow::on_volumeSlider_valueChanged(int value)
{
    if (!moveSoundEffect) return;

    if (!isMuted) {
        lastVolume = value;
    }

    moveSoundEffect->setVolume(value / 100.0);

    // 拖动滑块且大于0时，自动取消静音
    if (value > 0 && isMuted) {
        isMuted = false;
        ui->muteButton->setChecked(false);
    }
}

void MainWindow::spawnParticlesAt(int centerX, int centerY) {
    const double radius = 32;
    const int numParticlesPerPoint = 36;
    const double overallDownwardSpeed = -0.05; // 整体向下趋势

    for (int i = 0; i < numParticlesPerPoint; ++i) {
        // 1. 获取随机角度和随机速度大小
        double angle = distAngle(rng);
        double speed = distSpeed(rng);

        // 2. 计算圆环上的起始位置
        double px = centerX + radius * cos(angle);
        double py = centerY + radius * sin(angle);

        // 3. 【修改点】先创建粒子对象 p
        Particle p;

        // 4. 然后再给 p 的属性赋值
        p.x = px;
        p.y = py;

        // 计算并赋值速度
        p.vx = cos(angle) * speed;
        p.vy = sin(angle) * speed + overallDownwardSpeed;

        p.life = 1.0;
        p.initialLife = 1.0;

        allParticles.append(p);
    }

    // 【新增】添加新的冲击波
    ShockwaveInfo newShockwave;
    newShockwave.centerX = centerX;
    newShockwave.centerY = centerY;
    newShockwave.startTime = QTime::currentTime(); // 记录开始时间
    newShockwave.isActive = true;
    activeShockwaves.push_back(newShockwave);

}



MainWindow::~MainWindow()
{

    delete ui;
}