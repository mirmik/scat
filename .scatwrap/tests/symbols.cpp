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
&#9;std::string src = &quot;class Foo {\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;public:\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;    void bar();\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;};\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;class Bar {};\n&quot;;<br>
<br>
&#9;CppSymbolFinder finder(src);<br>
&#9;Region r;<br>
<br>
&#9;CHECK(finder.find_class(&quot;Foo&quot;, r));<br>
&#9;CHECK(r.start_line == 0);<br>
&#9;CHECK(r.end_line == 3);<br>
<br>
&#9;// Другой класс тоже должен находиться<br>
&#9;CHECK(finder.find_class(&quot;Bar&quot;, r));<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: ignores forward declaration&quot;)<br>
{<br>
&#9;std::string src = &quot;class Foo;\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;class Foo {\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;public:\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;    void bar();\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;};\n&quot;;<br>
<br>
&#9;CppSymbolFinder finder(src);<br>
&#9;Region r;<br>
<br>
&#9;CHECK(finder.find_class(&quot;Foo&quot;, r));<br>
&#9;CHECK(r.start_line == 2);<br>
&#9;CHECK(r.end_line == 5);<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: finds method declarations inside class&quot;)<br>
{<br>
&#9;std::string src = &quot;class Foo {\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;public:\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;    int bar(int x) const;\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;    void baz();\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;};\n&quot;;<br>
<br>
&#9;CppSymbolFinder finder(src);<br>
<br>
&#9;Region r1;<br>
&#9;CHECK(finder.find_method(&quot;Foo&quot;, &quot;bar&quot;, r1));<br>
&#9;CHECK(r1.start_line == 2);<br>
&#9;CHECK(r1.end_line == 2);<br>
<br>
&#9;Region r2;<br>
&#9;CHECK(finder.find_method(&quot;Foo&quot;, &quot;baz&quot;, r2));<br>
&#9;CHECK(r2.start_line == 3);<br>
&#9;CHECK(r2.end_line == 3);<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: finds inline method definition&quot;)<br>
{<br>
&#9;std::string src = &quot;class Foo {\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;public:\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;    int bar(int x) const {\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;        return x + 1;\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;    }\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;};\n&quot;;<br>
<br>
&#9;CppSymbolFinder finder(src);<br>
&#9;Region r;<br>
<br>
&#9;CHECK(finder.find_method(&quot;Foo&quot;, &quot;bar&quot;, r));<br>
&#9;CHECK(r.start_line == 2);<br>
&#9;CHECK(r.end_line == 4);<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: method not found returns false&quot;)<br>
{<br>
&#9;std::string src = &quot;class Foo {\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;public:\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;    void bar();\n&quot;<br>
&#9;&#9;&#9;&#9;&#9;&quot;};\n&quot;;<br>
<br>
&#9;CppSymbolFinder finder(src);<br>
&#9;Region r;<br>
<br>
&#9;CHECK_FALSE(finder.find_method(&quot;Foo&quot;, &quot;baz&quot;, r));<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: complex file with multiple classes and comments&quot;)<br>
{<br>
&#9;std::string src = &quot;#pragma once\n&quot;                                  // 0<br>
&#9;&#9;&#9;&#9;&#9;&quot;\n&quot;                                              // 1<br>
&#9;&#9;&#9;&#9;&#9;&quot;// forward decl\n&quot;                               // 2<br>
&#9;&#9;&#9;&#9;&#9;&quot;class Forward;\n&quot;                                // 3<br>
&#9;&#9;&#9;&#9;&#9;&quot;\n&quot;                                              // 4<br>
&#9;&#9;&#9;&#9;&#9;&quot;/* comment with class Fake { */\n&quot;               // 5<br>
&#9;&#9;&#9;&#9;&#9;&quot;\n&quot;                                              // 6<br>
&#9;&#9;&#9;&#9;&#9;&quot;class First {\n&quot;                                 // 7<br>
&#9;&#9;&#9;&#9;&#9;&quot;public:\n&quot;                                       // 8<br>
&#9;&#9;&#9;&#9;&#9;&quot;    // method comment\n&quot;                         // 9<br>
&#9;&#9;&#9;&#9;&#9;&quot;    int m1(int x) const;\n&quot;                      // 10<br>
&#9;&#9;&#9;&#9;&#9;&quot;    std::string m2() const {\n&quot;                  // 11<br>
&#9;&#9;&#9;&#9;&#9;&quot;        return \&quot;class Fake {\&quot;; // in string\n&quot; // 12<br>
&#9;&#9;&#9;&#9;&#9;&quot;    }\n&quot;                                         // 13<br>
&#9;&#9;&#9;&#9;&#9;&quot;};\n&quot;                                            // 14<br>
&#9;&#9;&#9;&#9;&#9;&quot;\n&quot;                                              // 15<br>
&#9;&#9;&#9;&#9;&#9;&quot;// Another class\n&quot;                              // 16<br>
&#9;&#9;&#9;&#9;&#9;&quot;struct Second {\n&quot;                               // 17<br>
&#9;&#9;&#9;&#9;&#9;&quot;    void go();\n&quot;                                // 18<br>
&#9;&#9;&#9;&#9;&#9;&quot;};\n&quot;;                                           // 19<br>
<br>
&#9;CppSymbolFinder finder(src);<br>
&#9;Region r;<br>
<br>
&#9;// forward declaration не должен считаться определением<br>
&#9;CHECK_FALSE(finder.find_class(&quot;Forward&quot;, r));<br>
<br>
&#9;// Первый класс<br>
&#9;CHECK(finder.find_class(&quot;First&quot;, r));<br>
&#9;CHECK(r.start_line == 7);<br>
&#9;CHECK(r.end_line == 14);<br>
<br>
&#9;// Второй класс<br>
&#9;CHECK(finder.find_class(&quot;Second&quot;, r));<br>
&#9;CHECK(r.start_line == 17);<br>
&#9;CHECK(r.end_line == 19);<br>
<br>
&#9;// Методы первого класса<br>
&#9;Region m1;<br>
&#9;CHECK(finder.find_method(&quot;First&quot;, &quot;m1&quot;, m1));<br>
&#9;CHECK(m1.start_line == 10);<br>
&#9;CHECK(m1.end_line == 10);<br>
<br>
&#9;Region m2;<br>
&#9;CHECK(finder.find_method(&quot;First&quot;, &quot;m2&quot;, m2));<br>
&#9;CHECK(m2.start_line == 11);<br>
&#9;CHECK(m2.end_line == 13);<br>
<br>
&#9;// Метод во втором классе<br>
&#9;Region m_go;<br>
&#9;CHECK(finder.find_method(&quot;Second&quot;, &quot;go&quot;, m_go));<br>
&#9;CHECK(m_go.start_line == 18);<br>
&#9;CHECK(m_go.end_line == 18);<br>
<br>
&#9;CHECK_FALSE(finder.find_class(&quot;Fake&quot;, r));<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: handles extra whitespace and newlines&quot;)<br>
{<br>
&#9;std::string src = &quot;  \n&quot;                 // 0<br>
&#9;&#9;&#9;&#9;&#9;&quot;class   Weird  \n&quot;    // 1<br>
&#9;&#9;&#9;&#9;&#9;&quot;  : public Base\n&quot;    // 2<br>
&#9;&#9;&#9;&#9;&#9;&quot;{\n&quot;                  // 3<br>
&#9;&#9;&#9;&#9;&#9;&quot;public:\n&quot;            // 4<br>
&#9;&#9;&#9;&#9;&#9;&quot;    void  run ( );\n&quot; // 5<br>
&#9;&#9;&#9;&#9;&#9;&quot;};\n&quot;;                // 6<br>
<br>
&#9;CppSymbolFinder finder(src);<br>
&#9;Region rc;<br>
<br>
&#9;CHECK(finder.find_class(&quot;Weird&quot;, rc));<br>
&#9;CHECK(rc.start_line == 1);<br>
&#9;CHECK(rc.end_line == 6);<br>
<br>
&#9;Region rm;<br>
&#9;CHECK(finder.find_method(&quot;Weird&quot;, &quot;run&quot;, rm));<br>
&#9;CHECK(rm.start_line == 5);<br>
&#9;CHECK(rm.end_line == 5);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
