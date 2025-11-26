#include "symbols.h"
#include "doctest/doctest.h"

#include <string>

TEST_CASE("CppSymbolFinder: finds simple class definition")
{
    std::string src = "class Foo {\n"
                      "public:\n"
                      "    void bar();\n"
                      "};\n"
                      "\n"
                      "class Bar {};\n";

    CppSymbolFinder finder(src);
    Region r;

    CHECK(finder.find_class("Foo", r));
    CHECK(r.start_line == 0);
    CHECK(r.end_line == 3);

    // Другой класс тоже должен находиться
    CHECK(finder.find_class("Bar", r));
}

TEST_CASE("CppSymbolFinder: ignores forward declaration")
{
    std::string src = "class Foo;\n"
                      "\n"
                      "class Foo {\n"
                      "public:\n"
                      "    void bar();\n"
                      "};\n";

    CppSymbolFinder finder(src);
    Region r;

    CHECK(finder.find_class("Foo", r));
    CHECK(r.start_line == 2);
    CHECK(r.end_line == 5);
}

TEST_CASE("CppSymbolFinder: finds method declarations inside class")
{
    std::string src = "class Foo {\n"
                      "public:\n"
                      "    int bar(int x) const;\n"
                      "    void baz();\n"
                      "};\n";

    CppSymbolFinder finder(src);

    Region r1;
    CHECK(finder.find_method("Foo", "bar", r1));
    CHECK(r1.start_line == 2);
    CHECK(r1.end_line == 2);

    Region r2;
    CHECK(finder.find_method("Foo", "baz", r2));
    CHECK(r2.start_line == 3);
    CHECK(r2.end_line == 3);
}

TEST_CASE("CppSymbolFinder: finds inline method definition")
{
    std::string src = "class Foo {\n"
                      "public:\n"
                      "    int bar(int x) const {\n"
                      "        return x + 1;\n"
                      "    }\n"
                      "};\n";

    CppSymbolFinder finder(src);
    Region r;

    CHECK(finder.find_method("Foo", "bar", r));
    CHECK(r.start_line == 2);
    CHECK(r.end_line == 4);
}

TEST_CASE("CppSymbolFinder: method not found returns false")
{
    std::string src = "class Foo {\n"
                      "public:\n"
                      "    void bar();\n"
                      "};\n";

    CppSymbolFinder finder(src);
    Region r;

    CHECK_FALSE(finder.find_method("Foo", "baz", r));
}
