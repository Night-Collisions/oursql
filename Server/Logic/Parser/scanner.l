%{
    #include <iostream>
    #include <stdio.h>
    #include <iostream>
    #include <sstream>
    #include <map>

  
    #include "../../Server/Logic/Parser/Nodes/Node.h"
    #include "../../Server/Logic/Parser/Nodes/Query.h"
    #include "../../Server/Logic/Parser/Nodes/VarList.h"
    #include "../../Server/Logic/Parser/Nodes/Variable.h"
    #include "../../Server/Logic/Parser/Nodes/Ident.h"
    #include "../../Server/Logic/Parser/Nodes/Constant.h"
    #include "../../Server/Logic/Parser/Nodes/NullConstant.h"
    #include "../../Server/Logic/Parser/Nodes/IntConstant.h"
    #include "../../Server/Logic/Parser/Nodes/RealConstant.h"
    #include "../../Server/Logic/Parser/Nodes/TextConstant.h"
    #include "../../Server/Logic/Parser/Nodes/ConstantList.h"
    #include "../../Server/Logic/Parser/Nodes/IdentList.h"
    #include "../../Server/Logic/Parser/Nodes/SelectList.h"
    #include "../../Server/Logic/Parser/Nodes/Expression.h"
    #include "../../Server/Logic/Parser/Nodes/RelExpr.h"

    #include "../../Server/Engine/Engine.h"
    #include "parser.hpp"
    
    extern FILE *yyin;
    extern FILE *yyout;
    void yyerror(const char *s);

    int lineno = 1;
    long long int symbols = 0;
    //todo: free?
    char *tmp_str = (char *) malloc(100);
    std::string in;
    std::string local_str;
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
ICONST        [ \t\r\f]*[0-9]{digit}*
FCONST        [ \t\r\f]*{digit}*"."{digit}+
SCONST        ["]([^"]|["]["]|[']['])*["]|[']([^']|["]["]|[']['])*[']

VARCHAR       "varchar"[ \t\r\f]*"("[ \t\r\f]*{ICONST}{1}[ \t\r\f]*")"

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

"and"|"&&"                 { return AND; }
"or"|"||"                  { return OR; } 
"not"                      { return NOT; }

"left"                     { return LEFT; }
"right"                    { return RIGHT; }
"inner"                    { return INNER; }
"outer"                    { return OUTER; }
"full"                     { return FULL; }
"join"                     { return JOIN; }
"union"                    { return UNION; }
"intersect"                { return INTERSECT; }

"as"                       { return AS; }
"on"                       { return ON; }
  
"int"                      { return INT; }
"real"                     { return REAL; }
{VARCHAR}                  { 
                              local_str = "";
                              int i = 0;
                              while (yytext[i] != '\0') {
                                if (isdigit(yytext[i])) {
                                    local_str += yytext[i];
                                }
                                ++i;
                              }

                              try {
                                yylval.varcharLen = std::stoi(local_str);
                              } catch(std::invalid_argument& tmp) {
                                yyerror("varchar oversize");
                                return 1;
                              }

                              return VARCHAR; 
                           }

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
                              local_str = "";
                              int checked = 0;
                              for (int i = 1; i < strlen(yytext) - 1; ++i) {
                                if (!checked && (yytext[i] == '\'' || yytext[i] == '\"')) {
                                  checked = 1;
                                } else {
                                  checked = 0;
                                  local_str += yytext[i];
                                }
                              }
                              
                              yylval.tConst = new TextConstant(local_str);
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
