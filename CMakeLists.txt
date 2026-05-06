cmake_minimum_required(VERSION 3.16)
project(BunkerRush CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Sources ──────────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/Game.cpp
    src/Menu.cpp
    src/Joueur.cpp
    src/Obstacle.cpp
)

add_executable(BunkerRush ${SOURCES})

target_include_directories(BunkerRush PRIVATE src)

# ── SFML ─────────────────────────────────────────────────────────────────────
find_package(SFML 2.5 COMPONENTS graphics audio window system REQUIRED)
target_link_libraries(BunkerRush
    sfml-graphics
    sfml-audio
    sfml-window
    sfml-system
)

# ── Copie des assets après build ─────────────────────────────────────────────
add_custom_command(TARGET BunkerRush POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:BunkerRush>/assets
    COMMENT "Copie des assets..."
)
