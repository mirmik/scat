#pragma once
#include "options.h"
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

std::string make_display_path(const std::filesystem::path &p);
void dump_file(const std::filesystem::path &p, bool first, const Options &opt);

// Безопасное получение размера файла; при ошибке возвращает 0.
std::uintmax_t get_file_size(const std::filesystem::path &p);

bool match_simple(const std::filesystem::path &p, const std::string &mask);

// HTML helpers
std::string html_escape(std::string_view src);
std::string wrap_cpp_as_html(std::string_view cpp_code,
                             std::string_view title = "C++ code");