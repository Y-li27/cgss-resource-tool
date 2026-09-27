# CGSS リソースツール（CGSS Resource Tool）

CGSS（アイドルマスター シンデレラガールズ スターライトステージ）のリソース検索・ダウンロード・アンパック一体型ツール
の、日本語翻訳版。
フォーク元は以下の通り。
https://github.com/BA-Momoi/cgss-resource-tool

C で記述（MinGW + CMake）、静的リンク。Windows 10/11 では展開するだけですぐ使え、ランタイムのインストールは不要です
> [!IMPORTANT]
> アンパック関連の機能を使う場合は、AssetStudio を同じディレクトリに置き、NET 7 をインストールしてください
> https://builds.dotnet.microsoft.com/dotnet/WindowsDesktop/7.0.20/windowsdesktop-runtime-7.0.20-win-x64.exe
> ゲーム関連のリソースおよびコンテンツの著作権は BANDAI NAMCO Entertainment Inc. に帰属します。
> 本ツールは学習・交流のみに使用してください。商用利用はしないでください。ダウンロード・アンパックした内容は 24 時間以内に削除してください。

---

## 機能一覧

メインメニュー：

```
1. リソース検索とダウンロード
2. アンパック
3. Spine プレビューを開く(beta)
4. USM/CGアンパック
```

### 1. リソース検索とダウンロード
- プリセットの CG / 2DMV / 3Dモデル / 2D SDキャラ（Spineモデル） / 楽曲 / 譜面 / カードイラスト画像 / 動的カードイラスト（Spineアニメーション）
  キャラのボイス＆テキスト / 3Dステージ / 3DMVキャラモーション / ゲームイラストステッカー / BGM のダウンロード
- カスタム検索に対応
- 一部はダウンロード完了後にアンパックするかを選べ、手間を減らせる

### 2. アンパック

- モデルを FBX にアンパック（AssetStudio.CLI を呼び出し。CLI で書き出したボディのテクスチャは自動参照できない）
- カードイラスト / 背景 / Live2D / 3Dフォト / Spine を PNG にアンパック
- Spine `.skel` を自動で JSON に変換し、次の 2 つを同時に生成：
  - `*.json`：3.6 形式（ブラウザプレビュー用）
  - `*_v38.json`：3.8.75 形式（Spine エディタ用）
- RGB メインテクスチャ + A8 アルファチャンネルを自動合成して `*_merged.png` を作り、対応する atlas も生成
- ACB 楽曲の抽出と HCA デコード（acb2wavs）

### 3. Spine プレビュー（beta）

`CGSS_DOWN` 内で Spine（Live2D）リソースを持つキャラを走査し、欠けている JSON を自動で追加変換する。
デフォルトのブラウザで `spine_preview/preview.html` を開けばアニメーションを再生できる。

- `.skel` ファイルを直接選択して自動で JSON に変換（atlas とテクスチャも同時に選ぶ必要がある）
- レイヤー順は自動で並べる：bg → eff2 → chara → eff1 → fg
- ブレンドモードは Spine の規則に従って再現（normal / additive / multiply / screen）
- デフォルトは WebGL レンダリングで、三角形の継ぎ目がない。使えない場合は canvas 2D + 2 倍スーパーサンプリングへ自動フォールバック
- 左右ミラー（flip）に対応。新カードは s、旧カードは n のスケルトンを、読み込む前に自動で知らせる
- ページに「MP4を書き出し」ボタンがある：WebCodecs のハードウェアエンコード H.264、30fps。
  背景のバウンディングボックスを出力サイズにする（新しい版の Chrome / Edge が必要）

  ### 4.USM/CGアンパック

- カスタム USM ファイルのアンパックに対応（ただしデフォルトの鍵は草菇の鍵。必要な場合は自分で usm.c の鍵を変更する）
- 

---

## Spine エディタ（3.8.75）の使い方

- Spine 3.8.75 は 3.6 の `.skel` / `.json` を直接開けない（データバージョンが一致している必要がある）
- 本ツールはアンパック / プレビュー時に、追加で `*_v38.json`（データバージョン 3.8.75）を生成する
  これらのカードイラストアニメには IK / Transform / Path コンストレイントがなく、3.6 → 3.8 の差はバージョン番号だけ。
  Spine 3.8 ランタイムで実機確認済みで、正常に読み込んで再生できる
- 3.8.75 エディタで開くもの：
  `*_v38.json` + `*_v38.atlas` + `*_merged.png`
  （merged は RGB メインテクスチャと A8 アルファチャンネルを合成した 1 枚のテクスチャ。エディタは 2 枚テクスチャに非対応）

---

## ディレクトリ構成

