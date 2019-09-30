%{
    #include "../../App/Logic/TableManager.h"
    #include <stdio.h>
    #include <string>
    #include <cstring>

    extern FILE *yyin;
    extern FILE *yyout;
    extern int lineno;
    extern int yylex();
    void yyerror(char *s);
%}

%token CREATE SHOW DROP
%token TABLE
%token LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE SEMI DOT COMMA
%token ID ICONST FCONST
%token INT REAL TEXT

%start expression

%type <ident> ID
%type <val> consts assign
%type <type> INT REAL TEXT type

%union {
    char type[30];
    char ident[100];
	char *val; 
}

%%

expression: statements SEMI;

statements: statement | statements statement;

statement: create_st body | 
    show_st | drop_st;

create_st: CREATE TABLE ID { initTable(yylval.ident); };

show_st: SHOW TABLE ID { };

drop_st: DROP TABLE ID {};

body: LPAREN decl RPAREN SEMI;

decl: variable | decl COMMA variable;

variable: ID type {
    addField(yylval.ident, $2, "");
} | ID type assign {
    addField(yylval.ident, $2, $3);
};

assign: LPAREN consts RPAREN { $$ = yylval.val; };

consts: ICONST | FCONST;

type: INT { strcpy($$, "int"); } | 
        REAL { strcpy($$, "real"); } | 
        TEXT { strcpy($$, "text"); };

%%