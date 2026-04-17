cmake_minimum_required(VERSION 3.16)
project(ALTERDUNE CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Sources communes ──────────────────────────────────────────────────────────
set(COMMON_SRC
    joueur.cpp
    monstre.cpp
    monstreNormal.cpp
    miniboss.cpp
    boss.cpp
    item.cpp
    actionAct.cpp
    bestiaire.cpp
    chargeurCSV.cpp
)

# ── Version console ───────────────────────────────────────────────────────────
add_executable(alterdune main.cpp jeu.cpp ${COMMON_SRC})

# ── SFML (bundled dans lib/cmake/SFML) ───────────────────────────────────────
set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}" ${CMAKE_PREFIX_PATH})
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

add_executable(alterdune_sfml main_sfml.cpp jeu_sfml.cpp ${COMMON_SRC})
target_include_directories(alterdune_sfml PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(alterdune_sfml PRIVATE SFML::Graphics SFML::Window SFML::System)

# ── macOS : linkage statique + frameworks ─────────────────────────────────────
if(APPLE)
    target_compile_definitions(alterdune_sfml PRIVATE SFML_STATIC)
    target_link_libraries(alterdune_sfml PRIVATE
        "-framework OpenGL"
        "-framework AppKit"
        "-framework Foundation"
        "-framework IOKit"
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework CoreVideo"
        "-framework Carbon"
    )
endif()

# ── Windows (MinGW) : copier les DLL à côté de l'exe ─────────────────────────
if(WIN32)
    set(SFML_DLLS
        sfml-graphics-3.dll
        sfml-window-3.dll
        sfml-system-3.dll
    )
    foreach(DLL ${SFML_DLLS})
        add_custom_command(TARGET alterdune_sfml POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_SOURCE_DIR}/bin/${DLL}"
                "$<TARGET_FILE_DIR:alterdune_sfml>/${DLL}"
        )
    endforeach()
endif()
