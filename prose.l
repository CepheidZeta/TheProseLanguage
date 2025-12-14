%{
#include "prose.tab.h"
#include <string.h>
#include <stdlib.h>

int line_number = 1;
%}

%%


[ \t]+     ;


\r\n       { ++line_number; }       
\n         { ++line_number; }       
\r         { ++line_number; }       
 
     
"start"         { return START; }
"done"          { return DONE; }
"integer"       { return INTEGER; }
"character"     { return CHARACTER; }
"do"            { return DO; }
"show"          { return SHOW; }

[a-zA-Z][a-zA-Z0-9_]* {
        yylval.sval = strdup(yytext);
        return ID;
}

[0-9]+ {
        yylval.ival = atoi(yytext);
        return NUMBER;
}

\"[^\n]*\" {
    int len = yyleng - 2;
    char *s = malloc(len + 1);
    strncpy(s, yytext + 1, len);
    s[len] = '\0';
    yylval.sval = s;
    return STRING;
}


"="             { return ASSIGN; }
","             { return COMMA; }
"."             { return DOT; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MULT; }
"/"             { return DIV; }
"("             { return LPAREN; }
")"             { return RPAREN; }


.               {
        printf("Lexical error: unexpected character '%s' at line %d\n", yytext, line_number);
}

%%

int yywrap() { return 1; }
