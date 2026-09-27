// unpack_res.c: キャラリソースのアンパック（カードイラスト/背景/カードイラストSpinaアニメ/3Dフォト/spine -> png/データ）
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "util.h"
#include "unpack.h"
#include "spine_convert.h"
#include "texture_merge.h"

#define MAX_RES_ITEMS 1024

typedef struct {
    wchar_t path[1100];
    wchar_t dir[1100];
    char name[256];
    char folder_name[256];
    char sub[64];
    int done;
    int spine_sub;   /* カードイラストSpinaアニメ/live2d: 独立した spine サブフォルダへアンパック */
} ResUnpackItem;

static void scan_res_dir(const wchar_t *chara_dir, const wchar_t *sub, const char *sub_u8,
                         ResUnpackItem *items, int *n, const char *folder_name, int spine_sub){
    wchar_t pat[1300];
    swprintf(pat, 1300, L"%ls\\%ls\\*.unity3d", chara_dir, sub);
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(pat, &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (*n >= MAX_RES_ITEMS) break;
        swprintf(items[*n].dir, 1100, L"%ls\\%ls", chara_dir, sub);
        swprintf(items[*n].path, 1100, L"%ls\\%ls\\%ls", chara_dir, sub, fd.cFileName);
        wide_to_utf8(fd.cFileName, items[*n].name, sizeof items[*n].name);
        snprintf(items[*n].folder_name, sizeof items[*n].folder_name, "%s", folder_name);
        snprintf(items[*n].sub, sizeof items[*n].sub, "%s", sub_u8);
        items[*n].spine_sub = spine_sub;
        wchar_t marker[1300];
        swprintf(marker, 1300, L"%ls\\%ls\\%ls.done", chara_dir, sub, fd.cFileName);
        items[*n].done = (GetFileAttributesW(marker) != INVALID_FILE_ATTRIBUTES);
        (*n)++;
    } while (FindNextFileW(h, &fd));
    FindClose(h);
}

/* 共有スケルトンに同梱のテンプレートスキン（卯月/杏の例）をサブフォルダへ移し、カード自身のSDキャラと混ざらないようにする */
static void move_shared_template_samples(const wchar_t *dir){
    /* 移動するファイル（共有スケルトン SPSprachen_N/s のテクスチャとアトラス。スケルトン本体は残す） */
    static const wchar_t *samples[] = {
        L"SPSprachen_N.png", L"SPSprachen_N.atlas", L"SPSprachen_N.atlas.asset",
        L"SPSprachen_N.atlas.atlas", L"SPSprachen_N_Atlas.json", L"SPSprachen_N_SkeletonData.json",
        L"SPSprachen_N.skel", L"SPSprachen_N.skel.asset", L"SPSprachen_N.json", L"SPSprachen_N_v38.json",
        L"SPSprachen_s.png", L"SPSprachen_s.atlas", L"SPSprachen_s.atlas.asset",
        L"SPSprachen_s.atlas.atlas", L"SPSprachen_s_Atlas.json", L"SPSprachen_s_SkeletonData.json",
    };
    wchar_t subdir[1300];
    swprintf(subdir, 1300, L"%ls\\テンプレート例(卯月杏)", dir);
    mkdirs(subdir);
    int moved = 0;
    for (int i = 0; i < (int)(sizeof samples / sizeof samples[0]); i++){
        wchar_t src[1300], dst[1300];
        swprintf(src, 1300, L"%ls\\%ls", dir, samples[i]);
        if (GetFileAttributesW(src) == INVALID_FILE_ATTRIBUTES) continue;
        swprintf(dst, 1300, L"%ls\\%ls", subdir, samples[i]);
        if (MoveFileW(src, dst)) moved++;
    }
    /* N スケルトンは旧カードのSDキャラ用（旧カードで s を使うと「大頭」になる）。spine ルートにも1部残して使いやすくする */
    static const wchar_t *keepN[] = {
        L"SPSprachen_N.skel", L"SPSprachen_N.skel.asset",
        L"SPSprachen_N.json", L"SPSprachen_N_v38.json"
    };
    for (int i = 0; i < (int)(sizeof keepN / sizeof keepN[0]); i++){
        wchar_t src[1300], dst[1300];
        swprintf(src, 1300, L"%ls\\%ls", subdir, keepN[i]);
        swprintf(dst, 1300, L"%ls\\%ls", dir, keepN[i]);
        if (GetFileAttributesW(src) != INVALID_FILE_ATTRIBUTES &&
            GetFileAttributesW(dst) == INVALID_FILE_ATTRIBUTES)
            CopyFileW(src, dst, FALSE);
    }
    if (moved > 0)
        printf("  共有スケルトンのテンプレート例(卯月/杏)を テンプレート例(卯月杏)\\ サブフォルダへ移しました\n");
}

