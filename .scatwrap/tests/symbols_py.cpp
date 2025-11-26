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
&emsp;std::string src = &quot;class Foo:\n&quot;            // 0<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    def bar(self):\n&quot;    // 1<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;        pass\n&quot;          // 2<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;\n&quot;                      // 3<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    def baz(self, x):\n&quot; // 4<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;        return x + 1\n&quot;; // 5<br>
<br>
&emsp;PythonSymbolFinder finder(src);<br>
&emsp;Region r_class;<br>
<br>
&emsp;CHECK(finder.find_class(&quot;Foo&quot;, r_class));<br>
&emsp;CHECK(r_class.start_line == 0);<br>
&emsp;CHECK(r_class.end_line == 5);<br>
<br>
&emsp;Region r_bar;<br>
&emsp;CHECK(finder.find_method(&quot;Foo&quot;, &quot;bar&quot;, r_bar));<br>
&emsp;CHECK(r_bar.start_line == 1);<br>
&emsp;CHECK(r_bar.end_line == 2);<br>
<br>
&emsp;Region r_baz;<br>
&emsp;CHECK(finder.find_method(&quot;Foo&quot;, &quot;baz&quot;, r_baz));<br>
&emsp;CHECK(r_baz.start_line == 4);<br>
&emsp;CHECK(r_baz.end_line == 5);<br>
}<br>
<br>
TEST_CASE(&quot;PythonSymbolFinder: ignores top-level functions&quot;)<br>
{<br>
&emsp;std::string src = &quot;def foo():\n&quot;         // 0<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    pass\n&quot;           // 1<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;\n&quot;                   // 2<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;class Foo:\n&quot;         // 3<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    def foo(self):\n&quot; // 4<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;        pass\n&quot;;      // 5<br>
<br>
&emsp;PythonSymbolFinder finder(src);<br>
&emsp;Region r_class;<br>
<br>
&emsp;CHECK(finder.find_class(&quot;Foo&quot;, r_class));<br>
&emsp;CHECK(r_class.start_line == 3);<br>
&emsp;CHECK(r_class.end_line == 5);<br>
<br>
&emsp;Region r_method;<br>
&emsp;CHECK(finder.find_method(&quot;Foo&quot;, &quot;foo&quot;, r_method));<br>
&emsp;CHECK(r_method.start_line == 4);<br>
&emsp;CHECK(r_method.end_line == 5);<br>
}<br>
<br>
TEST_CASE(&quot;PythonSymbolFinder: decorated methods&quot;)<br>
{<br>
&emsp;std::string src = &quot;class Foo:\n&quot;         // 0<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    @decorator\n&quot;     // 1<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    def bar(self):\n&quot; // 2<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;        pass\n&quot;;      // 3<br>
<br>
&emsp;PythonSymbolFinder finder(src);<br>
&emsp;Region r_method;<br>
<br>
&emsp;CHECK(finder.find_method(&quot;Foo&quot;, &quot;bar&quot;, r_method));<br>
&emsp;CHECK(r_method.start_line == 1); // вместе с декоратором<br>
&emsp;CHECK(r_method.end_line == 3);<br>
}<br>
<br>
TEST_CASE(&quot;PythonSymbolFinder: nested class and methods&quot;)<br>
{<br>
&emsp;std::string src = &quot;class Outer:\n&quot;         // 0<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    def a(self):\n&quot;     // 1<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;        pass\n&quot;         // 2<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    class Inner:\n&quot;     // 3<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;        def b(self):\n&quot; // 4<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;            pass\n&quot;;    // 5<br>
<br>
&emsp;PythonSymbolFinder finder(src);<br>
<br>
&emsp;Region r_outer;<br>
&emsp;CHECK(finder.find_class(&quot;Outer&quot;, r_outer));<br>
&emsp;CHECK(r_outer.start_line == 0);<br>
&emsp;CHECK(r_outer.end_line == 5);<br>
<br>
&emsp;Region r_a;<br>
&emsp;CHECK(finder.find_method(&quot;Outer&quot;, &quot;a&quot;, r_a));<br>
&emsp;CHECK(r_a.start_line == 1);<br>
&emsp;CHECK(r_a.end_line == 2);<br>
<br>
&emsp;Region r_inner_class;<br>
&emsp;CHECK(finder.find_class(&quot;Inner&quot;, r_inner_class));<br>
&emsp;CHECK(r_inner_class.start_line == 3);<br>
&emsp;CHECK(r_inner_class.end_line == 5);<br>
<br>
&emsp;Region r_b;<br>
&emsp;CHECK(finder.find_method(&quot;Inner&quot;, &quot;b&quot;, r_b));<br>
&emsp;CHECK(r_b.start_line == 4);<br>
&emsp;CHECK(r_b.end_line == 5);<br>
}<br>
<br>
TEST_CASE(&quot;PythonSymbolFinder: async methods&quot;)<br>
{<br>
&emsp;std::string src = &quot;class Foo:\n&quot;                 // 0<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    async def bar(self):\n&quot;   // 1<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;        await something()\n&quot;; // 2<br>
<br>
&emsp;PythonSymbolFinder finder(src);<br>
<br>
&emsp;Region r_class;<br>
&emsp;CHECK(finder.find_class(&quot;Foo&quot;, r_class));<br>
&emsp;CHECK(r_class.start_line == 0);<br>
&emsp;CHECK(r_class.end_line == 2);<br>
<br>
&emsp;Region r_bar;<br>
&emsp;CHECK(finder.find_method(&quot;Foo&quot;, &quot;bar&quot;, r_bar));<br>
&emsp;CHECK(r_bar.start_line == 1);<br>
&emsp;CHECK(r_bar.end_line == 2);<br>
}<br>
<br>
TEST_CASE(&quot;PythonSymbolFinder: whitespace, comments and docstrings&quot;)<br>
{<br>
&emsp;std::string src = &quot;#!/usr/bin/env python3\n&quot;                // 0<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;\n&quot;                                      // 1<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;class   Weird  (  Base  ):\n&quot;            // 2<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    \\\&quot;\\\&quot;\\\&quot;docstring\\\&quot;\\\&quot;\\\&quot;\n&quot; // 3<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    # comment inside class\n&quot;            // 4<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    def   run ( self,  x ):\n&quot;           // 5<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;        # inner comment\n&quot;               // 6<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;        return x + 1\n&quot;                  // 7<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;\n&quot;                                      // 8<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;def run(x):\n&quot;                           // 9<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&quot;    return x\n&quot;;                         // 10<br>
<br>
&emsp;PythonSymbolFinder finder(src);<br>
<br>
&emsp;Region r_class;<br>
&emsp;CHECK(finder.find_class(&quot;Weird&quot;, r_class));<br>
&emsp;CHECK(r_class.start_line == 2);<br>
&emsp;CHECK(r_class.end_line == 7);<br>
<br>
&emsp;Region r_run;<br>
&emsp;CHECK(finder.find_method(&quot;Weird&quot;, &quot;run&quot;, r_run));<br>
&emsp;CHECK(r_run.start_line == 5);<br>
&emsp;CHECK(r_run.end_line == 7);<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
