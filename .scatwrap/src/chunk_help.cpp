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
&emsp;std::cout<br>
&emsp;&emsp;&lt;&lt; &quot;# Chunk v2 — Change Description Format\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;Chunk v2 is a plain-text format for describing modifications to &quot;<br>
&emsp;&emsp;&quot;source files.\n&quot;<br>
&emsp;&emsp;&quot;A patch consists of multiple sections, each describing a single &quot;<br>
&emsp;&emsp;&quot;operation:\n&quot;<br>
&emsp;&emsp;&quot;line-based edits, text-based edits, or file-level operations.\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;## 1. Section structure\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;=== file: &lt;path&gt; ===\n&quot;<br>
&emsp;&emsp;&quot;&lt;command&gt;\n&quot;<br>
&emsp;&emsp;&quot;&lt;content...&gt;\n&quot;<br>
&emsp;&emsp;&quot;=END=\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;* &lt;path&gt; — relative file path\n&quot;<br>
&emsp;&emsp;&quot;* Empty lines between sections are allowed\n&quot;<br>
&emsp;&emsp;&quot;* Exactly one command per section\n&quot;<br>
&emsp;&emsp;&quot;* &lt;content&gt; may contain any lines, including empty ones\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;## 3. Commands (text-based)\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;Two formats are supported for text-based commands:\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;### 3.1 Legacy format (simple)\n&quot;<br>
&emsp;&emsp;&quot;These commands match an exact multi-line marker in the file.\n&quot;<br>
&emsp;&emsp;&quot;Everything before the first `---` is the marker; everything after &quot;<br>
&emsp;&emsp;&quot;it is content.\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;### Insert after marker\n&quot;<br>
&emsp;&emsp;&quot;--- insert-after-text\n&quot;<br>
&emsp;&emsp;&quot;&lt;marker lines...&gt;\n&quot;<br>
&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&quot;&lt;inserted lines...&gt;\n&quot;<br>
&emsp;&emsp;&quot;=END=\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;### Insert before marker\n&quot;<br>
&emsp;&emsp;&quot;--- insert-before-text\n&quot;<br>
&emsp;&emsp;&quot;&lt;marker lines...&gt;\n&quot;<br>
&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&quot;&lt;inserted lines...&gt;\n&quot;<br>
&emsp;&emsp;&quot;=END=\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;### Replace marker\n&quot;<br>
&emsp;&emsp;&quot;--- replace-text\n&quot;<br>
&emsp;&emsp;&quot;&lt;marker lines...&gt;\n&quot;<br>
&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&quot;&lt;new lines...&gt;\n&quot;<br>
&emsp;&emsp;&quot;=END=\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;### Delete marker\n&quot;<br>
&emsp;&emsp;&quot;--- delete-text\n&quot;<br>
&emsp;&emsp;&quot;&lt;marker lines...&gt;\n&quot;<br>
&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&quot;=END=\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;### 3.2 YAML strict format\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;In YAML mode you can also specify strict context around the &quot;<br>
&emsp;&emsp;&quot;marker:\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;--- replace-text\n&quot;<br>
&emsp;&emsp;&quot;BEFORE:\n&quot;<br>
&emsp;&emsp;&quot;  &lt;lines that must appear immediately above the marker&gt;\n&quot;<br>
&emsp;&emsp;&quot;MARKER:\n&quot;<br>
&emsp;&emsp;&quot;  &lt;marker lines&gt;\n&quot;<br>
&emsp;&emsp;&quot;AFTER:\n&quot;<br>
&emsp;&emsp;&quot;  &lt;lines that must appear immediately below the marker&gt;\n&quot;<br>
&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&quot;&lt;payload lines...&gt;\n&quot;<br>
&emsp;&emsp;&quot;=END=\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;Rules:\n&quot;<br>
&emsp;&emsp;&quot;* YAML mode is enabled only when the first non-empty line after &quot;<br>
&emsp;&emsp;&quot;the command\n&quot;<br>
&emsp;&emsp;&quot;  is one of: `BEFORE:`, `MARKER:`, `AFTER:`.\n&quot;<br>
&emsp;&emsp;&quot;* Matching is strict: BEFORE lines must be directly above the &quot;<br>
&emsp;&emsp;&quot;marker block;\n&quot;<br>
&emsp;&emsp;&quot;  AFTER lines must be directly below it.\n&quot;<br>
&emsp;&emsp;&quot;* Whitespace differences are ignored (lines are trimmed before &quot;<br>
&emsp;&emsp;&quot;comparison).\n&quot;<br>
&emsp;&emsp;&quot;* If BEFORE/AFTER are present, there is no fallback to the first &quot;<br>
&emsp;&emsp;&quot;occurrence\n&quot;<br>
&emsp;&emsp;&quot;  of the marker.\n&quot;<br>
&emsp;&emsp;&quot;* If more than one place matches the strict context, the patch &quot;<br>
&emsp;&emsp;&quot;fails as\n&quot;<br>
&emsp;&emsp;&quot;  ambiguous.\n&quot;<br>
&emsp;&emsp;&quot;* If no place matches the strict context, the patch fails with\n&quot;<br>
&emsp;&emsp;&quot;  \&quot;strict marker context not found\&quot;.\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;## 4. File-level commands\n&quot;<br>
&emsp;&emsp;&quot;These operations work on the whole file rather than its contents.\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;### Create or overwrite file\n&quot;<br>
&emsp;&emsp;&quot;--- create-file\n&quot;<br>
&emsp;&emsp;&quot;&lt;file content...&gt;\n&quot;<br>
&emsp;&emsp;&quot;=END=\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;### Delete file\n&quot;<br>
&emsp;&emsp;&quot;--- delete-file\n&quot;<br>
&emsp;&emsp;&quot;=END=\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;## 5. Examples\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;=== file: src/a.cpp ===\n&quot;<br>
&emsp;&emsp;&quot;--- replace 3:4\n&quot;<br>
&emsp;&emsp;&quot;int value = 42;\n&quot;<br>
&emsp;&emsp;&quot;std::cout &lt;&lt; value &lt;&lt; \&quot;\\\\n\&quot;;\n&quot;<br>
&emsp;&emsp;&quot;=END=\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;=== file: src/b.cpp ===\n&quot;<br>
&emsp;&emsp;&quot;--- insert-after 12\n&quot;<br>
&emsp;&emsp;&quot;log_debug(\&quot;checkpoint reached\&quot;);\n&quot;<br>
&emsp;&emsp;&quot;=END=\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;=== file: src/c.cpp ===\n&quot;<br>
&emsp;&emsp;&quot;--- delete 20:25\n&quot;<br>
&emsp;&emsp;&quot;=END=\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;=== file: assets/config.json ===\n&quot;<br>
&emsp;&emsp;&quot;--- create-file\n&quot;<br>
&emsp;&emsp;&quot;{ \&quot;version\&quot;: 1 }\n&quot;<br>
&emsp;&emsp;&quot;=END=\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;=== file: old/temp.txt ===\n&quot;<br>
&emsp;&emsp;&quot;--- delete-file\n&quot;<br>
&emsp;&emsp;&quot;=END=\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;---\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;## Recommended usage\n&quot;<br>
&emsp;&emsp;&quot;* Prefer text-based commands (`*-text`) — they are more stable &quot;<br>
&emsp;&emsp;&quot;when code moves.\n&quot;<br>
&emsp;&emsp;&quot;* Use file-level commands when creating or removing entire files.\n&quot;<br>
&emsp;&emsp;&quot;* Group modifications to multiple files into **one patch file**.\n&quot;<br>
&emsp;&emsp;&quot;\n&quot;<br>
&emsp;&emsp;&quot;*This cheat sheet is in the text for a reason. If you're asked to &quot;<br>
&emsp;&emsp;&quot;write a patch,\n&quot;<br>
&emsp;&emsp;&quot; use the following format: chunk_v2.\n&quot;<br>
&emsp;&emsp;&quot;*Try to strictly follow the rules described in this document, &quot;<br>
&emsp;&emsp;&quot;without making any\n&quot;<br>
&emsp;&emsp;&quot; syntactic errors.\n&quot;<br>
&emsp;&emsp;&quot;*When working with chunks, be careful that commands do not &quot;<br>
&emsp;&emsp;&quot;reference the same\n&quot;<br>
&emsp;&emsp;&quot; text macros. Macros should never overlap.\n&quot;;<br>
}<br>
<!-- END SCAT CODE -->
</body>
</html>
