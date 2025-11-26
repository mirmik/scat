<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/chunk_help.cpp</title>
</head>
<body>
<pre><code>
#include &lt;iostream&gt;
#include &quot;chunk_help.h&quot;

void print_chunk_help()
{
    std::cout &lt;&lt; &quot;# Chunk v2 — Change Description Format\n&quot;
                 &quot;\n&quot;
                 &quot;Chunk v2 is a plain-text format for describing modifications to source files.\n&quot;
                 &quot;A patch consists of multiple sections, each describing a single operation:\n&quot;
                 &quot;line-based edits, text-based edits, or file-level operations.\n&quot;
                 &quot;\n&quot;
                 &quot;---\n&quot;
                 &quot;\n&quot;
                 &quot;## 1. Section structure\n&quot;
                 &quot;\n&quot;
                 &quot;=== file: &lt;path&gt; ===\n&quot;
                 &quot;&lt;command&gt;\n&quot;
                 &quot;&lt;content...&gt;\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;* &lt;path&gt; — relative file path\n&quot;
                 &quot;* Empty lines between sections are allowed\n&quot;
                 &quot;* Exactly one command per section\n&quot;
                 &quot;* &lt;content&gt; may contain any lines, including empty ones\n&quot;
                 &quot;\n&quot;
                 &quot;---\n&quot;
                 &quot;\n&quot;
                 &quot;## 3. Commands (text-based)\n&quot;
                 &quot;\n&quot;
                 &quot;Two formats are supported for text-based commands:\n&quot;
                 &quot;\n&quot;
                 &quot;### 3.1 Legacy format (simple)\n&quot;
                 &quot;These commands match an exact multi-line marker in the file.\n&quot;
                 &quot;Everything before the first `---` is the marker; everything after it is content.\n&quot;
                 &quot;\n&quot;
                 &quot;### Insert after marker\n&quot;
                 &quot;--- insert-after-text\n&quot;
                 &quot;&lt;marker lines...&gt;\n&quot;
                 &quot;---\n&quot;
                 &quot;&lt;inserted lines...&gt;\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;### Insert before marker\n&quot;
                 &quot;--- insert-before-text\n&quot;
                 &quot;&lt;marker lines...&gt;\n&quot;
                 &quot;---\n&quot;
                 &quot;&lt;inserted lines...&gt;\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;### Replace marker\n&quot;
                 &quot;--- replace-text\n&quot;
                 &quot;&lt;marker lines...&gt;\n&quot;
                 &quot;---\n&quot;
                 &quot;&lt;new lines...&gt;\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;### Delete marker\n&quot;
                 &quot;--- delete-text\n&quot;
                 &quot;&lt;marker lines...&gt;\n&quot;
                 &quot;---\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;---\n&quot;
                 &quot;\n&quot;
                 &quot;### 3.2 YAML strict format\n&quot;
                 &quot;\n&quot;
                 &quot;In YAML mode you can also specify strict context around the marker:\n&quot;
                 &quot;\n&quot;
                 &quot;--- replace-text\n&quot;
                 &quot;BEFORE:\n&quot;
                 &quot;  &lt;lines that must appear immediately above the marker&gt;\n&quot;
                 &quot;MARKER:\n&quot;
                 &quot;  &lt;marker lines&gt;\n&quot;
                 &quot;AFTER:\n&quot;
                 &quot;  &lt;lines that must appear immediately below the marker&gt;\n&quot;
                 &quot;---\n&quot;
                 &quot;&lt;payload lines...&gt;\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;Rules:\n&quot;
                 &quot;* YAML mode is enabled only when the first non-empty line after the command\n&quot;
                 &quot;  is one of: `BEFORE:`, `MARKER:`, `AFTER:`.\n&quot;
                 &quot;* Matching is strict: BEFORE lines must be directly above the marker block;\n&quot;
                 &quot;  AFTER lines must be directly below it.\n&quot;
                 &quot;* Whitespace differences are ignored (lines are trimmed before comparison).\n&quot;
                 &quot;* If BEFORE/AFTER are present, there is no fallback to the first occurrence\n&quot;
                 &quot;  of the marker.\n&quot;
                 &quot;* If more than one place matches the strict context, the patch fails as\n&quot;
                 &quot;  ambiguous.\n&quot;
                 &quot;* If no place matches the strict context, the patch fails with\n&quot;
                 &quot;  \&quot;strict marker context not found\&quot;.\n&quot;
                 &quot;\n&quot;
                 &quot;---\n&quot;
                 &quot;\n&quot;
                 &quot;## 4. File-level commands\n&quot;
                 &quot;These operations work on the whole file rather than its contents.\n&quot;
                 &quot;\n&quot;
                 &quot;### Create or overwrite file\n&quot;
                 &quot;--- create-file\n&quot;
                 &quot;&lt;file content...&gt;\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;### Delete file\n&quot;
                 &quot;--- delete-file\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;---\n&quot;
                 &quot;\n&quot;
                 &quot;## 5. Examples\n&quot;
                 &quot;\n&quot;
                 &quot;=== file: src/a.cpp ===\n&quot;
                 &quot;--- replace 3:4\n&quot;
                 &quot;int value = 42;\n&quot;
                 &quot;std::cout &lt;&lt; value &lt;&lt; \&quot;\\\\n\&quot;;\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;=== file: src/b.cpp ===\n&quot;
                 &quot;--- insert-after 12\n&quot;
                 &quot;log_debug(\&quot;checkpoint reached\&quot;);\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;=== file: src/c.cpp ===\n&quot;
                 &quot;--- delete 20:25\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;=== file: assets/config.json ===\n&quot;
                 &quot;--- create-file\n&quot;
                 &quot;{ \&quot;version\&quot;: 1 }\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;=== file: old/temp.txt ===\n&quot;
                 &quot;--- delete-file\n&quot;
                 &quot;=END=\n&quot;
                 &quot;\n&quot;
                 &quot;---\n&quot;
                 &quot;\n&quot;
                 &quot;## Recommended usage\n&quot;
                 &quot;* Prefer text-based commands (`*-text`) — they are more stable when code moves.\n&quot;
                 &quot;* Use file-level commands when creating or removing entire files.\n&quot;
                 &quot;* Group modifications to multiple files into **one patch file**.\n&quot;
                 &quot;\n&quot;
                 &quot;*This cheat sheet is in the text for a reason. If you're asked to write a patch,\n&quot;
                 &quot; use the following format: chunk_v2.\n&quot;
                 &quot;*Try to strictly follow the rules described in this document, without making any\n&quot;
                 &quot; syntactic errors.\n&quot;
                 &quot;*When working with chunks, be careful that commands do not reference the same\n&quot;
                 &quot; text macros. Macros should never overlap.\n&quot;;
}
</code></pre>
</body>
</html>
