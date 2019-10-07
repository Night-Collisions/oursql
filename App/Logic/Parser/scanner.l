%{
    #include <iostream>
    #include <stdio.h>
    
    #include "../../App/Logic/Parser/Nodes/Node.h"
    #include "../../App/Logic/Parser/Nodes/Query.h"
    #include "../../App/Logic/Parser/Nodes/VarList.h"
    #include "../../App/Logic/Parser/Nodes/Command.h"
    #include "../../App/Logic/Parser/Nodes/Variable.h"
    #include "../../App/Logic/Parser/Nodes/Ident.h"

    #include "../../App/Engine/Engine.h"
    #include "parser.hpp"
    
    extern FILE *yyin;
    extern FILE *yyout;

    int lineno = 1;
    long long int symbols = 0;
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

%%

"//".*\n                   ; // ingore comments

"*"                        { return ASTERISK; }
"("                        { return LPAREN; }
")"                        { return RPAREN; }
"]"                        { return LBRACK; }
"["                        { return RBRACK; }
"{"                        { return LBRACE; }
"}"                        { return RBRACE; }
";"                        { return SEMI; }
"."                        { return DOT; }
","                        { return COMMA; }
"="                        { return EQUAL; }
"<"                        { return LESS; }
">"                        { return GREATER; }
"<="                       { return LESS_EQ; }
">="                       { return GREATER_EQ; }
"!="                       { return NOT_EQ; }


"create"                   { return CREATE; }
"show"                     { return SHOW; }
"drop"                     { return DROP; }
"tables"                   { return TABLES; }
"table"                    { return TABLE; }
"select"                   { return SELECT; }
"from"                     { return FROM; }
"where"                    { return WHERE; }
"insert"                   { return INSERT; }
"into"                     { return INTO; }
"values"                   { return VALUES; }


"int"                      { return INT; }
"text"                     { return TEXT; }
"real"                     { return REAL; }

{NOT_NULL}                 { return NOT_NULL; }
{PRIMARY_KEY}              { return PRIMARY_KEY; }
"unique"                   { return UNIQUE; }

{ID}                       { 
                              sscanf(yytext, "%s", tmp_str);
                              yylval.name = new std::string(tmp_str); 
                              return ID; 
                           }

{FCONST}                   { 

                              return FCONST; 
                           }

{ICONST}                   { 

                              return ICONST; 
                           }
                           
{SCONST}                   {

                              return SCONST; 
                           }        

"\n"                       { lineno += 1; }
[ \t\r\f]+                 ; //ignore spaces

.                          {  }


%%

void set_input_string(const char* in) {
  yy_scan_string(in);
}

void end_lexical_scan(void) {
  yy_delete_buffer(YY_CURRENT_BUFFER);
}
