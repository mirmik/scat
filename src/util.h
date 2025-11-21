#pragma once
#include <filesystem>
#include <string>
#include <vector>

std::string make_display_path(const std::filesystem::path& p);
void dump_file(const std::filesystem::path& p, bool first);

bool match_simple(const std::filesystem::path& p, const std::string& mask);
