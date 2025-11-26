<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/collector.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;collector.h&quot;<br>
#include &quot;glob.h&quot;<br>
#include &quot;util.h&quot;<br>
#include &lt;algorithm&gt;<br>
#include &lt;filesystem&gt;<br>
#include &lt;functional&gt;<br>
#include &lt;iostream&gt;<br>
#include &lt;sstream&gt;<br>
#include &lt;unordered_set&gt;<br>
<br>
namespace fs = std::filesystem;<br>
<br>
// ---------------------------------------------------------------<br>
// Вспомогалки для glob<br>
// ---------------------------------------------------------------<br>
<br>
// pattern like:  &quot;src/*&quot;  or  &quot;datas/**&quot;<br>
static bool has_double_star(const std::string &amp;s)<br>
{<br>
&emsp;return s.find(&quot;**&quot;) != std::string::npos;<br>
}<br>
<br>
static bool has_single_star(const std::string &amp;s)<br>
{<br>
&emsp;return s.find('*') != std::string::npos &amp;&amp; !has_double_star(s);<br>
}<br>
<br>
// Расширение одного правила<br>
static void expand_rule(const Rule &amp;r, std::vector&lt;fs::path&gt; &amp;out)<br>
{<br>
&emsp;const std::string &amp;pat = r.pattern;<br>
&emsp;std::error_code ec;<br>
<br>
&emsp;// ------------------------------------------------------------------<br>
&emsp;// новый glob — вынесен в glob.cpp<br>
&emsp;{<br>
&emsp;&emsp;auto v = expand_glob(pat);<br>
&emsp;&emsp;out.insert(out.end(), v.begin(), v.end());<br>
&emsp;&emsp;return;<br>
&emsp;}<br>
<br>
&emsp;// ==== * (one level) ====<br>
&emsp;if (has_single_star(pat))<br>
&emsp;{<br>
&emsp;&emsp;fs::path dir = fs::path(pat).parent_path();<br>
&emsp;&emsp;std::string mask = fs::path(pat).filename().string();<br>
<br>
&emsp;&emsp;if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec))<br>
&emsp;&emsp;&emsp;return;<br>
<br>
&emsp;&emsp;for (auto &amp;e : fs::directory_iterator(dir, ec))<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;if (e.is_regular_file() &amp;&amp; match_simple(e.path(), mask))<br>
&emsp;&emsp;&emsp;&emsp;out.push_back(e.path());<br>
&emsp;&emsp;}<br>
&emsp;&emsp;return;<br>
&emsp;}<br>
<br>
&emsp;// ==== Прямой путь ====<br>
&emsp;fs::path p = pat;<br>
<br>
&emsp;if (fs::is_regular_file(p, ec))<br>
&emsp;{<br>
&emsp;&emsp;out.push_back(fs::canonical(p, ec));<br>
&emsp;&emsp;return;<br>
&emsp;}<br>
<br>
&emsp;if (fs::is_directory(p, ec))<br>
&emsp;{<br>
&emsp;&emsp;for (auto &amp;e : fs::directory_iterator(p, ec))<br>
&emsp;&emsp;&emsp;if (e.is_regular_file())<br>
&emsp;&emsp;&emsp;&emsp;out.push_back(e.path());<br>
&emsp;&emsp;return;<br>
&emsp;}<br>
}<br>
<br>
// ---------------------------------------------------------------<br>
// collect_from_rules()<br>
// ---------------------------------------------------------------<br>
<br>
std::vector&lt;fs::path&gt; collect_from_rules(const std::vector&lt;Rule&gt; &amp;rules,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const Options &amp;opt)<br>
{<br>
&emsp;std::vector&lt;fs::path&gt; tmp;<br>
&emsp;std::error_code ec;<br>
<br>
&emsp;// 1. Собираем все include-рулы<br>
&emsp;for (const auto &amp;r : rules)<br>
&emsp;&emsp;if (!r.exclude)<br>
&emsp;&emsp;&emsp;expand_rule(r, tmp);<br>
<br>
&emsp;// 2. Применяем exclude-рулы через нормальный glob<br>
&emsp;for (const auto &amp;r : rules)<br>
&emsp;{<br>
&emsp;&emsp;if (!r.exclude)<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;auto bad = expand_glob(r.pattern);<br>
<br>
&emsp;&emsp;std::unordered_set&lt;std::string&gt; bad_abs;<br>
&emsp;&emsp;bad_abs.reserve(bad.size());<br>
<br>
&emsp;&emsp;for (auto &amp;b : bad)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::error_code ec;<br>
&emsp;&emsp;&emsp;auto absb = fs::absolute(b, ec);<br>
&emsp;&emsp;&emsp;bad_abs.insert(absb.string());<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;tmp.erase(std::remove_if(tmp.begin(),<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;tmp.end(),<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;[&amp;](const fs::path &amp;p)<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;std::error_code ec;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;auto absp = fs::absolute(p, ec);<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;return bad_abs.find(absp.string()) !=<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;bad_abs.end();<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;}),<br>
&emsp;&emsp;&emsp;&emsp;tmp.end());<br>
&emsp;}<br>
<br>
&emsp;// 3. Убираем дубликаты<br>
&emsp;std::sort(tmp.begin(), tmp.end());<br>
&emsp;tmp.erase(std::unique(tmp.begin(), tmp.end()), tmp.end());<br>
<br>
&emsp;return tmp;<br>
}<br>
<br>
// ---------------------------------------------------------------<br>
// collect_from_paths()<br>
// ---------------------------------------------------------------<br>
<br>
std::vector&lt;fs::path&gt; collect_from_paths(const std::vector&lt;std::string&gt; &amp;paths,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const Options &amp;opt)<br>
{<br>
&emsp;std::vector&lt;fs::path&gt; out;<br>
<br>
&emsp;for (auto &amp;s : paths)<br>
&emsp;{<br>
&emsp;&emsp;fs::path p = s;<br>
&emsp;&emsp;std::error_code ec;<br>
<br>
&emsp;&emsp;if (!fs::exists(p, ec))<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::cerr &lt;&lt; &quot;Not found: &quot; &lt;&lt; p &lt;&lt; &quot;\n&quot;;<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;if (fs::is_regular_file(p, ec))<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;out.push_back(fs::canonical(p, ec));<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;if (fs::is_directory(p, ec))<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;if (opt.recursive)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;for (auto &amp;e : fs::recursive_directory_iterator(p, ec))<br>
&emsp;&emsp;&emsp;&emsp;&emsp;if (e.is_regular_file())<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;out.push_back(e.path());<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;for (auto &amp;e : fs::directory_iterator(p, ec))<br>
&emsp;&emsp;&emsp;&emsp;&emsp;if (e.is_regular_file())<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;out.push_back(e.path());<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
&emsp;}<br>
<br>
&emsp;// Убираем дубликаты<br>
&emsp;std::sort(out.begin(), out.end());<br>
&emsp;out.erase(std::unique(out.begin(), out.end()), out.end());<br>
<br>
&emsp;return out;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