/* キャラリソースを1つアンパックし、png/データファイルを元のディレクトリへ書き出す */

static int extract_res_one(const ResUnpackItem *it, int idx){
    wchar_t exedir[1024], outdir[1200];
    GetModuleFileNameW(NULL, exedir, 1024);
    wchar_t *p = wcsrchr(exedir, L'\\');
    if (p) *p = 0;
    swprintf(outdir, 1200, L"%ls\\AssetStudio_out\\r%03d", exedir, idx);
    wipe_dir(outdir);
    mkdirs(outdir);

    wchar_t exe[1200], cmd[3000];
    find_assetstudio(exe, 1200);
    if (!exe[0]){
        printf("AssetStudio.CLI.exe が見つかりません。AssetStudio フォルダをプログラムと同じディレクトリに置いてください\n");
        return 0;
    }
    swprintf(cmd, 3000, L"\"%ls\" \"%ls\" \"%ls\" --game Normal", exe, it->path, outdir);
    printf("アンパック %s\\%s ...\n", it->sub, it->name);
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof si); si.cb = sizeof si;
    memset(&pi, 0, sizeof pi);
    if (!CreateProcessW(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)){
        printf("AssetStudio.CLI の起動に失敗 err=%lu\n", (unsigned long)GetLastError());
        return 0;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    int n = 0;
    /* 書き出し先: spine パックは探しやすいよう spine サブフォルダに分ける */
    wchar_t destdir[1300];
    wcscpy(destdir, it->dir);
    if (it->spine_sub){
        swprintf(destdir, 1300, L"%ls\\spine", it->dir);
        mkdirs(destdir);
    }
    n += copy_dir(outdir, L"Texture2D", destdir, L"*.png");
    n += copy_dir(outdir, L"Texture2D", destdir, L"*.tga");
    n += copy_dir(outdir, L"Sprite", destdir, L"*.png");
    n += copy_dir(outdir, L"TextAsset", destdir, L"*");
    if (!it->spine_sub && strcmp(it->sub, "spine") != 0)
        n += copy_dir(outdir, L"MonoBehaviour", destdir, L"*.json");
    n += copy_dir(outdir, L"AudioClip", destdir, L"*");

    /* カードイラストSpinaアニメ/live2d の .skel を .json に自動変換し、ブラウザプレビューしやすくする */
    int cn = convert_skels_in_dir(destdir);
    if (cn > 0)
        printf("  skel %d 個を json に変換（3.6 + 3.8.75、メインメニュー4でプレビュー可）\n", cn);
    /* atlas の書き出し名は *.atlas.asset。Spine エディタで直接開けるよう *.atlas もコピーする */
    {
        wchar_t apat[1300];
        swprintf(apat, 1300, L"%ls\\*.atlas.asset", destdir);
        WIN32_FIND_DATAW afd;
        HANDLE ah = FindFirstFileW(apat, &afd);
        if (ah != INVALID_HANDLE_VALUE){
            do {
                if (afd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
                wchar_t asrc[1300], adst[1300];
                swprintf(asrc, 1300, L"%ls\\%ls", destdir, afd.cFileName);
                wcscpy(adst, asrc);
                /* 末尾の .asset を除去: SPC301346.atlas.asset -> SPC301346.atlas */
                size_t alen = wcslen(adst);
                if (alen > 6 && _wcsicmp(adst + alen - 6, L".asset") == 0)
                    adst[alen - 6] = 0;
                if (GetFileAttributesW(adst) == INVALID_FILE_ATTRIBUTES)
                    CopyFileW(asrc, adst, FALSE);
            } while (FindNextFileW(ah, &afd));
            FindClose(ah);
        }
        /* 旧版が誤生成した *.atlas.atlas を掃除（中身は *.atlas と同じ） */
        {
            wchar_t apat2[1300];
            swprintf(apat2, 1300, L"%ls\\*.atlas.atlas", destdir);
            WIN32_FIND_DATAW afd2;
            HANDLE ah2 = FindFirstFileW(apat2, &afd2);
            if (ah2 != INVALID_HANDLE_VALUE){
                do {
                    if (afd2.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
                    wchar_t adup[1300], aclean[1300];
                    swprintf(adup, 1300, L"%ls\\%ls", destdir, afd2.cFileName);
                    wcscpy(aclean, adup);
                    size_t alen2 = wcslen(aclean);
                    if (alen2 > 6 && _wcsicmp(aclean + alen2 - 6, L".atlas") == 0)
                        aclean[alen2 - 6] = 0;   /* X.atlas.atlas -> X.atlas */
                    if (GetFileAttributesW(aclean) != INVALID_FILE_ATTRIBUTES)
                        DeleteFileW(adup);
                } while (FindNextFileW(ah2, &afd2));
                FindClose(ah2);
            }
        }
    }
    /* RGB + A8 テクスチャを合成し、それを参照する v38 atlas を生成（Spine 3.8.75 エディタ用） */
    if (it->spine_sub){
        int mn = merge_a8_textures_in_dir(destdir);
        if (mn > 0)
            printf("  テクスチャ %d 枚を合成（3.8.75 エディタ用）\n", mn);
    }
    /* 共有スケルトン同梱のテンプレートスキン（卯月/杏の例）をサブフォルダへ移す */
    if (!it->spine_sub && strcmp(it->sub, "spine") == 0)
        move_shared_template_samples(destdir);

    wchar_t wname[256], marker[1300];
    utf8_to_wide(it->name, wname, 256);
    swprintf(marker, 1300, L"%ls\\%ls.done", it->dir, wname);
    FILE *mf = _wfopen(marker, L"wb");
    if (mf){ fputs("done", mf); fclose(mf); }

    if (n == 0) printf("  コピーできるファイルがありません（パックに画像/データがない可能性）\n");
    printf("  完了。%d 個のファイルを書き出し\n", n);
    return n;
}


int unpack_resources_main(void){
    wchar_t wroot[1024];
    GetModuleFileNameW(NULL, wroot, 1024);
    wchar_t *p = wcsrchr(wroot, L'\\');
    if (p) *p = 0;
    wcscat(wroot, L"\\CGSS_DOWN");

    ResUnpackItem *items = (ResUnpackItem*)malloc(sizeof(ResUnpackItem) * MAX_RES_ITEMS);
    if (!items){ fprintf(stderr, "メモリ不足\n"); return 1; }
    int n = 0;

    /* キャラディレクトリ配下: カードイラスト/背景/カードイラストSpinaアニメ（旧名 live2d 互換）/3Dフォト/spine */
    const wchar_t *subs[6] = { L"カードイラスト", L"背景", L"カードイラストSpinaアニメ", L"live2d", L"3Dフォト", L"spine" };
    const char *subs_u8[6] = { "カードイラスト", "背景", "カードイラストSpinaアニメ", "live2d(旧)", "3Dフォト", "spine" };

    wchar_t pat[1300];
    swprintf(pat, 1300, L"%ls\\*", wroot);
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(pat, &fd);
    if (h != INVALID_HANDLE_VALUE){
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
            if (fd.cFileName[0] == L'.') continue;
            char chara_name[256];
            wide_to_utf8(fd.cFileName, chara_name, sizeof chara_name);
            wchar_t chara_dir[1300];
            swprintf(chara_dir, 1300, L"%ls\\%ls", wroot, fd.cFileName);
            for (int s = 0; s < 6; s++)
                scan_res_dir(chara_dir, subs[s], subs_u8[s], items, &n, chara_name, (s == 2 || s == 3));
        } while (FindNextFileW(h, &fd) && n < MAX_RES_ITEMS);
        FindClose(h);
    }

    if (n == 0){
        printf("CGSS_DOWN にカードイラスト/背景/カードイラストSpinaアニメ/3Dフォト/spine リソースがありません\n");
        free(items);
        return 1;
    }

    int ndone = 0;
    for (int i = 0; i < n; i++){
        printf("[%d] %s : %s\\%s %s\n", i + 1, items[i].folder_name, items[i].sub, items[i].name,
               items[i].done ? "[アンパック済]" : "[未アンパック]");
        if (items[i].done) ndone++;
    }
    printf("リソースパック %d 個、アンパック済 %d 個（a=未処理をすべて、番号で強制再アンパック、0=戻る）: ", n, ndone);
    char buf[128];
    if (fgets(buf, sizeof buf, stdin) == NULL){ free(items); return 1; }
    int *sel = (int*)malloc(sizeof(int) * n);
    if (!sel){ free(items); return 1; }
    int nsel = parse_multi(buf, sel, n);
    if (nsel < 0){
        nsel = 0;
        for (int i = 0; i < n; i++)
            if (!items[i].done) sel[nsel++] = i + 1;
        if (nsel == 0){ printf("すべてアンパック済みです。処理は不要です\n"); free(sel); free(items); return 0; }
    }
    if (nsel == 0){ free(sel); free(items); return 1; }

    for (int s = 0; s < nsel; s++){
        extract_res_one(&items[sel[s] - 1], s);
    }
    printf("すべて完了\n");
    free(sel);
    free(items);
    return 0;
}
