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
&emsp;// Разбиваем текст на строки (независимо от \r\n / \n)<br>
&emsp;std::string current;<br>
&emsp;current.reserve(80);<br>
<br>
&emsp;for (char c : text)<br>
&emsp;{<br>
&emsp;&emsp;if (c == '\n')<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;m_lines.push_back(current);<br>
&emsp;&emsp;&emsp;current.clear();<br>
&emsp;&emsp;}<br>
&emsp;&emsp;else if (c != '\r')<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;current.push_back(c);<br>
&emsp;&emsp;}<br>
&emsp;}<br>
&emsp;m_lines.push_back(current);<br>
<br>
&emsp;tokenize(text);<br>
}<br>
<br>
bool CppSymbolFinder::is_ident_start(char c)<br>
{<br>
&emsp;unsigned char uc = static_cast&lt;unsigned char&gt;(c);<br>
&emsp;return std::isalpha(uc) || c == '_';<br>
}<br>
<br>
bool CppSymbolFinder::is_ident_char(char c)<br>
{<br>
&emsp;unsigned char uc = static_cast&lt;unsigned char&gt;(c);<br>
&emsp;return std::isalnum(uc) || c == '_';<br>
}<br>
<br>
void CppSymbolFinder::tokenize(const std::string &amp;text)<br>
{<br>
&emsp;m_tokens.clear();<br>
<br>
&emsp;bool in_block_comment = false;<br>
&emsp;bool in_string = false;<br>
&emsp;bool in_char = false;<br>
<br>
&emsp;int line = 0;<br>
&emsp;const std::size_t n = text.size();<br>
<br>
&emsp;for (std::size_t i = 0; i &lt; n;)<br>
&emsp;{<br>
&emsp;&emsp;char c = text[i];<br>
<br>
&emsp;&emsp;if (c == '\n')<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;++line;<br>
&emsp;&emsp;&emsp;++i;<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;if (in_block_comment)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;if (c == '*' &amp;&amp; i + 1 &lt; n &amp;&amp; text[i + 1] == '/')<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;in_block_comment = false;<br>
&emsp;&emsp;&emsp;&emsp;i += 2;<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;++i;<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;if (in_string || in_char)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;if (c == '\\' &amp;&amp; i + 1 &lt; n)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;i += 2; // экранированный символ<br>
&emsp;&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;if (in_string &amp;&amp; c == '&quot;')<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;in_string = false;<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;else if (in_char &amp;&amp; c == '\'')<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;in_char = false;<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;++i;<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// Пробелы<br>
&emsp;&emsp;if (c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v')<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;++i;<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// Препроцессорная директива: пропускаем всю строку<br>
&emsp;&emsp;if (c == '#' &amp;&amp; (i == 0 || text[i - 1] == '\n'))<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;while (i &lt; n &amp;&amp; text[i] != '\n')<br>
&emsp;&emsp;&emsp;&emsp;++i;<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// Однострочный комментарий //<br>
&emsp;&emsp;if (c == '/' &amp;&amp; i + 1 &lt; n &amp;&amp; text[i + 1] == '/')<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;while (i &lt; n &amp;&amp; text[i] != '\n')<br>
&emsp;&emsp;&emsp;&emsp;++i;<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// Блочный комментарий /* ... */<br>
&emsp;&emsp;if (c == '/' &amp;&amp; i + 1 &lt; n &amp;&amp; text[i + 1] == '*')<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;in_block_comment = true;<br>
&emsp;&emsp;&emsp;i += 2;<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// Строковый литерал<br>
&emsp;&emsp;if (c == '&quot;')<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;in_string = true;<br>
&emsp;&emsp;&emsp;++i;<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// Символьный литерал<br>
&emsp;&emsp;if (c == '\'')<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;in_char = true;<br>
&emsp;&emsp;&emsp;++i;<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// Идентификатор / ключевое слово<br>
&emsp;&emsp;if (is_ident_start(c))<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;std::size_t start = i;<br>
&emsp;&emsp;&emsp;++i;<br>
&emsp;&emsp;&emsp;while (i &lt; n &amp;&amp; is_ident_char(text[i]))<br>
&emsp;&emsp;&emsp;&emsp;++i;<br>
<br>
&emsp;&emsp;&emsp;std::string ident = text.substr(start, i - start);<br>
<br>
&emsp;&emsp;&emsp;Token tok;<br>
&emsp;&emsp;&emsp;tok.text = ident;<br>
&emsp;&emsp;&emsp;tok.line = line;<br>
&emsp;&emsp;&emsp;if (ident == &quot;class&quot; || ident == &quot;struct&quot;)<br>
&emsp;&emsp;&emsp;&emsp;tok.kind = Token::Keyword;<br>
&emsp;&emsp;&emsp;else<br>
&emsp;&emsp;&emsp;&emsp;tok.kind = Token::Identifier;<br>
<br>
&emsp;&emsp;&emsp;m_tokens.push_back(std::move(tok));<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// Двойное двоеточие ::<br>
&emsp;&emsp;if (c == ':' &amp;&amp; i + 1 &lt; n &amp;&amp; text[i + 1] == ':')<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;Token tok;<br>
&emsp;&emsp;&emsp;tok.kind = Token::Symbol;<br>
&emsp;&emsp;&emsp;tok.text = &quot;::&quot;;<br>
&emsp;&emsp;&emsp;tok.line = line;<br>
&emsp;&emsp;&emsp;m_tokens.push_back(std::move(tok));<br>
&emsp;&emsp;&emsp;i += 2;<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// Любой пунктуационный символ как отдельный токен<br>
&emsp;&emsp;if (std::ispunct(static_cast&lt;unsigned char&gt;(c)))<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;Token tok;<br>
&emsp;&emsp;&emsp;tok.kind = Token::Symbol;<br>
&emsp;&emsp;&emsp;tok.text = std::string(1, c);<br>
&emsp;&emsp;&emsp;tok.line = line;<br>
&emsp;&emsp;&emsp;m_tokens.push_back(std::move(tok));<br>
&emsp;&emsp;&emsp;++i;<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// На всякий случай: пропускаем остальные символы<br>
&emsp;&emsp;++i;<br>
&emsp;}<br>
}<br>
<br>
bool CppSymbolFinder::find_class_internal(const std::string &amp;class_name,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;ClassRange &amp;out) const<br>
{<br>
&emsp;const std::size_t n = m_tokens.size();<br>
<br>
&emsp;for (std::size_t i = 0; i &lt; n; ++i)<br>
&emsp;{<br>
&emsp;&emsp;const Token &amp;t = m_tokens[i];<br>
&emsp;&emsp;if (t.kind != Token::Keyword)<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;if (t.text != &quot;class&quot; &amp;&amp; t.text != &quot;struct&quot;)<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;// Ищем первое имя после class/struct<br>
&emsp;&emsp;std::size_t j = i + 1;<br>
&emsp;&emsp;while (j &lt; n &amp;&amp; m_tokens[j].kind != Token::Identifier)<br>
&emsp;&emsp;&emsp;++j;<br>
&emsp;&emsp;if (j &gt;= n)<br>
&emsp;&emsp;&emsp;break;<br>
<br>
&emsp;&emsp;const Token &amp;name_tok = m_tokens[j];<br>
&emsp;&emsp;if (name_tok.text != class_name)<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;// Определение или forward-declaration?<br>
&emsp;&emsp;// Смотрим только до первой ';' или '{' после имени.<br>
&emsp;&emsp;std::size_t k = j + 1;<br>
&emsp;&emsp;bool saw_lbrace = false;<br>
&emsp;&emsp;for (; k &lt; n; ++k)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;const Token &amp;tk = m_tokens[k];<br>
&emsp;&emsp;&emsp;if (tk.text == &quot;{&quot;)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;saw_lbrace = true;<br>
&emsp;&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;if (tk.text == &quot;;&quot;)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;// forward-decl: встретили ';' раньше '{'<br>
&emsp;&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// Если до ';' так и не встретили '{', это forward-declaration.<br>
&emsp;&emsp;if (!saw_lbrace)<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;// Ищем соответствующую закрывающую '}'<br>
&emsp;&emsp;int depth = 1;<br>
&emsp;&emsp;std::size_t body_start = k;<br>
&emsp;&emsp;std::size_t body_end = k;<br>
<br>
&emsp;&emsp;for (std::size_t p = k + 1; p &lt; n; ++p)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;const Token &amp;tp = m_tokens[p];<br>
&emsp;&emsp;&emsp;if (tp.text == &quot;{&quot;)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;++depth;<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;else if (tp.text == &quot;}&quot;)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;--depth;<br>
&emsp;&emsp;&emsp;&emsp;if (depth == 0)<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;body_end = p;<br>
<br>
&emsp;&emsp;&emsp;&emsp;&emsp;// Захватываем возможное ';' после '}' (class Foo {...};)<br>
&emsp;&emsp;&emsp;&emsp;&emsp;std::size_t q = p + 1;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;if (q &lt; n &amp;&amp; m_tokens[q].text == &quot;;&quot;)<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;body_end = q;<br>
<br>
&emsp;&emsp;&emsp;&emsp;&emsp;out.region.start_line = t.line;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;out.region.end_line = m_tokens[body_end].line;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;out.body_start_token = body_start;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;out.body_end_token = body_end;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;return true;<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
&emsp;}<br>
<br>
&emsp;return false;<br>
}<br>
<br>
bool CppSymbolFinder::find_class(const std::string &amp;class_name,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Region &amp;out) const<br>
{<br>
&emsp;ClassRange cr;<br>
&emsp;if (!find_class_internal(class_name, cr))<br>
&emsp;&emsp;return false;<br>
<br>
&emsp;out = cr.region;<br>
&emsp;return true;<br>
}<br>
<br>
bool CppSymbolFinder::find_method(const std::string &amp;class_name,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;const std::string &amp;method_name,<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;Region &amp;out) const<br>
{<br>
&emsp;ClassRange cr;<br>
&emsp;if (!find_class_internal(class_name, cr))<br>
&emsp;&emsp;return false;<br>
<br>
&emsp;const std::size_t begin = cr.body_start_token + 1;<br>
&emsp;const std::size_t end = cr.body_end_token;<br>
&emsp;const std::size_t n = m_tokens.size();<br>
<br>
&emsp;if (begin &gt;= n || begin &gt;= end)<br>
&emsp;&emsp;return false;<br>
<br>
&emsp;// Ищем внутри тела класса<br>
&emsp;for (std::size_t i = begin; i &lt; end; ++i)<br>
&emsp;{<br>
&emsp;&emsp;const Token &amp;t = m_tokens[i];<br>
&emsp;&emsp;if (t.kind != Token::Identifier)<br>
&emsp;&emsp;&emsp;continue;<br>
&emsp;&emsp;if (t.text != method_name)<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;// После имени должен идти '('<br>
&emsp;&emsp;std::size_t j = i + 1;<br>
&emsp;&emsp;if (j &gt;= end || m_tokens[j].text != &quot;(&quot;)<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;// Определяем начало объявления:<br>
&emsp;&emsp;// идём назад до ближайшего ';', '{' или '}'.<br>
&emsp;&emsp;std::size_t start_tok = begin;<br>
&emsp;&emsp;for (std::size_t k = i; k &gt; begin; --k)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;const std::string &amp;s = m_tokens[k].text;<br>
&emsp;&emsp;&emsp;if (s == &quot;;&quot; || s == &quot;{&quot; || s == &quot;}&quot;)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;start_tok = k + 1;<br>
&emsp;&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;// 1) Пропускаем параметры (балансируем скобки)<br>
&emsp;&emsp;std::size_t pos = j;<br>
&emsp;&emsp;int paren_depth = 0;<br>
&emsp;&emsp;for (; pos &lt; end; ++pos)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;const std::string &amp;s = m_tokens[pos].text;<br>
&emsp;&emsp;&emsp;if (s == &quot;(&quot;)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;++paren_depth;<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;else if (s == &quot;)&quot;)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;--paren_depth;<br>
&emsp;&emsp;&emsp;&emsp;if (paren_depth == 0)<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;++pos; // идём на токен после ')'<br>
&emsp;&emsp;&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
&emsp;&emsp;if (paren_depth != 0)<br>
&emsp;&emsp;&emsp;continue; // сломанная сигнатура<br>
<br>
&emsp;&emsp;// 2) Ищем ';' или '{' (начало тела)<br>
&emsp;&emsp;std::size_t end_tok = start_tok;<br>
&emsp;&emsp;int brace_depth = 0;<br>
<br>
&emsp;&emsp;for (; pos &lt; end; ++pos)<br>
&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;const std::string &amp;s = m_tokens[pos].text;<br>
<br>
&emsp;&emsp;&emsp;if (s == &quot;;&quot;)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;end_tok = pos;<br>
&emsp;&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;&emsp;}<br>
<br>
&emsp;&emsp;&emsp;if (s == &quot;{&quot;)<br>
&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;brace_depth = 1;<br>
&emsp;&emsp;&emsp;&emsp;end_tok = pos;<br>
&emsp;&emsp;&emsp;&emsp;++pos;<br>
&emsp;&emsp;&emsp;&emsp;for (; pos &lt; end; ++pos)<br>
&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;const std::string &amp;sb = m_tokens[pos].text;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;if (sb == &quot;{&quot;)<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;++brace_depth;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;else if (sb == &quot;}&quot;)<br>
&emsp;&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;--brace_depth;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;if (brace_depth == 0)<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;{<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;end_tok = pos;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;&emsp;&emsp;break;<br>
&emsp;&emsp;&emsp;}<br>
&emsp;&emsp;}<br>
<br>
&emsp;&emsp;if (end_tok &lt; start_tok)<br>
&emsp;&emsp;&emsp;continue;<br>
<br>
&emsp;&emsp;int start_line_token = m_tokens[start_tok].line;<br>
&emsp;&emsp;int method_line = t.line;<br>
<br>
&emsp;&emsp;// Не вываливаемся на строку с access-specifier'ом (public:)<br>
&emsp;&emsp;out.start_line =<br>
&emsp;&emsp;&emsp;(start_line_token &lt; method_line) ? method_line : start_line_token;<br>
<br>
&emsp;&emsp;out.end_line = m_tokens[end_tok].line;<br>
&emsp;&emsp;return true;<br>
&emsp;}<br>
<br>
&emsp;return false;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
