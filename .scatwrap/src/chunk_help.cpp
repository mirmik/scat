<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>src/chunk_help.cpp</title>
</head>
<body>
<!-- BEGIN SCAT CODE -->
#include &quot;chunk_help.h&quot;<br>
#include &lt;iostream&gt;<br>
<br>
void print_chunk_help()<br>
{<br>
    std::cout<br>
        &lt;&lt; &quot;# Chunk v2 — Change Description Format\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;Chunk v2 is a plain-text format for describing modifications to &quot;<br>
           &quot;source files.\n&quot;<br>
           &quot;A patch consists of multiple sections, each describing a single &quot;<br>
           &quot;operation:\n&quot;<br>
           &quot;line-based edits, text-based edits, or file-level operations.\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;---\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;## 1. Section structure\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;=== file: &lt;path&gt; ===\n&quot;<br>
           &quot;&lt;command&gt;\n&quot;<br>
           &quot;&lt;content...&gt;\n&quot;<br>
           &quot;=END=\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;* &lt;path&gt; — relative file path\n&quot;<br>
           &quot;* Empty lines between sections are allowed\n&quot;<br>
           &quot;* Exactly one command per section\n&quot;<br>
           &quot;* &lt;content&gt; may contain any lines, including empty ones\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;---\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;## 3. Commands (text-based)\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;Two formats are supported for text-based commands:\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;### 3.1 Legacy format (simple)\n&quot;<br>
           &quot;These commands match an exact multi-line marker in the file.\n&quot;<br>
           &quot;Everything before the first `---` is the marker; everything after &quot;<br>
           &quot;it is content.\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;### Insert after marker\n&quot;<br>
           &quot;--- insert-after-text\n&quot;<br>
           &quot;&lt;marker lines...&gt;\n&quot;<br>
           &quot;---\n&quot;<br>
           &quot;&lt;inserted lines...&gt;\n&quot;<br>
           &quot;=END=\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;### Insert before marker\n&quot;<br>
           &quot;--- insert-before-text\n&quot;<br>
           &quot;&lt;marker lines...&gt;\n&quot;<br>
           &quot;---\n&quot;<br>
           &quot;&lt;inserted lines...&gt;\n&quot;<br>
           &quot;=END=\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;### Replace marker\n&quot;<br>
           &quot;--- replace-text\n&quot;<br>
           &quot;&lt;marker lines...&gt;\n&quot;<br>
           &quot;---\n&quot;<br>
           &quot;&lt;new lines...&gt;\n&quot;<br>
           &quot;=END=\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;### Delete marker\n&quot;<br>
           &quot;--- delete-text\n&quot;<br>
           &quot;&lt;marker lines...&gt;\n&quot;<br>
           &quot;---\n&quot;<br>
           &quot;=END=\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;---\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;### 3.2 YAML strict format\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;In YAML mode you can also specify strict context around the &quot;<br>
           &quot;marker:\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;--- replace-text\n&quot;<br>
           &quot;BEFORE:\n&quot;<br>
           &quot;  &lt;lines that must appear immediately above the marker&gt;\n&quot;<br>
           &quot;MARKER:\n&quot;<br>
           &quot;  &lt;marker lines&gt;\n&quot;<br>
           &quot;AFTER:\n&quot;<br>
           &quot;  &lt;lines that must appear immediately below the marker&gt;\n&quot;<br>
           &quot;---\n&quot;<br>
           &quot;&lt;payload lines...&gt;\n&quot;<br>
           &quot;=END=\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;Rules:\n&quot;<br>
           &quot;* YAML mode is enabled only when the first non-empty line after &quot;<br>
           &quot;the command\n&quot;<br>
           &quot;  is one of: `BEFORE:`, `MARKER:`, `AFTER:`.\n&quot;<br>
           &quot;* Matching is strict: BEFORE lines must be directly above the &quot;<br>
           &quot;marker block;\n&quot;<br>
           &quot;  AFTER lines must be directly below it.\n&quot;<br>
           &quot;* Whitespace differences are ignored (lines are trimmed before &quot;<br>
           &quot;comparison).\n&quot;<br>
           &quot;* If BEFORE/AFTER are present, there is no fallback to the first &quot;<br>
           &quot;occurrence\n&quot;<br>
           &quot;  of the marker.\n&quot;<br>
           &quot;* If more than one place matches the strict context, the patch &quot;<br>
           &quot;fails as\n&quot;<br>
           &quot;  ambiguous.\n&quot;<br>
           &quot;* If no place matches the strict context, the patch fails with\n&quot;<br>
           &quot;  \&quot;strict marker context not found\&quot;.\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;---\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;## 4. File-level commands\n&quot;<br>
           &quot;These operations work on the whole file rather than its contents.\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;### Create or overwrite file\n&quot;<br>
           &quot;--- create-file\n&quot;<br>
           &quot;&lt;file content...&gt;\n&quot;<br>
           &quot;=END=\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;### Delete file\n&quot;<br>
           &quot;--- delete-file\n&quot;<br>
           &quot;=END=\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;---\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;## 5. Examples\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;=== file: src/a.cpp ===\n&quot;<br>
           &quot;--- replace 3:4\n&quot;<br>
           &quot;int value = 42;\n&quot;<br>
           &quot;std::cout &lt;&lt; value &lt;&lt; \&quot;\\\\n\&quot;;\n&quot;<br>
           &quot;=END=\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;=== file: src/b.cpp ===\n&quot;<br>
           &quot;--- insert-after 12\n&quot;<br>
           &quot;log_debug(\&quot;checkpoint reached\&quot;);\n&quot;<br>
           &quot;=END=\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;=== file: src/c.cpp ===\n&quot;<br>
           &quot;--- delete 20:25\n&quot;<br>
           &quot;=END=\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;=== file: assets/config.json ===\n&quot;<br>
           &quot;--- create-file\n&quot;<br>
           &quot;{ \&quot;version\&quot;: 1 }\n&quot;<br>
           &quot;=END=\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;=== file: old/temp.txt ===\n&quot;<br>
           &quot;--- delete-file\n&quot;<br>
           &quot;=END=\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;---\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;## Recommended usage\n&quot;<br>
           &quot;* Prefer text-based commands (`*-text`) — they are more stable &quot;<br>
           &quot;when code moves.\n&quot;<br>
           &quot;* Use file-level commands when creating or removing entire files.\n&quot;<br>
           &quot;* Group modifications to multiple files into **one patch file**.\n&quot;<br>
           &quot;\n&quot;<br>
           &quot;*This cheat sheet is in the text for a reason. If you're asked to &quot;<br>
           &quot;write a patch,\n&quot;<br>
           &quot; use the following format: chunk_v2.\n&quot;<br>
           &quot;*Try to strictly follow the rules described in this document, &quot;<br>
           &quot;without making any\n&quot;<br>
           &quot; syntactic errors.\n&quot;<br>
           &quot;*When working with chunks, be careful that commands do not &quot;<br>
           &quot;reference the same\n&quot;<br>
           &quot; text macros. Macros should never overlap.\n&quot;;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
