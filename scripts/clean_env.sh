#!/usr/bin/env bash
# Nettoyage de l'environnement de compilation pour MSYS2 / MinGW-UCRT64

echo "[INFO] Nettoyage des variables d'environnement C++/CMake..."

unset CXXFLAGS
unset CFLAGS
unset CPPFLAGS
unset CPLUS_INCLUDE_PATH
unset CPATH
unset INCLUDE
unset LIBRARY_PATH

echo "[INFO] Variables nettoyées."
echo "[INFO] Relancer la configuration proprement :"
echo "   rm -rf build"
echo "   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release"
echo "   cmake --build build"
