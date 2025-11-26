<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>tests/symbols_py.cpp</title>
</head>
<body>
<pre><code>
#include &quot;doctest/doctest.h&quot;
#include &quot;symbols.h&quot;

#include &lt;string&gt;

TEST_CASE(&quot;PythonSymbolFinder: simple class and methods&quot;)
{
    std::string src = &quot;class Foo:\n&quot;            // 0
                      &quot;    def bar(self):\n&quot;    // 1
                      &quot;        pass\n&quot;          // 2
                      &quot;\n&quot;                      // 3
                      &quot;    def baz(self, x):\n&quot; // 4
                      &quot;        return x + 1\n&quot;; // 5

    PythonSymbolFinder finder(src);
    Region r_class;

    CHECK(finder.find_class(&quot;Foo&quot;, r_class));
    CHECK(r_class.start_line == 0);
    CHECK(r_class.end_line == 5);

    Region r_bar;
    CHECK(finder.find_method(&quot;Foo&quot;, &quot;bar&quot;, r_bar));
    CHECK(r_bar.start_line == 1);
    CHECK(r_bar.end_line == 2);

    Region r_baz;
    CHECK(finder.find_method(&quot;Foo&quot;, &quot;baz&quot;, r_baz));
    CHECK(r_baz.start_line == 4);
    CHECK(r_baz.end_line == 5);
}

TEST_CASE(&quot;PythonSymbolFinder: ignores top-level functions&quot;)
{
    std::string src = &quot;def foo():\n&quot;         // 0
                      &quot;    pass\n&quot;           // 1
                      &quot;\n&quot;                   // 2
                      &quot;class Foo:\n&quot;         // 3
                      &quot;    def foo(self):\n&quot; // 4
                      &quot;        pass\n&quot;;      // 5

    PythonSymbolFinder finder(src);
    Region r_class;

    CHECK(finder.find_class(&quot;Foo&quot;, r_class));
    CHECK(r_class.start_line == 3);
    CHECK(r_class.end_line == 5);

    Region r_method;
    CHECK(finder.find_method(&quot;Foo&quot;, &quot;foo&quot;, r_method));
    CHECK(r_method.start_line == 4);
    CHECK(r_method.end_line == 5);
}

TEST_CASE(&quot;PythonSymbolFinder: decorated methods&quot;)
{
    std::string src = &quot;class Foo:\n&quot;         // 0
                      &quot;    @decorator\n&quot;     // 1
                      &quot;    def bar(self):\n&quot; // 2
                      &quot;        pass\n&quot;;      // 3

    PythonSymbolFinder finder(src);
    Region r_method;

    CHECK(finder.find_method(&quot;Foo&quot;, &quot;bar&quot;, r_method));
    CHECK(r_method.start_line == 1); // вместе с декоратором
    CHECK(r_method.end_line == 3);
}

TEST_CASE(&quot;PythonSymbolFinder: nested class and methods&quot;)
{
    std::string src = &quot;class Outer:\n&quot;         // 0
                      &quot;    def a(self):\n&quot;     // 1
                      &quot;        pass\n&quot;         // 2
                      &quot;    class Inner:\n&quot;     // 3
                      &quot;        def b(self):\n&quot; // 4
                      &quot;            pass\n&quot;;    // 5

    PythonSymbolFinder finder(src);

    Region r_outer;
    CHECK(finder.find_class(&quot;Outer&quot;, r_outer));
    CHECK(r_outer.start_line == 0);
    CHECK(r_outer.end_line == 5);

    Region r_a;
    CHECK(finder.find_method(&quot;Outer&quot;, &quot;a&quot;, r_a));
    CHECK(r_a.start_line == 1);
    CHECK(r_a.end_line == 2);

    Region r_inner_class;
    CHECK(finder.find_class(&quot;Inner&quot;, r_inner_class));
    CHECK(r_inner_class.start_line == 3);
    CHECK(r_inner_class.end_line == 5);

    Region r_b;
    CHECK(finder.find_method(&quot;Inner&quot;, &quot;b&quot;, r_b));
    CHECK(r_b.start_line == 4);
    CHECK(r_b.end_line == 5);
}

TEST_CASE(&quot;PythonSymbolFinder: async methods&quot;)
{
    std::string src = &quot;class Foo:\n&quot;                 // 0
                      &quot;    async def bar(self):\n&quot;   // 1
                      &quot;        await something()\n&quot;; // 2

    PythonSymbolFinder finder(src);

    Region r_class;
    CHECK(finder.find_class(&quot;Foo&quot;, r_class));
    CHECK(r_class.start_line == 0);
    CHECK(r_class.end_line == 2);

    Region r_bar;
    CHECK(finder.find_method(&quot;Foo&quot;, &quot;bar&quot;, r_bar));
    CHECK(r_bar.start_line == 1);
    CHECK(r_bar.end_line == 2);
}

TEST_CASE(&quot;PythonSymbolFinder: whitespace, comments and docstrings&quot;)
{
    std::string src = &quot;#!/usr/bin/env python3\n&quot;                // 0
                      &quot;\n&quot;                                      // 1
                      &quot;class   Weird  (  Base  ):\n&quot;            // 2
                      &quot;    \\\&quot;\\\&quot;\\\&quot;docstring\\\&quot;\\\&quot;\\\&quot;\n&quot; // 3
                      &quot;    # comment inside class\n&quot;            // 4
                      &quot;    def   run ( self,  x ):\n&quot;           // 5
                      &quot;        # inner comment\n&quot;               // 6
                      &quot;        return x + 1\n&quot;                  // 7
                      &quot;\n&quot;                                      // 8
                      &quot;def run(x):\n&quot;                           // 9
                      &quot;    return x\n&quot;;                         // 10

    PythonSymbolFinder finder(src);

    Region r_class;
    CHECK(finder.find_class(&quot;Weird&quot;, r_class));
    CHECK(r_class.start_line == 2);
    CHECK(r_class.end_line == 7);

    Region r_run;
    CHECK(finder.find_method(&quot;Weird&quot;, &quot;run&quot;, r_run));
    CHECK(r_run.start_line == 5);
    CHECK(r_run.end_line == 7);
}

</code></pre>
</body>
</html>
