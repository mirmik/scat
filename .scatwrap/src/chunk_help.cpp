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
&#9;std::cout<br>
&#9;&#9;&lt;&lt; &quot;# Chunk v2 — Change Description Format\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;Chunk v2 is a plain-text format for describing modifications to &quot;<br>
&#9;&#9;&quot;source files.\n&quot;<br>
&#9;&#9;&quot;A patch consists of multiple sections, each describing a single &quot;<br>
&#9;&#9;&quot;operation:\n&quot;<br>
&#9;&#9;&quot;line-based edits, text-based edits, or file-level operations.\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;## 1. Section structure\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;=== file: &lt;path&gt; ===\n&quot;<br>
&#9;&#9;&quot;&lt;command&gt;\n&quot;<br>
&#9;&#9;&quot;&lt;content...&gt;\n&quot;<br>
&#9;&#9;&quot;=END=\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;* &lt;path&gt; — relative file path\n&quot;<br>
&#9;&#9;&quot;* Empty lines between sections are allowed\n&quot;<br>
&#9;&#9;&quot;* Exactly one command per section\n&quot;<br>
&#9;&#9;&quot;* &lt;content&gt; may contain any lines, including empty ones\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;## 3. Commands (text-based)\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;Two formats are supported for text-based commands:\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;### 3.1 Legacy format (simple)\n&quot;<br>
&#9;&#9;&quot;These commands match an exact multi-line marker in the file.\n&quot;<br>
&#9;&#9;&quot;Everything before the first `---` is the marker; everything after &quot;<br>
&#9;&#9;&quot;it is content.\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;### Insert after marker\n&quot;<br>
&#9;&#9;&quot;--- insert-after-text\n&quot;<br>
&#9;&#9;&quot;&lt;marker lines...&gt;\n&quot;<br>
&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&quot;&lt;inserted lines...&gt;\n&quot;<br>
&#9;&#9;&quot;=END=\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;### Insert before marker\n&quot;<br>
&#9;&#9;&quot;--- insert-before-text\n&quot;<br>
&#9;&#9;&quot;&lt;marker lines...&gt;\n&quot;<br>
&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&quot;&lt;inserted lines...&gt;\n&quot;<br>
&#9;&#9;&quot;=END=\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;### Replace marker\n&quot;<br>
&#9;&#9;&quot;--- replace-text\n&quot;<br>
&#9;&#9;&quot;&lt;marker lines...&gt;\n&quot;<br>
&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&quot;&lt;new lines...&gt;\n&quot;<br>
&#9;&#9;&quot;=END=\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;### Delete marker\n&quot;<br>
&#9;&#9;&quot;--- delete-text\n&quot;<br>
&#9;&#9;&quot;&lt;marker lines...&gt;\n&quot;<br>
&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&quot;=END=\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;### 3.2 YAML strict format\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;In YAML mode you can also specify strict context around the &quot;<br>
&#9;&#9;&quot;marker:\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;--- replace-text\n&quot;<br>
&#9;&#9;&quot;BEFORE:\n&quot;<br>
&#9;&#9;&quot;  &lt;lines that must appear immediately above the marker&gt;\n&quot;<br>
&#9;&#9;&quot;MARKER:\n&quot;<br>
&#9;&#9;&quot;  &lt;marker lines&gt;\n&quot;<br>
&#9;&#9;&quot;AFTER:\n&quot;<br>
&#9;&#9;&quot;  &lt;lines that must appear immediately below the marker&gt;\n&quot;<br>
&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&quot;&lt;payload lines...&gt;\n&quot;<br>
&#9;&#9;&quot;=END=\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;Rules:\n&quot;<br>
&#9;&#9;&quot;* YAML mode is enabled only when the first non-empty line after &quot;<br>
&#9;&#9;&quot;the command\n&quot;<br>
&#9;&#9;&quot;  is one of: `BEFORE:`, `MARKER:`, `AFTER:`.\n&quot;<br>
&#9;&#9;&quot;* Matching is strict: BEFORE lines must be directly above the &quot;<br>
&#9;&#9;&quot;marker block;\n&quot;<br>
&#9;&#9;&quot;  AFTER lines must be directly below it.\n&quot;<br>
&#9;&#9;&quot;* Whitespace differences are ignored (lines are trimmed before &quot;<br>
&#9;&#9;&quot;comparison).\n&quot;<br>
&#9;&#9;&quot;* If BEFORE/AFTER are present, there is no fallback to the first &quot;<br>
&#9;&#9;&quot;occurrence\n&quot;<br>
&#9;&#9;&quot;  of the marker.\n&quot;<br>
&#9;&#9;&quot;* If more than one place matches the strict context, the patch &quot;<br>
&#9;&#9;&quot;fails as\n&quot;<br>
&#9;&#9;&quot;  ambiguous.\n&quot;<br>
&#9;&#9;&quot;* If no place matches the strict context, the patch fails with\n&quot;<br>
&#9;&#9;&quot;  \&quot;strict marker context not found\&quot;.\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;## 4. File-level commands\n&quot;<br>
&#9;&#9;&quot;These operations work on the whole file rather than its contents.\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;### Create or overwrite file\n&quot;<br>
&#9;&#9;&quot;--- create-file\n&quot;<br>
&#9;&#9;&quot;&lt;file content...&gt;\n&quot;<br>
&#9;&#9;&quot;=END=\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;### Delete file\n&quot;<br>
&#9;&#9;&quot;--- delete-file\n&quot;<br>
&#9;&#9;&quot;=END=\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;## 5. Examples\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;=== file: src/a.cpp ===\n&quot;<br>
&#9;&#9;&quot;--- replace 3:4\n&quot;<br>
&#9;&#9;&quot;int value = 42;\n&quot;<br>
&#9;&#9;&quot;std::cout &lt;&lt; value &lt;&lt; \&quot;\\\\n\&quot;;\n&quot;<br>
&#9;&#9;&quot;=END=\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;=== file: src/b.cpp ===\n&quot;<br>
&#9;&#9;&quot;--- insert-after 12\n&quot;<br>
&#9;&#9;&quot;log_debug(\&quot;checkpoint reached\&quot;);\n&quot;<br>
&#9;&#9;&quot;=END=\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;=== file: src/c.cpp ===\n&quot;<br>
&#9;&#9;&quot;--- delete 20:25\n&quot;<br>
&#9;&#9;&quot;=END=\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;=== file: assets/config.json ===\n&quot;<br>
&#9;&#9;&quot;--- create-file\n&quot;<br>
&#9;&#9;&quot;{ \&quot;version\&quot;: 1 }\n&quot;<br>
&#9;&#9;&quot;=END=\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;=== file: old/temp.txt ===\n&quot;<br>
&#9;&#9;&quot;--- delete-file\n&quot;<br>
&#9;&#9;&quot;=END=\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;---\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;## Recommended usage\n&quot;<br>
&#9;&#9;&quot;* Prefer text-based commands (`*-text`) — they are more stable &quot;<br>
&#9;&#9;&quot;when code moves.\n&quot;<br>
&#9;&#9;&quot;* Use file-level commands when creating or removing entire files.\n&quot;<br>
&#9;&#9;&quot;* Group modifications to multiple files into **one patch file**.\n&quot;<br>
&#9;&#9;&quot;\n&quot;<br>
&#9;&#9;&quot;*This cheat sheet is in the text for a reason. If you're asked to &quot;<br>
&#9;&#9;&quot;write a patch,\n&quot;<br>
&#9;&#9;&quot; use the following format: chunk_v2.\n&quot;<br>
&#9;&#9;&quot;*Try to strictly follow the rules described in this document, &quot;<br>
&#9;&#9;&quot;without making any\n&quot;<br>
&#9;&#9;&quot; syntactic errors.\n&quot;<br>
&#9;&#9;&quot;*When working with chunks, be careful that commands do not &quot;<br>
&#9;&#9;&quot;reference the same\n&quot;<br>
&#9;&#9;&quot; text macros. Macros should never overlap.\n&quot;;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
