# JKA Demo Parser Library

![C++](https://img.shields.io/badge/language-C++17-blue.svg)
![CMake](https://img.shields.io/badge/build-CMake-green.svg)
![License](https://img.shields.io/badge/license-GPLv2-orange.svg)

## 📖 Description

`jka_demo_parser_lib` est une bibliothèque C++ moderne permettant de **parser et analyser les fichiers de démo DM_26** utilisés par *Jedi Knight: Jedi Academy*.  
Elle reconstruit fidèlement la structure interne des démos (messages réseau, snapshots, états des entités, commandes serveur, etc.) et propose une API claire pour exporter les données vers des formats modernes tels que **JSON**.

Cette bibliothèque s’appuie sur la logique originale d’**idTech3 / ioquake3 / OpenJK**, mais modernisée en C++17 :
- Gestion mémoire avec `std::unique_ptr` et `std::vector`
- API orientée objets
- Système de compression **Huffman adaptatif** (mode streaming pris en charge)
- Export JSON via [nlohmann/json](https://github.com/nlohmann/json)

---

## 🚀 Fonctionnalités

- Lecture complète des démos **DM_26**
- Support des instructions :  
  - `ServerCommand`  
  - `MapChange`  
  - `Snapshot` (+ `PlayerStateInstr`, `EntityStateInstr`)  
- Décompression réseau avec **Huffman** (mode streaming ou reset par message)
- Conversion vers JSON fidèle à la structure d’origine
- Outils CLI inclus :  
  - `jka_dump_info` : affiche des infos basiques sur une démo  
  - `jka_dump_json` : exporte la démo en JSON  

---

## 📦 Dépendances

- [CMake >= 3.16](https://cmake.org/)  
- C++17 compiler (GCC, Clang, MSVC, MinGW/MSYS2)  
- [nlohmann/json](https://github.com/nlohmann/json)  

Sous **MSYS2 / MinGW64** :  

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-nlohmann-json
