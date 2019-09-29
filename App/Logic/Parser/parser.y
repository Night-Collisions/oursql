%{
    #include "D:\IT\Cpp\oursql\App\Logic\TableManager.h"
    #include <stdio.h>
    #include <string>

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
%type <inum> ICONST
%type <type> INT REAL TEXT 

%union {
    char type[100];
    char ident[100];
	int inum; 
	double dnum; 
}

%%
expression: statements SEMI;

statements: create_st body  | create_st | show_st | drop_st;

create_st: CREATE TABLE ID { initTable(yylval.ident); };

show_st: SHOW TABLE ID { };

drop_st: DROP TABLE ID {};

body: LPAREN decl RPAREN SEMI;

decl: variable | decl COMMA variable;

variable: ID type {
    addField(yylval.ident, $2, 0);
} | ID type LPAREN ICONST RPAREN {
    addField(yylval.ident, $2, inum);
} ;

type: INT {$$ = "int";} | REAL {$$ = "real";} | TEXT {$$ = "text";}

%%