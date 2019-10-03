%{
    #include "../../App/Engine/Engine.h"
    #include "../../App/Logic/TableManager.h"
    #include <stdio.h>
    #include <string>
    #include <cstring>

    extern FILE *yyin;
    extern FILE *yyout;
    extern int lineno;
    extern int yylex();
    void yyerror(const char *s);

    int logging = 0;
    char buff[50];
%}

%error-verbose

%token CREATE SHOW DROP
%token TABLE TABLES
%token LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE SEMI DOT COMMA
%token ID ICONST FCONST SCONST
%token INT REAL TEXT
%token NOT_NULL PRIMARY_KEY UNIQUE

%start expression

%type <ident> id ID create show_create
%type <constraint_str> constraints constraint 
%type <type> INT REAL TEXT type

%union {
    char constraint_str[100];
    char type[20];
    char ident[20];
	char *val; 
    
}

%%

expression: statements;

statements: statement SEMI | statements statement SEMI;

statement: create body {
    int res = create(getTable());
    if (logging) {
        fprintf(yyout, "%d", res);
    }
} |
    show_create {
        try {
            fprintf(yyout, "%s", showCreateTable(showCreate($1)));
        } catch(std::exception& e) {
            yyerror(e.what());
            yyclearin;
        }
    } | show | drop | error SEMI {yyerrok; yyclearin;};

create: CREATE TABLE id {     
    try {
        initTable($3);

    } catch(std::exception& e) {
        yyerror(e.what());
        yyclearin;
    } } ;

show: SHOW TABLES ;

show_create: SHOW CREATE TABLE id { 
    strcpy($$, $4);
};

drop: DROP TABLE id {
    try {
            dropTable($3);
        } catch(std::exception& e) {
            yyerror(e.what());
            yyclearin;
        }};

body: LPAREN decl RPAREN;

decl: variable | decl COMMA variable;

variable: id type {
    try {
        addField($1, $2, "");
    } catch (std::invalid_argument& e) {
        yyerror(e.what());
        yyclearin;
    }
} | id type constraints {
    try {
        addField($1, $2, $3);
    } catch (std::invalid_argument& e) {
        yyerror(e.what());
        yyclearin;
    }
};

constraints: constraint | 
    constraints constraint { strcat($$, $2); };

constraint: NOT_NULL { strcpy($$, "not_null "); } | 
    PRIMARY_KEY { strcpy($$, "primary_key "); }|
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

void set_logging(int i) {
    logging = i;
}