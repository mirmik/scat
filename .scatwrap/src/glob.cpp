<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/glob.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;glob.h&quot;<br>
#include &quot;util.h&quot;<br>
#include &lt;algorithm&gt;<br>
#include &lt;filesystem&gt;<br>
#include &lt;functional&gt;<br>
#include &lt;sstream&gt;<br>
#include &lt;string&gt;<br>
#include &lt;vector&gt;<br>
<br>
namespace fs = std::filesystem;<br>
<br>
static bool is_wildcard(const std::string &amp;s)<br>
{<br>
&#9;return s == &quot;*&quot; || s == &quot;**&quot; || s.find('*') != std::string::npos;<br>
}<br>
<br>
// matcit только filename, НЕ директорию<br>
static bool match_name(const fs::path &amp;p, const std::string &amp;seg)<br>
{<br>
&#9;return match_simple(p, seg);<br>
}<br>
<br>
std::vector&lt;fs::path&gt; expand_glob(const std::string &amp;pattern)<br>
{<br>
&#9;std::vector&lt;fs::path&gt; out;<br>
&#9;std::error_code ec;<br>
<br>
&#9;fs::path pat(pattern);<br>
&#9;fs::path root = pat.root_path(); // &quot;/&quot; или &quot;C:&quot; или &quot;&quot;<br>
&#9;fs::path rel = pat.relative_path();<br>
<br>
&#9;if (root.empty())<br>
&#9;&#9;root = &quot;.&quot;;<br>
<br>
&#9;// Разбиваем на части<br>
&#9;std::vector&lt;std::string&gt; parts;<br>
&#9;{<br>
&#9;&#9;std::stringstream ss(rel.generic_string());<br>
&#9;&#9;std::string seg;<br>
&#9;&#9;while (std::getline(ss, seg, '/'))<br>
&#9;&#9;&#9;parts.push_back(seg);<br>
&#9;}<br>
<br>
&#9;// Определяем статический префикс (до первой звёздочки)<br>
&#9;size_t start = 0;<br>
&#9;for (; start &lt; parts.size(); ++start)<br>
&#9;{<br>
&#9;&#9;const auto &amp;seg = parts[start];<br>
&#9;&#9;if (is_wildcard(seg))<br>
&#9;&#9;&#9;break;<br>
&#9;&#9;root /= seg;<br>
&#9;}<br>
<br>
&#9;if (!fs::exists(root, ec))<br>
&#9;&#9;return out;<br>
<br>
&#9;// Основной обход<br>
&#9;std::function&lt;void(const fs::path &amp;, size_t)&gt; walk =<br>
&#9;&#9;[&amp;](const fs::path &amp;base, size_t idx)<br>
&#9;{<br>
&#9;&#9;if (idx == parts.size())<br>
&#9;&#9;{<br>
&#9;&#9;&#9;if (fs::is_regular_file(base, ec))<br>
&#9;&#9;&#9;&#9;out.push_back(base);<br>
&#9;&#9;&#9;return;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;const std::string &amp;seg = parts[idx];<br>
<br>
&#9;&#9;// ---------------------------------------------------------------------<br>
&#9;&#9;// &quot;**&quot; → полный рекурсивный обход<br>
&#9;&#9;// ---------------------------------------------------------------------<br>
&#9;&#9;if (seg == &quot;**&quot;)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;// вариант 0 уровней<br>
&#9;&#9;&#9;walk(base, idx + 1);<br>
<br>
&#9;&#9;&#9;// вариант 1+ уровней<br>
&#9;&#9;&#9;if (fs::is_directory(base, ec))<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;for (auto &amp;e : fs::directory_iterator(base, ec))<br>
&#9;&#9;&#9;&#9;&#9;walk(e.path(), idx); // двигаемся дальше, idx НЕ увеличиваем<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;return;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// ---------------------------------------------------------------------<br>
&#9;&#9;// &quot;*&quot; или &quot;ma*sk&quot; → только ОДИН уровень, без рекурсии<br>
&#9;&#9;// ---------------------------------------------------------------------<br>
&#9;&#9;if (seg.find('*') != std::string::npos)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;if (!fs::is_directory(base, ec))<br>
&#9;&#9;&#9;&#9;return;<br>
<br>
&#9;&#9;&#9;for (auto &amp;e : fs::directory_iterator(base, ec))<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;if (!match_name(e.path(), seg))<br>
&#9;&#9;&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;&#9;&#9;// если это последний сегмент — принимаем только файлы<br>
&#9;&#9;&#9;&#9;if (idx + 1 == parts.size())<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;if (e.is_regular_file())<br>
&#9;&#9;&#9;&#9;&#9;&#9;out.push_back(e.path());<br>
&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;&#9;else<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;walk(e.path(), idx + 1);<br>
&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;return;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// ---------------------------------------------------------------------<br>
&#9;&#9;// обычное имя → просто переходим<br>
&#9;&#9;// ---------------------------------------------------------------------<br>
&#9;&#9;fs::path next = base / seg;<br>
&#9;&#9;if (fs::exists(next, ec))<br>
&#9;&#9;&#9;walk(next, idx + 1);<br>
&#9;};<br>
<br>
&#9;walk(root, start);<br>
<br>
&#9;// удалить дубликаты<br>
&#9;std::sort(out.begin(), out.end());<br>
&#9;out.erase(std::unique(out.begin(), out.end()), out.end());<br>
<br>
&#9;return out;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
