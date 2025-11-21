#include "scat.h"
#include "doctest/doctest.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

TEST_CASE("scat walks example/ correctly")
{
    fs::path tmp = fs::temp_directory_path() / "scat_test";
    fs::remove_all(tmp);
    fs::create_directories(tmp / "a/b");
    fs::create_directories(tmp / "c");

    {
        std::ofstream(tmp / "1.txt") << "Hi";
        std::ofstream(tmp / "a/b/2.txt") << "Hello World!";
        std::ofstream(tmp / "c/3.txt") << "You find me!";
    }

    // перенаправляем stdout
    std::stringstream buffer;
    auto old = std::cout.rdbuf(buffer.rdbuf());

    const char* argv[] = {"scat", tmp.string().c_str(), "-r"};
    scat_main(3, (char**)argv);

    std::cout.rdbuf(old);

    std::string out = buffer.str();
    CHECK(out.find("=====") != std::string::npos);
    CHECK(out.find("Hello World!") != std::string::npos);
    CHECK(out.find("Hi") != std::string::npos);
    CHECK(out.find("You find me!") != std::string::npos);
}
