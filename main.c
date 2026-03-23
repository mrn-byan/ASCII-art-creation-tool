#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <math.h>

// 70段階の密度スケール（写真のディテールを拾うために拡張）
const char* G_SCALE = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
const int G_COUNT = 69;

const int MAX_WIDTH = 200; // 写真用に少し幅を広げると綺麗に見えます

int main() {
    SetConsoleOutputCP(65001);
    char path[256];
    int width, height, channels;

    printf("画像のパスを入力してください: ");
    if (fgets(path, sizeof(path), stdin) == NULL) return 1;
    path[strcspn(path, "\n")] = 0;

    if (path[0] == '"') {
        memmove(path, path + 1, strlen(path));
        if (path[strlen(path) - 1] == '"') path[strlen(path) - 1] = 0;
    }

    unsigned char *img = stbi_load(path, &width, &height, &channels, 1);
    if (img == NULL) {
        printf("エラー: 画像を読み込めませんでした。\n");
        return 1;
    }

    // --- 1. 自動コントラスト調整のための事前スキャン ---
    unsigned char min_val = 255, max_val = 0;
    for (int i = 0; i < width * height; i++) {
        if (img[i] < min_val) min_val = img[i];
        if (img[i] > max_val) max_val = img[i];
    }
    // 全く同じ色の画像の場合の回避
    if (min_val == max_val) max_val = min_val + 1;

    double scale = (double)MAX_WIDTH / width;
    if (scale > 1.0) scale = 1.0;
    int new_width = (int)(width * scale);
    int new_height = (int)(height * scale * 0.48); // ターミナルの行間に合わせて微調整

    printf("\n--- HIGH-DEF ASCII ART (%d x %d) ---\n\n", new_width, new_height);

    // --- 2. 描画ループ ---
    for (int y = 0; y < new_height; y++) {
        for (int x = 0; x < new_width; x++) {
            int old_x = (int)(x / scale);
            int old_y = (int)(y / (scale * 0.48));
            
            if (old_x >= width) old_x = width - 1;
            if (old_y >= height) old_y = height - 1;

            unsigned char pixel = img[old_y * width + old_x];

            // --- 3. 輝度の正規化とガンマ補正 ---
            // 最小値を0、最大値を1.0にする（これでコントラストがハッキリする）
            double normalized = (double)(pixel - min_val) / (max_val - min_val);
            
            // 写真の場合は gamma = 1.6 くらいがディテールが残りやすい
            double corrected = pow(normalized, 1.6);
            
            // 背景が黒いターミナルの場合、文字の密度を反転させる
            // 密度が高い文字 ($ @) を明るい部分に割り当てる
            int index = (int)(corrected * (G_COUNT - 1));
            
            if (index < 0) index = 0;
            if (index >= G_COUNT) index = G_COUNT - 1;
            
            putchar(G_SCALE[(G_COUNT - 1) - index]); // 反転して出力
        }
        putchar('\n');
    }

    printf("\n--- END ---\n");
    stbi_image_free(img);
    printf("\n終了するには Enter を押してください...");
    getchar(); getchar();
    return 0;
}