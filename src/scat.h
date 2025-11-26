#pragma once
#include <vector>
#include <filesystem>
#include "chunk_help.h"

int scat_main(int argc, char** argv);
void print_tree(const std::vector<std::filesystem::path>& files);
