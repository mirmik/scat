<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/git_info.h</title>
</head>
<body>
<pre><code>
#pragma once

#include &lt;string&gt;

struct GitInfo {
    std::string commit;
    std::string remote;
    bool has_commit = false;
    bool has_remote = false;
};

// Tries to detect git commit hash and remote origin URL
GitInfo detect_git_info();

</code></pre>
</body>
</html>