```
CGSS/
├── main.c                     メインメニュー入口（1.リソース検索とダウンロード 2.アンパック 3.Spineプレビュー 4.USM/CGアンパック）
├── browse.c / .h              リソース検索+ダウンロード統合モジュール（自由検索 / BGM / 楽曲 / カード /
├── miniz                       zip アンパック用のサードパーティライブラリ
│                               譜面 / ステージ / モーション / 3Dモデル / Spine / ステッカー / CGムービー）
├── cg.c / .h                  USM/CG アンパック（カスタムファイル/ディレクトリのアンパック、ダウンロード済み CG のアンパック、
│                               動画+対応する音声を自動合成して mp4）
├── check_updata.c              一時的に廃止
├── sticker.c / .h              browse を拡張。テクスチャのダウンロード後に spine プロジェクトファイルと png 画像の書き出しを呼び出す
├── paper.c / .h               ページ移動メニュー部品（pager：単一選択/複数選択、方向キーでページ移動、全画面リスト）
├── auto_updata.c / .h          バージョン番号を検出し、github 上のバージョン番号と比較。新バージョンがあれば自動アップデートを選べる
├── usm.c                      独立した USM アンパックのコマンドラインツール（usm.exe）
├── lookup_*.c / lookup_table.h  旧検索モジュール（browse に統合済み。参考として残す）
├── download.c / .h            旧ダウンロードモジュール（browse に統合済み。参考として残す）
├── net.c / .h                 ネットワークダウンロード（CDN を拡張子で分類：unity3d/acb/usm/bdb）+ LZ4 展開
├── acb.c / .h                 ACB 楽曲の抽出と HCA デコード（acb2wavs）
├── unpack.c / .h              アンパックメニュー + 共通アンパック処理
├── unpack_fbx.c               モデルを FBX にアンパック
├── unpack_res.c               キャラリソースのアンパック
├── spine_convert.c / .h       Spine .skel -> JSON 変換（CGSS ビッグエンディアン形式）
├── texture_merge.cpp / .h     RGB + A8 テクスチャ合成
├── preview.c / .h             Spine ブラウザプレビュー
├── util.c / .h                共通ユーティリティ（ディレクトリ作成 / エンコーディング変換 / 複数選択の解析など）
├── GBKswapUTF8.c / .h         エンコーディング変換
├── data.h                     データ構造の定義（カードイラスト/キャラなど）
├── PNG                        記録.md の画像フォルダ
├── sqlite3.c / sqlite3.h / sqlite3ext.h   SQLite ライブラリ
├── spine_preview/             Spine プレビュー用のウェブページリソース
├── cgss_apply_textures.py      Blender テクスチャスクリプト
├── cgss_anim_to_shapekeys.py   Blender シェイプキースクリプト
├── CMakeLists.txt              ビルド設定
├── ffmpeg.exe                  動画変換（サードパーティ。リポジトリには入れず、リリース包に同梱）
├── master.mdb / manifest_10133800.db   ゲームデータベース（リポジトリ外。自分で用意）
├── 記録.md                     メモに近いもの
└── README.md                   本文書
```

---

## コンパイル

MinGW-w64 と CMake が必要です：

```bash
cmake -S . -B build_static -DCMAKE_BUILD_TYPE=Release -DCGSS_STATIC=ON
cmake --build build_static --target all -j 8
```

成果物：

- `build_static/CGSS_Script.exe`（メインプログラム）
- `build_static/usm.exe`（usm アンパックプログラム）

デフォルトは静的リンク：libgcc / libstdc++ は exe に組み込み済み。成果物は Windows のシステム DLL だけに依存し、
MinGW のインストールも、MinGW DLL の同梱も不要です。

動的リンクが必要な場合（容量は少し小さいが、libgcc / libstdc++ の 2 つの DLL を一緒に置く必要がある）：

```bash
cmake -S . -B build -DCGSS_STATIC=OFF
```

## 依存関係

- データベース：ゲームクライアントから抽出した `master.mdb`（メイン DB）と `manifest_10133800.db`（リソースマニフェスト）。
- あるいは `manifest_10133900.db` を利用する。
- NET 7 環境（任意）
- Github に問題なくアクセスできるネットワーク環境
- サードパーティライブラリ: miniz
- USM 以外のアンパック：RazTools の改造版 AssetStudio（.NET 7 プログラム。.NET 7 Desktop Runtime のインストールが必要）
- ボイスのデコード：deretore-toolkit の `acb2wavs.exe` と、同じディレクトリの DLL
- Blender スクリプト（任意）：
  - `cgss_apply_textures.py`：Blender 上の FBX に自動でテクスチャを貼り、粗さ = 1
  - `cgss_anim_to_shapekeys.py`：ボーンの表情モーションをシェイプキーにベイク
- `ffmpeg.exe` で MV の音声と映像を合成

---

## よくある質問

- アンパックで「AssetStudio.CLI の起動に失敗」と出る：.NET 7 Desktop Runtime をインストール
- アップデート失敗：新しいフォルダを作る権限があること、Github に正常にアクセスできることを確認する。それでもダメなら、手動で Release をダウンロードして置き換えてください。余裕があれば issue も出してもらえると助かります
- ボイスのデコードで出力がない：`acb2wavs.exe` と同じディレクトリの DLL がウイルス対策ソフトに削除されていないか確認
- データベースが見つからない：`master.mdb`、`manifest_*.db` を exe と同じディレクトリに置けばよい。
  `_nodb` 版は自分でデータベースを用意するか、`check_update.exe` を一度実行してマニフェスト DB を取得する
- CLI で書き出した FBX のボディにテクスチャがない：テクスチャ付きの body_FBX は GUI で書き出す必要がある
  （アンパックメニュー内に詳しい手順あり）
- アンパックを選んでも必要なファイルが出てこない：exe と同じディレクトリに AssetStudio があることを確認する

---

## 謝辞

- リソースマニフェスト / リソースサーバー構成は [mishiro](https://github.com/toyobayashi/mishiro) を参考
- モデルのアンパックには AssetStudio を使用
https://github.com/RazTools/Studio
- 音声デコードには deretore-toolkit（acb2wavs）を使用
- Spine プレビューには Spine Runtimes（spine-core / spine-canvas / spine-webgl）を使用
