<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/symbols_py.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;doctest/doctest.h&quot;<br>
#include &quot;symbols.h&quot;<br>
<br>
#include &lt;string&gt;<br>
<br>
TEST_CASE(&quot;PythonSymbolFinder: simple class and methods&quot;)<br>
{<br>
    std::string src = &quot;class Foo:\n&quot;            // 0<br>
                      &quot;    def bar(self):\n&quot;    // 1<br>
                      &quot;        pass\n&quot;          // 2<br>
                      &quot;\n&quot;                      // 3<br>
                      &quot;    def baz(self, x):\n&quot; // 4<br>
                      &quot;        return x + 1\n&quot;; // 5<br>
<br>
    PythonSymbolFinder finder(src);<br>
    Region r_class;<br>
<br>
    CHECK(finder.find_class(&quot;Foo&quot;, r_class));<br>
    CHECK(r_class.start_line == 0);<br>
    CHECK(r_class.end_line == 5);<br>
<br>
    Region r_bar;<br>
    CHECK(finder.find_method(&quot;Foo&quot;, &quot;bar&quot;, r_bar));<br>
    CHECK(r_bar.start_line == 1);<br>
    CHECK(r_bar.end_line == 2);<br>
<br>
    Region r_baz;<br>
    CHECK(finder.find_method(&quot;Foo&quot;, &quot;baz&quot;, r_baz));<br>
    CHECK(r_baz.start_line == 4);<br>
    CHECK(r_baz.end_line == 5);<br>
}<br>
<br>
TEST_CASE(&quot;PythonSymbolFinder: ignores top-level functions&quot;)<br>
{<br>
    std::string src = &quot;def foo():\n&quot;         // 0<br>
                      &quot;    pass\n&quot;           // 1<br>
                      &quot;\n&quot;                   // 2<br>
                      &quot;class Foo:\n&quot;         // 3<br>
                      &quot;    def foo(self):\n&quot; // 4<br>
                      &quot;        pass\n&quot;;      // 5<br>
<br>
    PythonSymbolFinder finder(src);<br>
    Region r_class;<br>
<br>
    CHECK(finder.find_class(&quot;Foo&quot;, r_class));<br>
    CHECK(r_class.start_line == 3);<br>
    CHECK(r_class.end_line == 5);<br>
<br>
    Region r_method;<br>
    CHECK(finder.find_method(&quot;Foo&quot;, &quot;foo&quot;, r_method));<br>
    CHECK(r_method.start_line == 4);<br>
    CHECK(r_method.end_line == 5);<br>
}<br>
<br>
TEST_CASE(&quot;PythonSymbolFinder: decorated methods&quot;)<br>
{<br>
    std::string src = &quot;class Foo:\n&quot;         // 0<br>
                      &quot;    @decorator\n&quot;     // 1<br>
                      &quot;    def bar(self):\n&quot; // 2<br>
                      &quot;        pass\n&quot;;      // 3<br>
<br>
    PythonSymbolFinder finder(src);<br>
    Region r_method;<br>
<br>
    CHECK(finder.find_method(&quot;Foo&quot;, &quot;bar&quot;, r_method));<br>
    CHECK(r_method.start_line == 1); // вместе с декоратором<br>
    CHECK(r_method.end_line == 3);<br>
}<br>
<br>
TEST_CASE(&quot;PythonSymbolFinder: nested class and methods&quot;)<br>
{<br>
    std::string src = &quot;class Outer:\n&quot;         // 0<br>
                      &quot;    def a(self):\n&quot;     // 1<br>
                      &quot;        pass\n&quot;         // 2<br>
                      &quot;    class Inner:\n&quot;     // 3<br>
                      &quot;        def b(self):\n&quot; // 4<br>
                      &quot;            pass\n&quot;;    // 5<br>
<br>
    PythonSymbolFinder finder(src);<br>
<br>
    Region r_outer;<br>
    CHECK(finder.find_class(&quot;Outer&quot;, r_outer));<br>
    CHECK(r_outer.start_line == 0);<br>
    CHECK(r_outer.end_line == 5);<br>
<br>
    Region r_a;<br>
    CHECK(finder.find_method(&quot;Outer&quot;, &quot;a&quot;, r_a));<br>
    CHECK(r_a.start_line == 1);<br>
    CHECK(r_a.end_line == 2);<br>
<br>
    Region r_inner_class;<br>
    CHECK(finder.find_class(&quot;Inner&quot;, r_inner_class));<br>
    CHECK(r_inner_class.start_line == 3);<br>
    CHECK(r_inner_class.end_line == 5);<br>
<br>
    Region r_b;<br>
    CHECK(finder.find_method(&quot;Inner&quot;, &quot;b&quot;, r_b));<br>
    CHECK(r_b.start_line == 4);<br>
    CHECK(r_b.end_line == 5);<br>
}<br>
<br>
TEST_CASE(&quot;PythonSymbolFinder: async methods&quot;)<br>
{<br>
    std::string src = &quot;class Foo:\n&quot;                 // 0<br>
                      &quot;    async def bar(self):\n&quot;   // 1<br>
                      &quot;        await something()\n&quot;; // 2<br>
<br>
    PythonSymbolFinder finder(src);<br>
<br>
    Region r_class;<br>
    CHECK(finder.find_class(&quot;Foo&quot;, r_class));<br>
    CHECK(r_class.start_line == 0);<br>
    CHECK(r_class.end_line == 2);<br>
<br>
    Region r_bar;<br>
    CHECK(finder.find_method(&quot;Foo&quot;, &quot;bar&quot;, r_bar));<br>
    CHECK(r_bar.start_line == 1);<br>
    CHECK(r_bar.end_line == 2);<br>
}<br>
<br>
TEST_CASE(&quot;PythonSymbolFinder: whitespace, comments and docstrings&quot;)<br>
{<br>
    std::string src = &quot;#!/usr/bin/env python3\n&quot;                // 0<br>
                      &quot;\n&quot;                                      // 1<br>
                      &quot;class   Weird  (  Base  ):\n&quot;            // 2<br>
                      &quot;    \\\&quot;\\\&quot;\\\&quot;docstring\\\&quot;\\\&quot;\\\&quot;\n&quot; // 3<br>
                      &quot;    # comment inside class\n&quot;            // 4<br>
                      &quot;    def   run ( self,  x ):\n&quot;           // 5<br>
                      &quot;        # inner comment\n&quot;               // 6<br>
                      &quot;        return x + 1\n&quot;                  // 7<br>
                      &quot;\n&quot;                                      // 8<br>
                      &quot;def run(x):\n&quot;                           // 9<br>
                      &quot;    return x\n&quot;;                         // 10<br>
<br>
    PythonSymbolFinder finder(src);<br>
<br>
    Region r_class;<br>
    CHECK(finder.find_class(&quot;Weird&quot;, r_class));<br>
    CHECK(r_class.start_line == 2);<br>
    CHECK(r_class.end_line == 7);<br>
<br>
    Region r_run;<br>
    CHECK(finder.find_method(&quot;Weird&quot;, &quot;run&quot;, r_run));<br>
    CHECK(r_run.start_line == 5);<br>
    CHECK(r_run.end_line == 7);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
