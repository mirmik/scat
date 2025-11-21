#pragma once
#include <vector>
#include <filesystem>

int scat_main(int argc, char** argv);
void print_chunk_help();
void print_tree(const std::vector<std::filesystem::path>& files);
