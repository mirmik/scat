#pragma once
#include <filesystem>
#include <string>
#include <vector>

// expand_glob:
//   Поддерживает паттерны:
//     * "*" внутри уровня
//     * "**" — рекурсивный обход
//     * сложные пути вида "foo/*/bar/**/*.txt"
//
// Возвращает список regular files.
std::vector<std::filesystem::path>
expand_glob(const std::string& pattern);
