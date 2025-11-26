<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/symbols.cpp</title>
</head>
<body>
<pre><code>
#include &quot;symbols.h&quot;
#include &quot;doctest/doctest.h&quot;

#include &lt;string&gt;

TEST_CASE(&quot;CppSymbolFinder: finds simple class definition&quot;)
{
    std::string src = &quot;class Foo {\n&quot;
                      &quot;public:\n&quot;
                      &quot;    void bar();\n&quot;
                      &quot;};\n&quot;
                      &quot;\n&quot;
                      &quot;class Bar {};\n&quot;;

    CppSymbolFinder finder(src);
    Region r;

    CHECK(finder.find_class(&quot;Foo&quot;, r));
    CHECK(r.start_line == 0);
    CHECK(r.end_line == 3);

    // Другой класс тоже должен находиться
    CHECK(finder.find_class(&quot;Bar&quot;, r));
}

TEST_CASE(&quot;CppSymbolFinder: ignores forward declaration&quot;)
{
    std::string src = &quot;class Foo;\n&quot;
                      &quot;\n&quot;
                      &quot;class Foo {\n&quot;
                      &quot;public:\n&quot;
                      &quot;    void bar();\n&quot;
                      &quot;};\n&quot;;

    CppSymbolFinder finder(src);
    Region r;

    CHECK(finder.find_class(&quot;Foo&quot;, r));
    CHECK(r.start_line == 2);
    CHECK(r.end_line == 5);
}

TEST_CASE(&quot;CppSymbolFinder: finds method declarations inside class&quot;)
{
    std::string src = &quot;class Foo {\n&quot;
                      &quot;public:\n&quot;
                      &quot;    int bar(int x) const;\n&quot;
                      &quot;    void baz();\n&quot;
                      &quot;};\n&quot;;

    CppSymbolFinder finder(src);

    Region r1;
    CHECK(finder.find_method(&quot;Foo&quot;, &quot;bar&quot;, r1));
    CHECK(r1.start_line == 2);
    CHECK(r1.end_line == 2);

    Region r2;
    CHECK(finder.find_method(&quot;Foo&quot;, &quot;baz&quot;, r2));
    CHECK(r2.start_line == 3);
    CHECK(r2.end_line == 3);
}

TEST_CASE(&quot;CppSymbolFinder: finds inline method definition&quot;)
{
    std::string src = &quot;class Foo {\n&quot;
                      &quot;public:\n&quot;
                      &quot;    int bar(int x) const {\n&quot;
                      &quot;        return x + 1;\n&quot;
                      &quot;    }\n&quot;
                      &quot;};\n&quot;;

    CppSymbolFinder finder(src);
    Region r;

    CHECK(finder.find_method(&quot;Foo&quot;, &quot;bar&quot;, r));
    CHECK(r.start_line == 2);
    CHECK(r.end_line == 4);
}

TEST_CASE(&quot;CppSymbolFinder: method not found returns false&quot;)
{
    std::string src = &quot;class Foo {\n&quot;
                      &quot;public:\n&quot;
                      &quot;    void bar();\n&quot;
                      &quot;};\n&quot;;

    CppSymbolFinder finder(src);
    Region r;

    CHECK_FALSE(finder.find_method(&quot;Foo&quot;, &quot;baz&quot;, r));
}

TEST_CASE(&quot;CppSymbolFinder: complex file with multiple classes and comments&quot;)
{
    std::string src = &quot;#pragma once\n&quot;                                  // 0
                      &quot;\n&quot;                                              // 1
                      &quot;// forward decl\n&quot;                               // 2
                      &quot;class Forward;\n&quot;                                // 3
                      &quot;\n&quot;                                              // 4
                      &quot;/* comment with class Fake { */\n&quot;               // 5
                      &quot;\n&quot;                                              // 6
                      &quot;class First {\n&quot;                                 // 7
                      &quot;public:\n&quot;                                       // 8
                      &quot;    // method comment\n&quot;                         // 9
                      &quot;    int m1(int x) const;\n&quot;                      // 10
                      &quot;    std::string m2() const {\n&quot;                  // 11
                      &quot;        return \&quot;class Fake {\&quot;; // in string\n&quot; // 12
                      &quot;    }\n&quot;                                         // 13
                      &quot;};\n&quot;                                            // 14
                      &quot;\n&quot;                                              // 15
                      &quot;// Another class\n&quot;                              // 16
                      &quot;struct Second {\n&quot;                               // 17
                      &quot;    void go();\n&quot;                                // 18
                      &quot;};\n&quot;;                                           // 19

    CppSymbolFinder finder(src);
    Region r;

    // forward declaration не должен считаться определением
    CHECK_FALSE(finder.find_class(&quot;Forward&quot;, r));

    // Первый класс
    CHECK(finder.find_class(&quot;First&quot;, r));
    CHECK(r.start_line == 7);
    CHECK(r.end_line == 14);

    // Второй класс
    CHECK(finder.find_class(&quot;Second&quot;, r));
    CHECK(r.start_line == 17);
    CHECK(r.end_line == 19);

    // Методы первого класса
    Region m1;
    CHECK(finder.find_method(&quot;First&quot;, &quot;m1&quot;, m1));
    CHECK(m1.start_line == 10);
    CHECK(m1.end_line == 10);

    Region m2;
    CHECK(finder.find_method(&quot;First&quot;, &quot;m2&quot;, m2));
    CHECK(m2.start_line == 11);
    CHECK(m2.end_line == 13);

    // Метод во втором классе
    Region m_go;
    CHECK(finder.find_method(&quot;Second&quot;, &quot;go&quot;, m_go));
    CHECK(m_go.start_line == 18);
    CHECK(m_go.end_line == 18);

    CHECK_FALSE(finder.find_class(&quot;Fake&quot;, r));
}

TEST_CASE(&quot;CppSymbolFinder: handles extra whitespace and newlines&quot;)
{
    std::string src = &quot;  \n&quot;                 // 0
                      &quot;class   Weird  \n&quot;    // 1
                      &quot;  : public Base\n&quot;    // 2
                      &quot;{\n&quot;                  // 3
                      &quot;public:\n&quot;            // 4
                      &quot;    void  run ( );\n&quot; // 5
                      &quot;};\n&quot;;                // 6

    CppSymbolFinder finder(src);
    Region rc;

    CHECK(finder.find_class(&quot;Weird&quot;, rc));
    CHECK(rc.start_line == 1);
    CHECK(rc.end_line == 6);

    Region rm;
    CHECK(finder.find_method(&quot;Weird&quot;, &quot;run&quot;, rm));
    CHECK(rm.start_line == 5);
    CHECK(rm.end_line == 5);
}

</code></pre>
</body>
</html>
