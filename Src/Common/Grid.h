#pragma once
#include <DxLib.h>

class Grid {
public:
    Grid(int size = 10, int count = 20, int color = GetColor(150, 150, 150));

    // 描画
    void Draw() const;

    // セルサイズを変更
    void SetCellSize(int size);

    // グリッド半径を変更
    void SetHalfCount(int count);

    // 色を変更
    void SetColor(int color);

private:
    int cellSize;   // 1マスのサイズ
    int halfCount;  // グリッドの半分のマス数
    int lineColor;  // 線の色
};

