#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>
#include "options.h"

std::string make_display_path(const std::filesystem::path& p);
void dump_file(const std::filesystem::path& p, bool first, const Options& opt);

// Безопасное получение размера файла; при ошибке возвращает 0.
std::uintmax_t get_file_size(const std::filesystem::path& p);

bool match_simple(const std::filesystem::path& p, const std::string& mask);
