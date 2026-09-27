cmake_minimum_required(VERSION 3.10)
project(CGSS_Script)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
option(CGSS_STATIC "MinGWランタイム（libgcc/libstdc++）を静的リンク（リリースビルドでは有効を推奨）" ON)
add_executable(CGSS_Script
    main.c
    lookup_common.c lookup_3d.c lookup_spine.c lookup_song.c lookup_action.c
    lookup_chart.c lookup_card.c lookup_voice.c lookup_stage.c lookup_main.c
    download.c net.c acb.c util.c
    unpack.c unpack_fbx.c unpack_res.c
    sticker.c
    paper.c
    browse.c
    cg.c
    spine_convert.c preview.c texture_merge.cpp
    GBKswapUTF8.c sqlite3.c
    sticker.c
    auto_updata.c 
    miniz/miniz.h
    miniz/miniz.c)

target_link_libraries(CGSS_Script winhttp gdiplus pathcch)

target_compile_options(CGSS_Script PRIVATE -fexec-charset=UTF-8)

# 独立したリソースマニフェストの更新確認ツール（check_update.exe）は現在一時的に廃止
#add_executable(check_update
#    check_update.c
#    sqlite3.c)
#target_link_libraries(check_update winhttp)
#target_compile_options(check_update PRIVATE -fexec-charset=UTF-8)

if(CGSS_STATIC)
    target_link_options(CGSS_Script PRIVATE -static)
#    target_link_options(check_update PRIVATE -static)
endif()
# usmアンパックツール
add_executable(usm usm.c)
target_compile_options(usm PRIVATE -fexec-charset=UTF-8)
# ビルド後、spine_preview のWeb用リソースを exe と同じディレクトリへコピー（メインメニュー 4 のプレビュー用）
add_custom_command(TARGET CGSS_Script POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/spine_preview
        $<TARGET_FILE_DIR:CGSS_Script>/spine_preview
    COMMENT "Copying spine_preview assets next to CGSS_Script.exe")

# プロジェクトのルートディレクトリに ffmpeg.exe があれば exe の隣へ一緒にコピー（CG/USM のアンパック後に mp4 へ変換するため）
if(EXISTS ${CMAKE_SOURCE_DIR}/ffmpeg.exe)
    add_custom_command(TARGET CGSS_Script POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/ffmpeg.exe
            $<TARGET_FILE_DIR:CGSS_Script>/ffmpeg.exe
        COMMENT "Copying ffmpeg.exe next to CGSS_Script.exe")
endif()
