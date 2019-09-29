%{
    #include <stdio.h>
    // + какая то сторонняя библа типа api от движка

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

%type <name> ID
%type <number> INT REAL

%union{
	char name[20];
	int number; 
}

%%
expression: statements SEMI;

statements: create_st body {} | create_st | show_st | drop_st;

create_st: CREATE TABLE ID;

show_st: SHOW TABLE ID { /*show()*/};

drop_st: DROP TABLE ID {};

body: LPAREN decl RPAREN SEMI;

decl: variable | decl COMMA variable;

variable: ID type;

type: INT | REAL | TEXT

%%