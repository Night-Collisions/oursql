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
%token ID ICONST FCONST SCONST
%token INT REAL TEXT

%start expression

%type <ident> id ID
%type <val> assign
%type <type> INT REAL TEXT type

%union {
    char type[20];
    char ident[20];
	char *val; 
}

%%

expression: statements SEMI;

statements: statement | statements statement;

statement: create_st body |
    show_st | drop_st;

create_st: CREATE TABLE id { initTable($3); };

show_st: SHOW TABLE id { };

drop_st: DROP TABLE id {};

body: LPAREN decl RPAREN;

decl: variable | decl COMMA variable;

variable: id type {
    addField($1, $2, "");
} | id type assign {
    addField($1, $2, $3);
};

assign: LPAREN consts RPAREN { $$ = yylval.val; };

consts: ICONST | FCONST | SCONST;

type: INT { strcpy($$, "int"); } |
        REAL { strcpy($$, "real"); } |
        TEXT { strcpy($$, "text"); };

id: ID { strcpy($$, yylval.ident);}

%%