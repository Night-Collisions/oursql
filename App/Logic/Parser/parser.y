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
%token TABLE TABLES
%token LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE SEMI DOT COMMA
%token ID ICONST FCONST SCONST
%token INT REAL TEXT
%token NOT_NULL PRIMARY_KEY FOREIGN_KEY UNIQUE

%start expression

%type <ident> id ID
%type <constraint_str> constraints constraint 
%type <type> INT REAL TEXT type

%union {
    char constraint_str[100];
    char type[20];
    char ident[20];
	char *val; 
    FILE *response;
}

%%

expression: statements;

statements: statement SEMI | statements statement SEMI;

statement: create body |
    show_create | show | drop;

create: CREATE TABLE id { initTable($3); };

show: SHOW TABLES { yyerror("32423432424"); };

show_create: SHOW CREATE TABLE id { };

drop: DROP TABLE id {};

body: LPAREN decl RPAREN;

decl: variable | decl COMMA variable;

variable: id type {
    addField($1, $2, "");
} | id type constraints {
    addField($1, $2, $3);
};

constraints: constraint | 
    constraints constraint { strcat($$, $2); };

constraint: NOT_NULL { strcpy($$, "not_null "); } | 
    PRIMARY_KEY { strcpy($$, "primary_key "); }| 
    FOREIGN_KEY { strcpy($$, "foreign_key "); } | 
    UNIQUE { strcpy($$, "unique "); }; 

type: INT { strcpy($$, "int"); } |
        REAL { strcpy($$, "real"); } |
        TEXT { strcpy($$, "text"); };

id: ID { strcpy($$, yylval.ident);}

%%

void set_input_string(const char* in);
void end_lexical_scan(void);

int parse_string(const char* in) {
  set_input_string(in);
  int rv = yyparse();
  end_lexical_scan();
  return rv;
}