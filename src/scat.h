#pragma once
#include <filesystem>
#include <vector>
struct Options;

int scat_main(int argc, char **argv);
void print_tree(const std::vector<std::filesystem::path> &files);
int wrap_files_to_html(const std::vector<std::filesystem::path> &files,
                       const Options &opt);
