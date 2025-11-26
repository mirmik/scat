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
&emsp;std::string src = &quot;class Foo {\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;public:\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    void bar();\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;};\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;class Bar {};\n&quot;;<br>
<br>
&emsp;CppSymbolFinder finder(src);<br>
&emsp;Region r;<br>
<br>
&emsp;CHECK(finder.find_class(&quot;Foo&quot;, r));<br>
&emsp;CHECK(r.start_line == 0);<br>
&emsp;CHECK(r.end_line == 3);<br>
<br>
&emsp;// Другой класс тоже должен находиться<br>
&emsp;CHECK(finder.find_class(&quot;Bar&quot;, r));<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: ignores forward declaration&quot;)<br>
{<br>
&emsp;std::string src = &quot;class Foo;\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;class Foo {\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;public:\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    void bar();\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;};\n&quot;;<br>
<br>
&emsp;CppSymbolFinder finder(src);<br>
&emsp;Region r;<br>
<br>
&emsp;CHECK(finder.find_class(&quot;Foo&quot;, r));<br>
&emsp;CHECK(r.start_line == 2);<br>
&emsp;CHECK(r.end_line == 5);<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: finds method declarations inside class&quot;)<br>
{<br>
&emsp;std::string src = &quot;class Foo {\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;public:\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    int bar(int x) const;\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    void baz();\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;};\n&quot;;<br>
<br>
&emsp;CppSymbolFinder finder(src);<br>
<br>
&emsp;Region r1;<br>
&emsp;CHECK(finder.find_method(&quot;Foo&quot;, &quot;bar&quot;, r1));<br>
&emsp;CHECK(r1.start_line == 2);<br>
&emsp;CHECK(r1.end_line == 2);<br>
<br>
&emsp;Region r2;<br>
&emsp;CHECK(finder.find_method(&quot;Foo&quot;, &quot;baz&quot;, r2));<br>
&emsp;CHECK(r2.start_line == 3);<br>
&emsp;CHECK(r2.end_line == 3);<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: finds inline method definition&quot;)<br>
{<br>
&emsp;std::string src = &quot;class Foo {\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;public:\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    int bar(int x) const {\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;        return x + 1;\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    }\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;};\n&quot;;<br>
<br>
&emsp;CppSymbolFinder finder(src);<br>
&emsp;Region r;<br>
<br>
&emsp;CHECK(finder.find_method(&quot;Foo&quot;, &quot;bar&quot;, r));<br>
&emsp;CHECK(r.start_line == 2);<br>
&emsp;CHECK(r.end_line == 4);<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: method not found returns false&quot;)<br>
{<br>
&emsp;std::string src = &quot;class Foo {\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;public:\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    void bar();\n&quot;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;};\n&quot;;<br>
<br>
&emsp;CppSymbolFinder finder(src);<br>
&emsp;Region r;<br>
<br>
&emsp;CHECK_FALSE(finder.find_method(&quot;Foo&quot;, &quot;baz&quot;, r));<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: complex file with multiple classes and comments&quot;)<br>
{<br>
&emsp;std::string src = &quot;#pragma once\n&quot;                                  // 0<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;\n&quot;                                              // 1<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;// forward decl\n&quot;                               // 2<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;class Forward;\n&quot;                                // 3<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;\n&quot;                                              // 4<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;/* comment with class Fake { */\n&quot;               // 5<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;\n&quot;                                              // 6<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;class First {\n&quot;                                 // 7<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;public:\n&quot;                                       // 8<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    // method comment\n&quot;                         // 9<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    int m1(int x) const;\n&quot;                      // 10<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    std::string m2() const {\n&quot;                  // 11<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;        return \&quot;class Fake {\&quot;; // in string\n&quot; // 12<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    }\n&quot;                                         // 13<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;};\n&quot;                                            // 14<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;\n&quot;                                              // 15<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;// Another class\n&quot;                              // 16<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;struct Second {\n&quot;                               // 17<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    void go();\n&quot;                                // 18<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;};\n&quot;;                                           // 19<br>
<br>
&emsp;CppSymbolFinder finder(src);<br>
&emsp;Region r;<br>
<br>
&emsp;// forward declaration не должен считаться определением<br>
&emsp;CHECK_FALSE(finder.find_class(&quot;Forward&quot;, r));<br>
<br>
&emsp;// Первый класс<br>
&emsp;CHECK(finder.find_class(&quot;First&quot;, r));<br>
&emsp;CHECK(r.start_line == 7);<br>
&emsp;CHECK(r.end_line == 14);<br>
<br>
&emsp;// Второй класс<br>
&emsp;CHECK(finder.find_class(&quot;Second&quot;, r));<br>
&emsp;CHECK(r.start_line == 17);<br>
&emsp;CHECK(r.end_line == 19);<br>
<br>
&emsp;// Методы первого класса<br>
&emsp;Region m1;<br>
&emsp;CHECK(finder.find_method(&quot;First&quot;, &quot;m1&quot;, m1));<br>
&emsp;CHECK(m1.start_line == 10);<br>
&emsp;CHECK(m1.end_line == 10);<br>
<br>
&emsp;Region m2;<br>
&emsp;CHECK(finder.find_method(&quot;First&quot;, &quot;m2&quot;, m2));<br>
&emsp;CHECK(m2.start_line == 11);<br>
&emsp;CHECK(m2.end_line == 13);<br>
<br>
&emsp;// Метод во втором классе<br>
&emsp;Region m_go;<br>
&emsp;CHECK(finder.find_method(&quot;Second&quot;, &quot;go&quot;, m_go));<br>
&emsp;CHECK(m_go.start_line == 18);<br>
&emsp;CHECK(m_go.end_line == 18);<br>
<br>
&emsp;CHECK_FALSE(finder.find_class(&quot;Fake&quot;, r));<br>
}<br>
<br>
TEST_CASE(&quot;CppSymbolFinder: handles extra whitespace and newlines&quot;)<br>
{<br>
&emsp;std::string src = &quot;  \n&quot;                 // 0<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;class   Weird  \n&quot;    // 1<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;  : public Base\n&quot;    // 2<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;{\n&quot;                  // 3<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;public:\n&quot;            // 4<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    void  run ( );\n&quot; // 5<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;};\n&quot;;                // 6<br>
<br>
&emsp;CppSymbolFinder finder(src);<br>
&emsp;Region rc;<br>
<br>
&emsp;CHECK(finder.find_class(&quot;Weird&quot;, rc));<br>
&emsp;CHECK(rc.start_line == 1);<br>
&emsp;CHECK(rc.end_line == 6);<br>
<br>
&emsp;Region rm;<br>
&emsp;CHECK(finder.find_method(&quot;Weird&quot;, &quot;run&quot;, rm));<br>
&emsp;CHECK(rm.start_line == 5);<br>
&emsp;CHECK(rm.end_line == 5);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
