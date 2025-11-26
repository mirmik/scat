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
&emsp;return s == &quot;*&quot; || s == &quot;**&quot; || s.find('*') != std::string::npos;<br>
}<br>
<br>
// matcit только filename, НЕ директорию<br>
static bool match_name(const fs::path &amp;p, const std::string &amp;seg)<br>
{<br>
&emsp;return match_simple(p, seg);<br>
}<br>
<br>
std::vector&lt;fs::path&gt; expand_glob(const std::string &amp;pattern)<br>
{<br>
&emsp;std::vector&lt;fs::path&gt; out;<br>
&emsp;std::error_code ec;<br>
<br>
&emsp;fs::path pat(pattern);<br>
&emsp;fs::path root = pat.root_path(); // &quot;/&quot; или &quot;C:&quot; или &quot;&quot;<br>
&emsp;fs::path rel = pat.relative_path();<br>
<br>
&emsp;if (root.empty())<br>
&emsp;&emsp;root = &quot;.&quot;;<br>
<br>
&emsp;// Разбиваем на части<br>
&emsp;std::vector&lt;std::string&gt; parts;<br>
&emsp;{<br>
&emsp;&emsp;std::stringstream ss(rel.generic_string());<br>
&emsp;&emsp;std::string seg;<br>
&emsp;&emsp;while (std::getline(ss, seg, '/'))<br>
&emsp;&emsp;&emsp;parts.push_back(seg);<br>
&emsp;}<br>
<br>
&emsp;// Определяем статический префикс (до первой звёздочки)<br>
&emsp;size_t start = 0;<br>
&emsp;for (; start &lt; parts.size(); ++start)<br>
&emsp;{<br>
&emsp;&emsp;const auto &amp;seg = parts[start];<br>
&emsp;&emsp;if (is_wildcard(seg))<br>
&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;root /= seg;<br>
&emsp;}<br>
<br>
&emsp;if (!fs::exists(root, ec))<br>
&emsp;&emsp;return out;<br>
<br>
&emsp;// Основной обход<br>
&emsp;std::function&lt;void(const fs::path &amp;, size_t)&gt; walk =<br>
&emsp;&emsp;[&amp;](const fs::path &amp;base, size_t idx)<br>
&emsp;{<br>
&emsp;&emsp;if (idx == parts.size())<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;if (fs::is_regular_file(base, ec))<br>
&emsp;&emsp;&emsp;&emsp;out.push_back(base);<br>
&emsp;&emsp;&emsp;return;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;const std::string &amp;seg = parts[idx];<br>
<br>
&emsp;&emsp;// ---------------------------------------------------------------------<br>
&emsp;&emsp;// &quot;**&quot; → полный рекурсивный обход<br>
&emsp;&emsp;// ---------------------------------------------------------------------<br>
&emsp;&emsp;if (seg == &quot;**&quot;)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;// вариант 0 уровней<br>
&emsp;&emsp;&emsp;walk(base, idx + 1);<br>
<br>
&emsp;&emsp;&emsp;// вариант 1+ уровней<br>
&emsp;&emsp;&emsp;if (fs::is_directory(base, ec))<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;for (auto &amp;e : fs::directory_iterator(base, ec))<br>
&emsp;&emsp;&emsp;&emsp;&emsp;walk(e.path(), idx); // двигаемся дальше, idx НЕ увеличиваем<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;return;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// ---------------------------------------------------------------------<br>
&emsp;&emsp;// &quot;*&quot; или &quot;ma*sk&quot; → только ОДИН уровень, без рекурсии<br>
&emsp;&emsp;// ---------------------------------------------------------------------<br>
&emsp;&emsp;if (seg.find('*') != std::string::npos)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;if (!fs::is_directory(base, ec))<br>
&emsp;&emsp;&emsp;&emsp;return;<br>
<br>
&emsp;&emsp;&emsp;for (auto &amp;e : fs::directory_iterator(base, ec))<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;if (!match_name(e.path(), seg))<br>
&emsp;&emsp;&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;&emsp;&emsp;// если это последний сегмент — принимаем только файлы<br>
&emsp;&emsp;&emsp;&emsp;if (idx + 1 == parts.size())<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;if (e.is_regular_file())<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;out.push_back(e.path());<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;walk(e.path(), idx + 1);<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;return;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// ---------------------------------------------------------------------<br>
&emsp;&emsp;// обычное имя → просто переходим<br>
&emsp;&emsp;// ---------------------------------------------------------------------<br>
&emsp;&emsp;fs::path next = base / seg;<br>
&emsp;&emsp;if (fs::exists(next, ec))<br>
&emsp;&emsp;&emsp;walk(next, idx + 1);<br>
&emsp;};<br>
<br>
&emsp;walk(root, start);<br>
<br>
&emsp;// удалить дубликаты<br>
&emsp;std::sort(out.begin(), out.end());<br>
&emsp;out.erase(std::unique(out.begin(), out.end()), out.end());<br>
<br>
&emsp;return out;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
