<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/symbols.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;symbols.h&quot;<br>
#include &quot;doctest/doctest.h&quot;<br>
<br>
#include &lt;string&gt;<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: finds simple class definition&quot;)<br>
{<br>
    std::string src = &quot;class Foo {\n&quot;<br>
                      &quot;public:\n&quot;<br>
                      &quot;    void bar();\n&quot;<br>
                      &quot;};\n&quot;<br>
                      &quot;\n&quot;<br>
                      &quot;class Bar {};\n&quot;;<br>
<br>
    CppSymbolFinder finder(src);<br>
    Region r;<br>
<br>
    CHECK(finder.find_class(&quot;Foo&quot;, r));<br>
    CHECK(r.start_line == 0);<br>
    CHECK(r.end_line == 3);<br>
<br>
    // Другой класс тоже должен находиться<br>
    CHECK(finder.find_class(&quot;Bar&quot;, r));<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: ignores forward declaration&quot;)<br>
{<br>
    std::string src = &quot;class Foo;\n&quot;<br>
                      &quot;\n&quot;<br>
                      &quot;class Foo {\n&quot;<br>
                      &quot;public:\n&quot;<br>
                      &quot;    void bar();\n&quot;<br>
                      &quot;};\n&quot;;<br>
<br>
    CppSymbolFinder finder(src);<br>
    Region r;<br>
<br>
    CHECK(finder.find_class(&quot;Foo&quot;, r));<br>
    CHECK(r.start_line == 2);<br>
    CHECK(r.end_line == 5);<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: finds method declarations inside class&quot;)<br>
{<br>
    std::string src = &quot;class Foo {\n&quot;<br>
                      &quot;public:\n&quot;<br>
                      &quot;    int bar(int x) const;\n&quot;<br>
                      &quot;    void baz();\n&quot;<br>
                      &quot;};\n&quot;;<br>
<br>
    CppSymbolFinder finder(src);<br>
<br>
    Region r1;<br>
    CHECK(finder.find_method(&quot;Foo&quot;, &quot;bar&quot;, r1));<br>
    CHECK(r1.start_line == 2);<br>
    CHECK(r1.end_line == 2);<br>
<br>
    Region r2;<br>
    CHECK(finder.find_method(&quot;Foo&quot;, &quot;baz&quot;, r2));<br>
    CHECK(r2.start_line == 3);<br>
    CHECK(r2.end_line == 3);<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: finds inline method definition&quot;)<br>
{<br>
    std::string src = &quot;class Foo {\n&quot;<br>
                      &quot;public:\n&quot;<br>
                      &quot;    int bar(int x) const {\n&quot;<br>
                      &quot;        return x + 1;\n&quot;<br>
                      &quot;    }\n&quot;<br>
                      &quot;};\n&quot;;<br>
<br>
    CppSymbolFinder finder(src);<br>
    Region r;<br>
<br>
    CHECK(finder.find_method(&quot;Foo&quot;, &quot;bar&quot;, r));<br>
    CHECK(r.start_line == 2);<br>
    CHECK(r.end_line == 4);<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: method not found returns false&quot;)<br>
{<br>
    std::string src = &quot;class Foo {\n&quot;<br>
                      &quot;public:\n&quot;<br>
                      &quot;    void bar();\n&quot;<br>
                      &quot;};\n&quot;;<br>
<br>
    CppSymbolFinder finder(src);<br>
    Region r;<br>
<br>
    CHECK_FALSE(finder.find_method(&quot;Foo&quot;, &quot;baz&quot;, r));<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: complex file with multiple classes and comments&quot;)<br>
{<br>
    std::string src = &quot;#pragma once\n&quot;                                  // 0<br>
                      &quot;\n&quot;                                              // 1<br>
                      &quot;// forward decl\n&quot;                               // 2<br>
                      &quot;class Forward;\n&quot;                                // 3<br>
                      &quot;\n&quot;                                              // 4<br>
                      &quot;/* comment with class Fake { */\n&quot;               // 5<br>
                      &quot;\n&quot;                                              // 6<br>
                      &quot;class First {\n&quot;                                 // 7<br>
                      &quot;public:\n&quot;                                       // 8<br>
                      &quot;    // method comment\n&quot;                         // 9<br>
                      &quot;    int m1(int x) const;\n&quot;                      // 10<br>
                      &quot;    std::string m2() const {\n&quot;                  // 11<br>
                      &quot;        return \&quot;class Fake {\&quot;; // in string\n&quot; // 12<br>
                      &quot;    }\n&quot;                                         // 13<br>
                      &quot;};\n&quot;                                            // 14<br>
                      &quot;\n&quot;                                              // 15<br>
                      &quot;// Another class\n&quot;                              // 16<br>
                      &quot;struct Second {\n&quot;                               // 17<br>
                      &quot;    void go();\n&quot;                                // 18<br>
                      &quot;};\n&quot;;                                           // 19<br>
<br>
    CppSymbolFinder finder(src);<br>
    Region r;<br>
<br>
    // forward declaration не должен считаться определением<br>
    CHECK_FALSE(finder.find_class(&quot;Forward&quot;, r));<br>
<br>
    // Первый класс<br>
    CHECK(finder.find_class(&quot;First&quot;, r));<br>
    CHECK(r.start_line == 7);<br>
    CHECK(r.end_line == 14);<br>
<br>
    // Второй класс<br>
    CHECK(finder.find_class(&quot;Second&quot;, r));<br>
    CHECK(r.start_line == 17);<br>
    CHECK(r.end_line == 19);<br>
<br>
    // Методы первого класса<br>
    Region m1;<br>
    CHECK(finder.find_method(&quot;First&quot;, &quot;m1&quot;, m1));<br>
    CHECK(m1.start_line == 10);<br>
    CHECK(m1.end_line == 10);<br>
<br>
    Region m2;<br>
    CHECK(finder.find_method(&quot;First&quot;, &quot;m2&quot;, m2));<br>
    CHECK(m2.start_line == 11);<br>
    CHECK(m2.end_line == 13);<br>
<br>
    // Метод во втором классе<br>
    Region m_go;<br>
    CHECK(finder.find_method(&quot;Second&quot;, &quot;go&quot;, m_go));<br>
    CHECK(m_go.start_line == 18);<br>
    CHECK(m_go.end_line == 18);<br>
<br>
    CHECK_FALSE(finder.find_class(&quot;Fake&quot;, r));<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: handles extra whitespace and newlines&quot;)<br>
{<br>
    std::string src = &quot;  \n&quot;                 // 0<br>
                      &quot;class   Weird  \n&quot;    // 1<br>
                      &quot;  : public Base\n&quot;    // 2<br>
                      &quot;{\n&quot;                  // 3<br>
                      &quot;public:\n&quot;            // 4<br>
                      &quot;    void  run ( );\n&quot; // 5<br>
                      &quot;};\n&quot;;                // 6<br>
<br>
    CppSymbolFinder finder(src);<br>
    Region rc;<br>
<br>
    CHECK(finder.find_class(&quot;Weird&quot;, rc));<br>
    CHECK(rc.start_line == 1);<br>
    CHECK(rc.end_line == 6);<br>
<br>
    Region rm;<br>
    CHECK(finder.find_method(&quot;Weird&quot;, &quot;run&quot;, rm));<br>
    CHECK(rm.start_line == 5);<br>
    CHECK(rm.end_line == 5);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
