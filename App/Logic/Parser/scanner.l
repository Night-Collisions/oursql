%{
    #include <iostream>
    #include <stdio.h>
    #include <iostream>
    #include <sstream>
    #include <map>

    
    #include "../../App/Logic/Parser/Nodes/Node.h"
    #include "../../App/Logic/Parser/Nodes/Query.h"
    #include "../../App/Logic/Parser/Nodes/VarList.h"
    #include "../../App/Logic/Parser/Nodes/Variable.h"
    #include "../../App/Logic/Parser/Nodes/Ident.h"
    #include "../../App/Logic/Parser/Nodes/Constant.h"
    #include "../../App/Logic/Parser/Nodes/NullConstant.h"
    #include "../../App/Logic/Parser/Nodes/IntConstant.h"
    #include "../../App/Logic/Parser/Nodes/RealConstant.h"
    #include "../../App/Logic/Parser/Nodes/TextConstant.h"
    #include "../../App/Logic/Parser/Nodes/Relation.h"
    #include "../../App/Logic/Parser/Nodes/ConstantList.h"
    #include "../../App/Logic/Parser/Nodes/IdentList.h"
    #include "../../App/Logic/Parser/Nodes/SelectList.h"

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
aquote        \'

ID            {alpha}+{alnum_dash}*
ICONST        "0"|-?[0-9]{digit}*
FCONST        -?{digit}*"."{digit}+
SCONST        {dquote}.*{dquote}|{aquote}.*{aquote}

NOT_NULL      "not"[ \t\r\f]+"null"
PRIMARY_KEY   "primary"[ \t\r\f]+"key"

%%

"//".*\n                   ; // ingore comments

"*"                        { return ASTERISK; }
"+"                        { return PLUS; }
"-"                        { return MINUS; }
"/"                        { return DIVIDE; }
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
"set"                      { return SET; }
"update"                   { return UPDATE; }
"delete"                   { return DELETE; }


"int"                      { return INT; }
"text"                     { return TEXT; }
"real"                     { return REAL; }

{NOT_NULL}                 { return NOT_NULL; }
"null"                     { return NULL_; }
{PRIMARY_KEY}              { return PRIMARY_KEY; }
"unique"                   { return UNIQUE; }

{ID}                       { 
                              sscanf(yytext, "%s", tmp_str);
                              yylval.name = new std::string(tmp_str); 
                              return ID; 
                           }

{FCONST}                   { 
                              sscanf(yytext, "%s", tmp_str);
                              yylval.rConst = new RealConstant(std::string(tmp_str));
                              return FCONST; 
                           }

{ICONST}                   { 
                              sscanf(yytext, "%s", tmp_str);
                              yylval.iConst = new IntConstant(std::string(tmp_str));
                              return ICONST; 
                           }
                           
{SCONST}                   {
                              sscanf(yytext, "%s", tmp_str);
                              yylval.tConst = new TextConstant(std::string(tmp_str));
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
