# proseLanguage_backend
Run: <br>
<br>win_flex prose.l
<br>win_bison -d prose.y
<br>gcc lex.yy.c prose.tab.c -o prose.exe
<br>./prose.exe input.txt
