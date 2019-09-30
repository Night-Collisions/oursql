%{
    #include <stdio.h>
    #include "parser.hpp"

    void yyerror(char *s) {
      fprintf (stderr, "%s\n", s);
    }

    extern FILE *yyin;
    extern FILE *yyout;

    int lineno = 1;
    //todo: free?
    char *tmp_str = (char *) malloc(100);
%}

%option noyywrap

alpha         [a-zA-Z]
digit         [0-9]
alnum         {alpha}|{digit}
alnum_dash    {alnum}|_

ID            {alpha}+{alnum_dash}*
ICONST        "0"|[0-9]{digit}*
FCONST        "0"|{digit}*"."{digit}+

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

{FCONST}                   { 
                              sscanf(yytext, "%s", tmp_str);
                              yylval.val = tmp_str;
                              return FCONST; 
                            }

{ICONST}                   { 
                              sscanf(yytext, "%s", tmp_str);
                              yylval.val = tmp_str;
                              return ICONST; 
                           }

"\n"                       { lineno += 1; }
[ \t\r\f]+                 ; //ignore spaces

.                          { yyerror("Unknown character"); }


%%