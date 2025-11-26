<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/symbols.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;symbols.h&quot;<br>
<br>
#include &lt;cctype&gt;<br>
<br>
CppSymbolFinder::CppSymbolFinder(const std::string &amp;text)<br>
{<br>
&#9;// Разбиваем текст на строки (независимо от \r\n / \n)<br>
&#9;std::string current;<br>
&#9;current.reserve(80);<br>
<br>
&#9;for (char c : text)<br>
&#9;{<br>
&#9;&#9;if (c == '\n')<br>
&#9;&#9;{<br>
&#9;&#9;&#9;m_lines.push_back(current);<br>
&#9;&#9;&#9;current.clear();<br>
&#9;&#9;}<br>
&#9;&#9;else if (c != '\r')<br>
&#9;&#9;{<br>
&#9;&#9;&#9;current.push_back(c);<br>
&#9;&#9;}<br>
&#9;}<br>
&#9;m_lines.push_back(current);<br>
<br>
&#9;tokenize(text);<br>
}<br>
<br>
bool CppSymbolFinder::is_ident_start(char c)<br>
{<br>
&#9;unsigned char uc = static_cast&lt;unsigned char&gt;(c);<br>
&#9;return std::isalpha(uc) || c == '_';<br>
}<br>
<br>
bool CppSymbolFinder::is_ident_char(char c)<br>
{<br>
&#9;unsigned char uc = static_cast&lt;unsigned char&gt;(c);<br>
&#9;return std::isalnum(uc) || c == '_';<br>
}<br>
<br>
void CppSymbolFinder::tokenize(const std::string &amp;text)<br>
{<br>
&#9;m_tokens.clear();<br>
<br>
&#9;bool in_block_comment = false;<br>
&#9;bool in_string = false;<br>
&#9;bool in_char = false;<br>
<br>
&#9;int line = 0;<br>
&#9;const std::size_t n = text.size();<br>
<br>
&#9;for (std::size_t i = 0; i &lt; n;)<br>
&#9;{<br>
&#9;&#9;char c = text[i];<br>
<br>
&#9;&#9;if (c == '\n')<br>
&#9;&#9;{<br>
&#9;&#9;&#9;++line;<br>
&#9;&#9;&#9;++i;<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;if (in_block_comment)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;if (c == '*' &amp;&amp; i + 1 &lt; n &amp;&amp; text[i + 1] == '/')<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;in_block_comment = false;<br>
&#9;&#9;&#9;&#9;i += 2;<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;else<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;++i;<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;if (in_string || in_char)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;if (c == '\\' &amp;&amp; i + 1 &lt; n)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;i += 2; // экранированный символ<br>
&#9;&#9;&#9;&#9;continue;<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;if (in_string &amp;&amp; c == '&quot;')<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;in_string = false;<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;else if (in_char &amp;&amp; c == '\'')<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;in_char = false;<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;++i;<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// Пробелы<br>
&#9;&#9;if (c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v')<br>
&#9;&#9;{<br>
&#9;&#9;&#9;++i;<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// Препроцессорная директива: пропускаем всю строку<br>
&#9;&#9;if (c == '#' &amp;&amp; (i == 0 || text[i - 1] == '\n'))<br>
&#9;&#9;{<br>
&#9;&#9;&#9;while (i &lt; n &amp;&amp; text[i] != '\n')<br>
&#9;&#9;&#9;&#9;++i;<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// Однострочный комментарий //<br>
&#9;&#9;if (c == '/' &amp;&amp; i + 1 &lt; n &amp;&amp; text[i + 1] == '/')<br>
&#9;&#9;{<br>
&#9;&#9;&#9;while (i &lt; n &amp;&amp; text[i] != '\n')<br>
&#9;&#9;&#9;&#9;++i;<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// Блочный комментарий /* ... */<br>
&#9;&#9;if (c == '/' &amp;&amp; i + 1 &lt; n &amp;&amp; text[i + 1] == '*')<br>
&#9;&#9;{<br>
&#9;&#9;&#9;in_block_comment = true;<br>
&#9;&#9;&#9;i += 2;<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// Строковый литерал<br>
&#9;&#9;if (c == '&quot;')<br>
&#9;&#9;{<br>
&#9;&#9;&#9;in_string = true;<br>
&#9;&#9;&#9;++i;<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// Символьный литерал<br>
&#9;&#9;if (c == '\'')<br>
&#9;&#9;{<br>
&#9;&#9;&#9;in_char = true;<br>
&#9;&#9;&#9;++i;<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// Идентификатор / ключевое слово<br>
&#9;&#9;if (is_ident_start(c))<br>
&#9;&#9;{<br>
&#9;&#9;&#9;std::size_t start = i;<br>
&#9;&#9;&#9;++i;<br>
&#9;&#9;&#9;while (i &lt; n &amp;&amp; is_ident_char(text[i]))<br>
&#9;&#9;&#9;&#9;++i;<br>
<br>
&#9;&#9;&#9;std::string ident = text.substr(start, i - start);<br>
<br>
&#9;&#9;&#9;Token tok;<br>
&#9;&#9;&#9;tok.text = ident;<br>
&#9;&#9;&#9;tok.line = line;<br>
&#9;&#9;&#9;if (ident == &quot;class&quot; || ident == &quot;struct&quot;)<br>
&#9;&#9;&#9;&#9;tok.kind = Token::Keyword;<br>
&#9;&#9;&#9;else<br>
&#9;&#9;&#9;&#9;tok.kind = Token::Identifier;<br>
<br>
&#9;&#9;&#9;m_tokens.push_back(std::move(tok));<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// Двойное двоеточие ::<br>
&#9;&#9;if (c == ':' &amp;&amp; i + 1 &lt; n &amp;&amp; text[i + 1] == ':')<br>
&#9;&#9;{<br>
&#9;&#9;&#9;Token tok;<br>
&#9;&#9;&#9;tok.kind = Token::Symbol;<br>
&#9;&#9;&#9;tok.text = &quot;::&quot;;<br>
&#9;&#9;&#9;tok.line = line;<br>
&#9;&#9;&#9;m_tokens.push_back(std::move(tok));<br>
&#9;&#9;&#9;i += 2;<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// Любой пунктуационный символ как отдельный токен<br>
&#9;&#9;if (std::ispunct(static_cast&lt;unsigned char&gt;(c)))<br>
&#9;&#9;{<br>
&#9;&#9;&#9;Token tok;<br>
&#9;&#9;&#9;tok.kind = Token::Symbol;<br>
&#9;&#9;&#9;tok.text = std::string(1, c);<br>
&#9;&#9;&#9;tok.line = line;<br>
&#9;&#9;&#9;m_tokens.push_back(std::move(tok));<br>
&#9;&#9;&#9;++i;<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// На всякий случай: пропускаем остальные символы<br>
&#9;&#9;++i;<br>
&#9;}<br>
}<br>
<br>
bool CppSymbolFinder::find_class_internal(const std::string &amp;class_name,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;ClassRange &amp;out) const<br>
{<br>
&#9;const std::size_t n = m_tokens.size();<br>
<br>
&#9;for (std::size_t i = 0; i &lt; n; ++i)<br>
&#9;{<br>
&#9;&#9;const Token &amp;t = m_tokens[i];<br>
&#9;&#9;if (t.kind != Token::Keyword)<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;if (t.text != &quot;class&quot; &amp;&amp; t.text != &quot;struct&quot;)<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;// Ищем первое имя после class/struct<br>
&#9;&#9;std::size_t j = i + 1;<br>
&#9;&#9;while (j &lt; n &amp;&amp; m_tokens[j].kind != Token::Identifier)<br>
&#9;&#9;&#9;++j;<br>
&#9;&#9;if (j &gt;= n)<br>
&#9;&#9;&#9;break;<br>
<br>
&#9;&#9;const Token &amp;name_tok = m_tokens[j];<br>
&#9;&#9;if (name_tok.text != class_name)<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;// Определение или forward-declaration?<br>
&#9;&#9;// Смотрим только до первой ';' или '{' после имени.<br>
&#9;&#9;std::size_t k = j + 1;<br>
&#9;&#9;bool saw_lbrace = false;<br>
&#9;&#9;for (; k &lt; n; ++k)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;const Token &amp;tk = m_tokens[k];<br>
&#9;&#9;&#9;if (tk.text == &quot;{&quot;)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;saw_lbrace = true;<br>
&#9;&#9;&#9;&#9;break;<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;if (tk.text == &quot;;&quot;)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;// forward-decl: встретили ';' раньше '{'<br>
&#9;&#9;&#9;&#9;break;<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// Если до ';' так и не встретили '{', это forward-declaration.<br>
&#9;&#9;if (!saw_lbrace)<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;// Ищем соответствующую закрывающую '}'<br>
&#9;&#9;int depth = 1;<br>
&#9;&#9;std::size_t body_start = k;<br>
&#9;&#9;std::size_t body_end = k;<br>
<br>
&#9;&#9;for (std::size_t p = k + 1; p &lt; n; ++p)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;const Token &amp;tp = m_tokens[p];<br>
&#9;&#9;&#9;if (tp.text == &quot;{&quot;)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;++depth;<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;else if (tp.text == &quot;}&quot;)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;--depth;<br>
&#9;&#9;&#9;&#9;if (depth == 0)<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;body_end = p;<br>
<br>
&#9;&#9;&#9;&#9;&#9;// Захватываем возможное ';' после '}' (class Foo {...};)<br>
&#9;&#9;&#9;&#9;&#9;std::size_t q = p + 1;<br>
&#9;&#9;&#9;&#9;&#9;if (q &lt; n &amp;&amp; m_tokens[q].text == &quot;;&quot;)<br>
&#9;&#9;&#9;&#9;&#9;&#9;body_end = q;<br>
<br>
&#9;&#9;&#9;&#9;&#9;out.region.start_line = t.line;<br>
&#9;&#9;&#9;&#9;&#9;out.region.end_line = m_tokens[body_end].line;<br>
&#9;&#9;&#9;&#9;&#9;out.body_start_token = body_start;<br>
&#9;&#9;&#9;&#9;&#9;out.body_end_token = body_end;<br>
&#9;&#9;&#9;&#9;&#9;return true;<br>
&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
&#9;}<br>
<br>
&#9;return false;<br>
}<br>
<br>
bool CppSymbolFinder::find_class(const std::string &amp;class_name,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;Region &amp;out) const<br>
{<br>
&#9;ClassRange cr;<br>
&#9;if (!find_class_internal(class_name, cr))<br>
&#9;&#9;return false;<br>
<br>
&#9;out = cr.region;<br>
&#9;return true;<br>
}<br>
<br>
bool CppSymbolFinder::find_method(const std::string &amp;class_name,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;const std::string &amp;method_name,<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;&#9;Region &amp;out) const<br>
{<br>
&#9;ClassRange cr;<br>
&#9;if (!find_class_internal(class_name, cr))<br>
&#9;&#9;return false;<br>
<br>
&#9;const std::size_t begin = cr.body_start_token + 1;<br>
&#9;const std::size_t end = cr.body_end_token;<br>
&#9;const std::size_t n = m_tokens.size();<br>
<br>
&#9;if (begin &gt;= n || begin &gt;= end)<br>
&#9;&#9;return false;<br>
<br>
&#9;// Ищем внутри тела класса<br>
&#9;for (std::size_t i = begin; i &lt; end; ++i)<br>
&#9;{<br>
&#9;&#9;const Token &amp;t = m_tokens[i];<br>
&#9;&#9;if (t.kind != Token::Identifier)<br>
&#9;&#9;&#9;continue;<br>
&#9;&#9;if (t.text != method_name)<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;// После имени должен идти '('<br>
&#9;&#9;std::size_t j = i + 1;<br>
&#9;&#9;if (j &gt;= end || m_tokens[j].text != &quot;(&quot;)<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;// Определяем начало объявления:<br>
&#9;&#9;// идём назад до ближайшего ';', '{' или '}'.<br>
&#9;&#9;std::size_t start_tok = begin;<br>
&#9;&#9;for (std::size_t k = i; k &gt; begin; --k)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;const std::string &amp;s = m_tokens[k].text;<br>
&#9;&#9;&#9;if (s == &quot;;&quot; || s == &quot;{&quot; || s == &quot;}&quot;)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;start_tok = k + 1;<br>
&#9;&#9;&#9;&#9;break;<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
<br>
&#9;&#9;// 1) Пропускаем параметры (балансируем скобки)<br>
&#9;&#9;std::size_t pos = j;<br>
&#9;&#9;int paren_depth = 0;<br>
&#9;&#9;for (; pos &lt; end; ++pos)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;const std::string &amp;s = m_tokens[pos].text;<br>
&#9;&#9;&#9;if (s == &quot;(&quot;)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;++paren_depth;<br>
&#9;&#9;&#9;}<br>
&#9;&#9;&#9;else if (s == &quot;)&quot;)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;--paren_depth;<br>
&#9;&#9;&#9;&#9;if (paren_depth == 0)<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;++pos; // идём на токен после ')'<br>
&#9;&#9;&#9;&#9;&#9;break;<br>
&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
&#9;&#9;if (paren_depth != 0)<br>
&#9;&#9;&#9;continue; // сломанная сигнатура<br>
<br>
&#9;&#9;// 2) Ищем ';' или '{' (начало тела)<br>
&#9;&#9;std::size_t end_tok = start_tok;<br>
&#9;&#9;int brace_depth = 0;<br>
<br>
&#9;&#9;for (; pos &lt; end; ++pos)<br>
&#9;&#9;{<br>
&#9;&#9;&#9;const std::string &amp;s = m_tokens[pos].text;<br>
<br>
&#9;&#9;&#9;if (s == &quot;;&quot;)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;end_tok = pos;<br>
&#9;&#9;&#9;&#9;break;<br>
&#9;&#9;&#9;}<br>
<br>
&#9;&#9;&#9;if (s == &quot;{&quot;)<br>
&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;brace_depth = 1;<br>
&#9;&#9;&#9;&#9;end_tok = pos;<br>
&#9;&#9;&#9;&#9;++pos;<br>
&#9;&#9;&#9;&#9;for (; pos &lt; end; ++pos)<br>
&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;const std::string &amp;sb = m_tokens[pos].text;<br>
&#9;&#9;&#9;&#9;&#9;if (sb == &quot;{&quot;)<br>
&#9;&#9;&#9;&#9;&#9;&#9;++brace_depth;<br>
&#9;&#9;&#9;&#9;&#9;else if (sb == &quot;}&quot;)<br>
&#9;&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;&#9;--brace_depth;<br>
&#9;&#9;&#9;&#9;&#9;&#9;if (brace_depth == 0)<br>
&#9;&#9;&#9;&#9;&#9;&#9;{<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;end_tok = pos;<br>
&#9;&#9;&#9;&#9;&#9;&#9;&#9;break;<br>
&#9;&#9;&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;&#9;}<br>
&#9;&#9;&#9;&#9;break;<br>
&#9;&#9;&#9;}<br>
&#9;&#9;}<br>
<br>
&#9;&#9;if (end_tok &lt; start_tok)<br>
&#9;&#9;&#9;continue;<br>
<br>
&#9;&#9;int start_line_token = m_tokens[start_tok].line;<br>
&#9;&#9;int method_line = t.line;<br>
<br>
&#9;&#9;// Не вываливаемся на строку с access-specifier'ом (public:)<br>
&#9;&#9;out.start_line =<br>
&#9;&#9;&#9;(start_line_token &lt; method_line) ? method_line : start_line_token;<br>
<br>
&#9;&#9;out.end_line = m_tokens[end_tok].line;<br>
&#9;&#9;return true;<br>
&#9;}<br>
<br>
&#9;return false;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
