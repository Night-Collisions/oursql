%{
    #include <stdio.h>
    #include "parser.hpp"

    void yyerror(char *s) {
      fprintf (stderr, "%s\n", s);
    }

    extern FILE *yyin;
    extern FILE *yyout;

    int lineno = 1;
%}

%option noyywrap

alpha       [a-zA-Z]
digit       [0-9]B
alnum       {alpha}|{digit}
alnum_dash  {alnum}|_

ID          {alpha}+{alnum_dash}*
ICONST      "0"|[0-9]{digit}*
FCONST      "0"|{digit}*"."{digit}+

%%

"//".*\n                   ; // ingore comments

"("                        { return LPAREN; }
")"                        { return RPAREN; }
"]"                        { return LBRACK; }
"["                        { return RBRACK; }
"{"                        { return LBRACE; }
"}"                        { return RBRACE; }
";"                        { return SEMI; }
"."                        { return DOT; }
","                        { return COMMA; }

"create"|"CREATE"          { return CREATE; }
"show"|"SHOW"              { return SHOW; }
"drop"|"DROP"              { return DROP; }
"table"|"TABLE"            { return TABLE; }

"int"|"INT"                { return INT; }
"text"|"TEXT"              { return TEXT; }
"real"|"REAL"              { return REAL; }

{ID}                       { 
                              sscanf(yytext, "%s", yylval.ident); 
                              return ID; 
                           }
{ICONST}                   { 
                              sscanf(yytext, "%d", yylval.inum);
                              sprintf(yylval.val, "%d", yylval.inum);
                              return ICONST; 
                           }
{FCONST}                   { 
                              sscanf(yytext, "%d", yylval.dnum);
                              sprintf(yylval.val, "%f", yylval.dnum);
                              return FCONST; 
                            }

"\n"                       { lineno += 1; }
[ \t\r\f]+                 ; //ignore spaces

.                          { yyerror("Unknown character"); }


%%