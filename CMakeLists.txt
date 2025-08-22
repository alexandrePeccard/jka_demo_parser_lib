cmake_minimum_required(VERSION 3.16)
project(jka_demo_parser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Trouver nlohmann_json ---
find_package(nlohmann_json REQUIRED)

# --- Fichiers sources ---
file(GLOB_RECURSE JKA_DEMO_PARSER_SOURCES src/*.cc)
if (NOT JKA_DEMO_PARSER_SOURCES)
    message(FATAL_ERROR "Aucun fichier source trouvé dans src/")
endif()

# --- Créer la bibliothèque principale ---
add_library(jka_demo_parser ${JKA_DEMO_PARSER_SOURCES})
target_include_directories(jka_demo_parser PUBLIC include)

# --- Exemple : dump_info ---
add_executable(jka_dump_info examples/dump_info.cpp)
target_link_libraries(jka_dump_info PRIVATE jka_demo_parser)

# --- Exemple : dump_json ---
add_executable(jka_dump_json examples/dump_json.cpp)
target_link_libraries(jka_dump_json PRIVATE jka_demo_parser nlohmann_json::nlohmann_json)
