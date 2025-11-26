<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/parser.cpp</title>
</head>
<body>
<pre><code>
#include &quot;parser.h&quot;
#include &lt;fstream&gt;
#include &lt;stdexcept&gt;
#include &lt;cctype&gt;
#include &quot;rules.h&quot;

namespace fs = std::filesystem;

static inline void trim(std::string&amp; s)
{
    size_t b = s.find_first_not_of(&quot; \t\r\n&quot;);
    if (b == std::string::npos) { s.clear(); return; }
    size_t e = s.find_last_not_of(&quot; \t\r\n&quot;);
    s = s.substr(b, e - b + 1);
}

Config parse_config(const fs::path&amp; path)
{
    Config cfg;

    enum Section { TEXT_RULES, TREE_RULES };
    Section current = TEXT_RULES;

    std::ifstream in(path);
    if (!in.is_open())
        throw std::runtime_error(&quot;Failed to open config file: &quot; + path.string());

    std::string line;
    while (std::getline(in, line))
    {
        trim(line);

        if (line.empty() || line[0] == '#')
            continue;

        if (line == &quot;[TREE]&quot;)
        {
            current = TREE_RULES;
            continue;
        }

        Rule r = Rule::from_string(line);

        if (current == TEXT_RULES)
            cfg.text_rules.push_back(r);
        else
            cfg.tree_rules.push_back(r);
    }

    return cfg;
}

</code></pre>
</body>
</html>
