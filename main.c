#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "unpack.h"
#include "preview.h"
#include "paper.h"
#include "browse.h"
#include "cg.h"
#include "auto_updata.h"
#define Version 1.61
#define BUILD_VERIANT "db"

int main(void){
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);   // コンソールを UTF-8 に固定し、入出力を統一
    enable_vt();
    int rc = updata_main(Version);
    if(rc == -1){
        
        printf("アップデートに失敗しました。プログラムを閉じて再試行してください\n");
        fflush(stdout);
        system("pause");
    }
    if(rc == 2){
    /* アップデートスクリプトはすでにバックグラウンド／新しいウィンドウで実行中。本体はすぐに終了する */
    printf("アップデート中です。まもなく終了します...\n");
    fflush(stdout);
    return 0;
    }
                       // ANSIエスケープを有効化(画面クリア/反転)。でないと画面が文字化けする
    /* 検索とダウンロードは1つのモジュール(browse.c)に統合済み。メインメニューの入口は1つだけ */
    def menu[] = {
        {"1.リソース検索とダウンロード", browse_main, 0},
        {"2.アンパック", unpack_main, 0},
        {"3.Spineプレビューを開く(beta)", open_spine_preview, 0},
        {"4.USM/CGアンパック", unpack_usm, 0},
        {"5.終了", NULL, 0},
        {"END", NULL, 0}            /* 番兵は必ず最終行 */
    };

    while(1){
        int rc = pager_pick("メインメニュー", menu, 0);
        if(rc == -1)                /* Esc: メニュー表示を続ける */
            continue;
        if(rc == 4)                 /* "5.終了" は第4項(インデックスは0始まり) */
            break;
        /* 選択項目の func は pager 内で呼び出し済み。ここではそのままループ */
    }

    fflush(stdout);    // 先に結果をすべて出力してからキー待ち。リダイレクト時に pause と混ざらないようにする
    system("pause");   // exe をダブルクリックしたときウィンドウがすぐ閉じない。任意のキーで終了
    return 0;
}
