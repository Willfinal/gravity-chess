#include "mainwindow.h"
#include "ui_mainwindow.h"

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

    int startY = 227;   // Y坐标 (根据你的背景图调整，之前可能是227？)
    int startX = 61;    // 第一列中心X
    int stepX = 89;     // 间距

    // 只需要一个循环，因为 m_boardPoints 是一维数组 [7]
    for (int i = 0; i < 7; i++) {
        m_boardPoints[i] = QPoint(startX + i * stepX, startY);
    }

    // 锁定按钮
    ui->Retract->setEnabled(false);
    ui->Replay->setEnabled(false);

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
    // 复用你原来的“左44，右45”硬判定逻辑
    if (x >= (61 - 44))      col = 0;
    if (x >= (150 - 44))     col = 1;
    if (x >= (239 - 44))     col = 2;
    if (x >= (328 - 44))     col = 3;
    if (x >= (417 - 44))     col = 4;
    if (x >= (506 - 44))     col = 5;
    if (x >= (595 - 44))     col = 6;

    // 边界修正
    if (col == 6 && x > (595 + 45)) col = 6;
    if (x < (61 - 44)) col = 0;

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
    winningPieces.clear(); // 撤回一步，胜利状态就解除了，白环要消失

    isYellowTurn = !isYellowTurn;
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
MainWindow::~MainWindow()
{
    delete ui;}