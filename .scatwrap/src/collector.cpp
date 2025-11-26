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
&#9;return s.find(&quot;**&quot;) != std::string::npos;<br>
}<br>
<br>
static bool has_single_star(const std::string &amp;s)<br>
{<br>
&#9;return s.find('*') != std::string::npos &amp;&amp; !has_double_star(s);<br>
}<br>
<br>
// Расширение одного правила<br>
static void expand_rule(const Rule &amp;r, std::vector&lt;fs::path&gt; &amp;out)<br>
{<br>
&#9;const std::string &amp;pat = r.pattern;<br>
&#9;std::error_code ec;<br>
<br>
&#9;// ------------------------------------------------------------------<br>
&#9;// новый glob — вынесен в glob.cpp<br>
&#9;{<br>
&#9;&#9;auto v = expand_glob(pat);<br>
&#9;&#9;out.insert(out.end(), v.begin(), v.end());<br>
&#9;&#9;return;<br>
&#9;}<br>
<br>
&#9;// ==== * (one level) ====<br>
&#9;if (has_single_star(pat))<br>
&#9;{<br>
&#9;&#9;fs::path dir = fs::path(pat).parent_path();<br>
&#9;&#9;std::string mask = fs::path(pat).filename().string();<br>
<br>
&#9;&#9;if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec))<br>
&#9;&#9;&#9;return;<br>
<br>
&#9;&#9;for (auto &amp;e : fs::directory_iterator(dir, ec))<br>
&#9;&#9;{<br>
&#9;&#9;&#9;if (e.is_regular_file() &amp;&amp; match_simple(e.path(), mask))<br>
&#9;&#9;&#9;&#9;out.push_back(e.path());<br>
&#9;&#9;}<br>
&#9;&#9;return;<br>
&#9;}<br>
<br>
&#9;// ==== Прямой путь ====<br>
&#9;fs::path p = pat;<br>
<br>
&#9;if (fs::is_regular_file(p, ec))<br>
&#9;{<br>
&#9;&#9;out.push_back(fs::canonical(p, ec));<br>
&#9;&#9;return;<br>
&#9;}<br>
<br>
&#9;if (fs::is_directory(p, ec))<br>
&#9;{<br>
&#9;&#9;for (auto &amp;e : fs::directory_iterator(p, ec))<br>
&#9;&#9;&#9;if (e.is_regular_file())<br>
&#9;&#9;&#9;&#9;out.push_back(e.path());<br>
&#9;&#9;return;<br>
&#9;}<br>
}<br>
<br>
// ---------------------------------------------------------------<br>
// collect_from_rules()<br>
// ---------------------------------------------------------------<br>
<br>
std::vector&lt;fs::path&gt; collect_from_rules(const std::vector&lt;Rule&gt; &amp;rules,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;const Options &amp;opt)<br>
{<br>
&#9;std::vector&lt;fs::path&gt; tmp;<br>
&#9;std::error_code ec;<br>
<br>
&#9;// 1. Собираем все include-рулы<br>
&#9;for (const auto &amp;r : rules)<br>
&#9;&#9;if (!r.exclude)<br>
&#9;&#9;&#9;expand_rule(r, tmp);<br>
<br>
&#9;// 2. Применяем exclude-рулы через нормальный glob<br>
&#9;for (const auto &amp;r : rules)<br>
&#9;{<br>
&#9;&#9;if (!r.exclude)<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;auto bad = expand_glob(r.pattern);<br>
<br>
&#9;&#9;std::unordered_set&lt;std::string&gt; bad_abs;<br>
&#9;&#9;bad_abs.reserve(bad.size());<br>
<br>
&#9;&#9;for (auto &amp;b : bad)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::error_code ec;<br>
&#9;&#9;&#9;auto absb = fs::absolute(b, ec);<br>
&#9;&#9;&#9;bad_abs.insert(absb.string());<br>
&#9;&#9;}<br>
<br>
&#9;&#9;tmp.erase(std::remove_if(tmp.begin(),<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;tmp.end(),<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;[&amp;](const fs::path &amp;p)<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;std::error_code ec;<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;auto absp = fs::absolute(p, ec);<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;return bad_abs.find(absp.string()) !=<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;bad_abs.end();<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;}),<br>
&#9;&#9;&#9;&#9;tmp.end());<br>
&#9;}<br>
<br>
&#9;// 3. Убираем дубликаты<br>
&#9;std::sort(tmp.begin(), tmp.end());<br>
&#9;tmp.erase(std::unique(tmp.begin(), tmp.end()), tmp.end());<br>
<br>
&#9;return tmp;<br>
}<br>
<br>
// ---------------------------------------------------------------<br>
// collect_from_paths()<br>
// ---------------------------------------------------------------<br>
<br>
std::vector&lt;fs::path&gt; collect_from_paths(const std::vector&lt;std::string&gt; &amp;paths,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;const Options &amp;opt)<br>
{<br>
&#9;std::vector&lt;fs::path&gt; out;<br>
<br>
&#9;for (auto &amp;s : paths)<br>
&#9;{<br>
&#9;&#9;fs::path p = s;<br>
&#9;&#9;std::error_code ec;<br>
<br>
&#9;&#9;if (!fs::exists(p, ec))<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::cerr &lt;&lt; &quot;Not found: &quot; &lt;&lt; p &lt;&lt; &quot;\n&quot;;<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;if (fs::is_regular_file(p, ec))<br>
&#9;&#9;{<br>
&#9;&#9;&#9;out.push_back(fs::canonical(p, ec));<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;if (fs::is_directory(p, ec))<br>
&#9;&#9;{<br>
&#9;&#9;&#9;if (opt.recursive)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;for (auto &amp;e : fs::recursive_directory_iterator(p, ec))<br>
&#9;&#9;&#9;&#9;&#9;if (e.is_regular_file())<br>
&#9;&#9;&#9;&#9;&#9;&#9;out.push_back(e.path());<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;else<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;for (auto &amp;e : fs::directory_iterator(p, ec))<br>
&#9;&#9;&#9;&#9;&#9;if (e.is_regular_file())<br>
&#9;&#9;&#9;&#9;&#9;&#9;out.push_back(e.path());<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
&#9;}<br>
<br>
&#9;// Убираем дубликаты<br>
&#9;std::sort(out.begin(), out.end());<br>
&#9;out.erase(std::unique(out.begin(), out.end()), out.end());<br>
<br>
&#9;return out;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
