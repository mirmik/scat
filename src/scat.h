#pragma once
#include <filesystem>
#include <vector>

int scat_main(int argc, char **argv);
void print_tree(const std::vector<std::filesystem::path> &files);
