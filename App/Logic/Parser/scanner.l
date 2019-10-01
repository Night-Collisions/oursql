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
%option caseless

alpha         [a-zA-Z]
digit         [0-9]
alnum         {alpha}|{digit}
alnum_dash    {alnum}|_
dquote        \"

ID            {alpha}+{alnum_dash}*
ICONST        "0"|[0-9]{digit}*
FCONST        "0"|{digit}*"."{digit}+
SCONST        {dquote}.*{dquote}

NOT_NULL      "not"[ \t\r\f]+"null"
PRIMARY_KEY   "primary"[ \t\r\f]+"key"
FOREIGN_KEY   "foreign"[ \t\r\f]+"key"

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

"create"                   { return CREATE; }
"show"                     { return SHOW; }
"drop"                     { return DROP; }
"tables"                   { return TABLES; }
"table"                    { return TABLE; }

"int"                      { return INT; }
"text"                     { return TEXT; }
"real"                     { return REAL; }

{NOT_NULL}                 { return NOT_NULL; }
{PRIMARY_KEY}              { return PRIMARY_KEY; }
{FOREIGN_KEY}              { return FOREIGN_KEY; }
"unique"                   { return UNIQUE; }

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
                           
{SCONST}                   {
                              sscanf(yytext, "%s", tmp_str);
                              yylval.val = tmp_str;
                              return SCONST; 
                           }        

"\n"                       { lineno += 1; }
[ \t\r\f]+                 ; //ignore spaces

.                          { yyerror("Unknown character"); }


%%

void set_input_string(const char* in) {
  yy_scan_string(in);
}

void end_lexical_scan(void) {
  yy_delete_buffer(YY_CURRENT_BUFFER);
}