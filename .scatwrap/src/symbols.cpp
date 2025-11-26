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
    // Разбиваем текст на строки (независимо от \r\n / \n)<br>
    std::string current;<br>
    current.reserve(80);<br>
<br>
    for (char c : text)<br>
    {<br>
        if (c == '\n')<br>
        {<br>
            m_lines.push_back(current);<br>
            current.clear();<br>
        }<br>
        else if (c != '\r')<br>
        {<br>
            current.push_back(c);<br>
        }<br>
    }<br>
    m_lines.push_back(current);<br>
<br>
    tokenize(text);<br>
}<br>
<br>
bool CppSymbolFinder::is_ident_start(char c)<br>
{<br>
    unsigned char uc = static_cast&lt;unsigned char&gt;(c);<br>
    return std::isalpha(uc) || c == '_';<br>
}<br>
<br>
bool CppSymbolFinder::is_ident_char(char c)<br>
{<br>
    unsigned char uc = static_cast&lt;unsigned char&gt;(c);<br>
    return std::isalnum(uc) || c == '_';<br>
}<br>
<br>
void CppSymbolFinder::tokenize(const std::string &amp;text)<br>
{<br>
    m_tokens.clear();<br>
<br>
    bool in_block_comment = false;<br>
    bool in_string = false;<br>
    bool in_char = false;<br>
<br>
    int line = 0;<br>
    const std::size_t n = text.size();<br>
<br>
    for (std::size_t i = 0; i &lt; n;)<br>
    {<br>
        char c = text[i];<br>
<br>
        if (c == '\n')<br>
        {<br>
            ++line;<br>
            ++i;<br>
            continue;<br>
        }<br>
<br>
        if (in_block_comment)<br>
        {<br>
            if (c == '*' &amp;&amp; i + 1 &lt; n &amp;&amp; text[i + 1] == '/')<br>
            {<br>
                in_block_comment = false;<br>
                i += 2;<br>
            }<br>
            else<br>
            {<br>
                ++i;<br>
            }<br>
            continue;<br>
        }<br>
<br>
        if (in_string || in_char)<br>
        {<br>
            if (c == '\\' &amp;&amp; i + 1 &lt; n)<br>
            {<br>
                i += 2; // экранированный символ<br>
                continue;<br>
            }<br>
            if (in_string &amp;&amp; c == '&quot;')<br>
            {<br>
                in_string = false;<br>
            }<br>
            else if (in_char &amp;&amp; c == '\'')<br>
            {<br>
                in_char = false;<br>
            }<br>
            ++i;<br>
            continue;<br>
        }<br>
<br>
        // Пробелы<br>
        if (c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v')<br>
        {<br>
            ++i;<br>
            continue;<br>
        }<br>
<br>
        // Препроцессорная директива: пропускаем всю строку<br>
        if (c == '#' &amp;&amp; (i == 0 || text[i - 1] == '\n'))<br>
        {<br>
            while (i &lt; n &amp;&amp; text[i] != '\n')<br>
                ++i;<br>
            continue;<br>
        }<br>
<br>
        // Однострочный комментарий //<br>
        if (c == '/' &amp;&amp; i + 1 &lt; n &amp;&amp; text[i + 1] == '/')<br>
        {<br>
            while (i &lt; n &amp;&amp; text[i] != '\n')<br>
                ++i;<br>
            continue;<br>
        }<br>
<br>
        // Блочный комментарий /* ... */<br>
        if (c == '/' &amp;&amp; i + 1 &lt; n &amp;&amp; text[i + 1] == '*')<br>
        {<br>
            in_block_comment = true;<br>
            i += 2;<br>
            continue;<br>
        }<br>
<br>
        // Строковый литерал<br>
        if (c == '&quot;')<br>
        {<br>
            in_string = true;<br>
            ++i;<br>
            continue;<br>
        }<br>
<br>
        // Символьный литерал<br>
        if (c == '\'')<br>
        {<br>
            in_char = true;<br>
            ++i;<br>
            continue;<br>
        }<br>
<br>
        // Идентификатор / ключевое слово<br>
        if (is_ident_start(c))<br>
        {<br>
            std::size_t start = i;<br>
            ++i;<br>
            while (i &lt; n &amp;&amp; is_ident_char(text[i]))<br>
                ++i;<br>
<br>
            std::string ident = text.substr(start, i - start);<br>
<br>
            Token tok;<br>
            tok.text = ident;<br>
            tok.line = line;<br>
            if (ident == &quot;class&quot; || ident == &quot;struct&quot;)<br>
                tok.kind = Token::Keyword;<br>
            else<br>
                tok.kind = Token::Identifier;<br>
<br>
            m_tokens.push_back(std::move(tok));<br>
            continue;<br>
        }<br>
<br>
        // Двойное двоеточие ::<br>
        if (c == ':' &amp;&amp; i + 1 &lt; n &amp;&amp; text[i + 1] == ':')<br>
        {<br>
            Token tok;<br>
            tok.kind = Token::Symbol;<br>
            tok.text = &quot;::&quot;;<br>
            tok.line = line;<br>
            m_tokens.push_back(std::move(tok));<br>
            i += 2;<br>
            continue;<br>
        }<br>
<br>
        // Любой пунктуационный символ как отдельный токен<br>
        if (std::ispunct(static_cast&lt;unsigned char&gt;(c)))<br>
        {<br>
            Token tok;<br>
            tok.kind = Token::Symbol;<br>
            tok.text = std::string(1, c);<br>
            tok.line = line;<br>
            m_tokens.push_back(std::move(tok));<br>
            ++i;<br>
            continue;<br>
        }<br>
<br>
        // На всякий случай: пропускаем остальные символы<br>
        ++i;<br>
    }<br>
}<br>
<br>
bool CppSymbolFinder::find_class_internal(const std::string &amp;class_name,<br>
                                          ClassRange &amp;out) const<br>
{<br>
    const std::size_t n = m_tokens.size();<br>
<br>
    for (std::size_t i = 0; i &lt; n; ++i)<br>
    {<br>
        const Token &amp;t = m_tokens[i];<br>
        if (t.kind != Token::Keyword)<br>
            continue;<br>
        if (t.text != &quot;class&quot; &amp;&amp; t.text != &quot;struct&quot;)<br>
            continue;<br>
<br>
        // Ищем первое имя после class/struct<br>
        std::size_t j = i + 1;<br>
        while (j &lt; n &amp;&amp; m_tokens[j].kind != Token::Identifier)<br>
            ++j;<br>
        if (j &gt;= n)<br>
            break;<br>
<br>
        const Token &amp;name_tok = m_tokens[j];<br>
        if (name_tok.text != class_name)<br>
            continue;<br>
<br>
        // Определение или forward-declaration?<br>
        // Смотрим только до первой ';' или '{' после имени.<br>
        std::size_t k = j + 1;<br>
        bool saw_lbrace = false;<br>
        for (; k &lt; n; ++k)<br>
        {<br>
            const Token &amp;tk = m_tokens[k];<br>
            if (tk.text == &quot;{&quot;)<br>
            {<br>
                saw_lbrace = true;<br>
                break;<br>
            }<br>
            if (tk.text == &quot;;&quot;)<br>
            {<br>
                // forward-decl: встретили ';' раньше '{'<br>
                break;<br>
            }<br>
        }<br>
<br>
        // Если до ';' так и не встретили '{', это forward-declaration.<br>
        if (!saw_lbrace)<br>
            continue;<br>
<br>
        // Ищем соответствующую закрывающую '}'<br>
        int depth = 1;<br>
        std::size_t body_start = k;<br>
        std::size_t body_end = k;<br>
<br>
        for (std::size_t p = k + 1; p &lt; n; ++p)<br>
        {<br>
            const Token &amp;tp = m_tokens[p];<br>
            if (tp.text == &quot;{&quot;)<br>
            {<br>
                ++depth;<br>
            }<br>
            else if (tp.text == &quot;}&quot;)<br>
            {<br>
                --depth;<br>
                if (depth == 0)<br>
                {<br>
                    body_end = p;<br>
<br>
                    // Захватываем возможное ';' после '}' (class Foo {...};)<br>
                    std::size_t q = p + 1;<br>
                    if (q &lt; n &amp;&amp; m_tokens[q].text == &quot;;&quot;)<br>
                        body_end = q;<br>
<br>
                    out.region.start_line = t.line;<br>
                    out.region.end_line = m_tokens[body_end].line;<br>
                    out.body_start_token = body_start;<br>
                    out.body_end_token = body_end;<br>
                    return true;<br>
                }<br>
            }<br>
        }<br>
    }<br>
<br>
    return false;<br>
}<br>
<br>
bool CppSymbolFinder::find_class(const std::string &amp;class_name,<br>
                                 Region &amp;out) const<br>
{<br>
    ClassRange cr;<br>
    if (!find_class_internal(class_name, cr))<br>
        return false;<br>
<br>
    out = cr.region;<br>
    return true;<br>
}<br>
<br>
bool CppSymbolFinder::find_method(const std::string &amp;class_name,<br>
                                  const std::string &amp;method_name,<br>
                                  Region &amp;out) const<br>
{<br>
    ClassRange cr;<br>
    if (!find_class_internal(class_name, cr))<br>
        return false;<br>
<br>
    const std::size_t begin = cr.body_start_token + 1;<br>
    const std::size_t end = cr.body_end_token;<br>
    const std::size_t n = m_tokens.size();<br>
<br>
    if (begin &gt;= n || begin &gt;= end)<br>
        return false;<br>
<br>
    // Ищем внутри тела класса<br>
    for (std::size_t i = begin; i &lt; end; ++i)<br>
    {<br>
        const Token &amp;t = m_tokens[i];<br>
        if (t.kind != Token::Identifier)<br>
            continue;<br>
        if (t.text != method_name)<br>
            continue;<br>
<br>
        // После имени должен идти '('<br>
        std::size_t j = i + 1;<br>
        if (j &gt;= end || m_tokens[j].text != &quot;(&quot;)<br>
            continue;<br>
<br>
        // Определяем начало объявления:<br>
        // идём назад до ближайшего ';', '{' или '}'.<br>
        std::size_t start_tok = begin;<br>
        for (std::size_t k = i; k &gt; begin; --k)<br>
        {<br>
            const std::string &amp;s = m_tokens[k].text;<br>
            if (s == &quot;;&quot; || s == &quot;{&quot; || s == &quot;}&quot;)<br>
            {<br>
                start_tok = k + 1;<br>
                break;<br>
            }<br>
        }<br>
<br>
        // 1) Пропускаем параметры (балансируем скобки)<br>
        std::size_t pos = j;<br>
        int paren_depth = 0;<br>
        for (; pos &lt; end; ++pos)<br>
        {<br>
            const std::string &amp;s = m_tokens[pos].text;<br>
            if (s == &quot;(&quot;)<br>
            {<br>
                ++paren_depth;<br>
            }<br>
            else if (s == &quot;)&quot;)<br>
            {<br>
                --paren_depth;<br>
                if (paren_depth == 0)<br>
                {<br>
                    ++pos; // идём на токен после ')'<br>
                    break;<br>
                }<br>
            }<br>
        }<br>
        if (paren_depth != 0)<br>
            continue; // сломанная сигнатура<br>
<br>
        // 2) Ищем ';' или '{' (начало тела)<br>
        std::size_t end_tok = start_tok;<br>
        int brace_depth = 0;<br>
<br>
        for (; pos &lt; end; ++pos)<br>
        {<br>
            const std::string &amp;s = m_tokens[pos].text;<br>
<br>
            if (s == &quot;;&quot;)<br>
            {<br>
                end_tok = pos;<br>
                break;<br>
            }<br>
<br>
            if (s == &quot;{&quot;)<br>
            {<br>
                brace_depth = 1;<br>
                end_tok = pos;<br>
                ++pos;<br>
                for (; pos &lt; end; ++pos)<br>
                {<br>
                    const std::string &amp;sb = m_tokens[pos].text;<br>
                    if (sb == &quot;{&quot;)<br>
                        ++brace_depth;<br>
                    else if (sb == &quot;}&quot;)<br>
                    {<br>
                        --brace_depth;<br>
                        if (brace_depth == 0)<br>
                        {<br>
                            end_tok = pos;<br>
                            break;<br>
                        }<br>
                    }<br>
                }<br>
                break;<br>
            }<br>
        }<br>
<br>
        if (end_tok &lt; start_tok)<br>
            continue;<br>
<br>
        int start_line_token = m_tokens[start_tok].line;<br>
        int method_line = t.line;<br>
<br>
        // Не вываливаемся на строку с access-specifier'ом (public:)<br>
        out.start_line =<br>
            (start_line_token &lt; method_line) ? method_line : start_line_token;<br>
<br>
        out.end_line = m_tokens[end_tok].line;<br>
        return true;<br>
    }<br>
<br>
    return false;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
