%{
    #include "../../App/Logic/Parser/Nodes/Node.h"
    #include "../../App/Logic/Parser/Nodes/Query.h"
    #include "../../App/Logic/Parser/Nodes/VarList.h"
    #include "../../App/Logic/Parser/Nodes/Command.h"
    #include "../../App/Logic/Parser/Nodes/Variable.h"
    #include "../../App/Logic/Parser/Nodes/Ident.h"
    #include "../../App/Core/Exception.h"

    #include "../../App/Engine/Engine.h"
    #include "../../App/Engine/Column.h"

    #include <stdio.h>
    #include <string>
    #include <cstring>

    extern FILE *yyin;
    extern FILE *yyout;
    extern int lineno;
    extern int yylex();
    void yyerror(const char *s);

    Query *parseTree;
    std::vector<Variable *> varList;
    std::set<ColumnConstraint> constraintList;

    exc::Exception* exception;
%}

%error-verbose

%token CREATE SHOW DROP SELECT
%token TABLE TABLES VALUES
%token LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE SEMI DOT COMMA ASTERISK
%token EQUAL GREATER LESS GREATER_EQ LESS_EQ NOT_EQ 
%token ID ICONST FCONST SCONST
%token INT REAL TEXT
%token NOT_NULL PRIMARY_KEY UNIQUE

%type<query> create show_create drop_table select
%type<ident> id
%type<var> variable
%type<dataType> type
%type<constraint> constraint

%start expression

%union {
    ColumnConstraint constraint;
    Ident *ident;
    Command *command;
    Query *query;
    Variable *var;
    DataType dataType;
    std::string *name;
}

%%

expression: 
    statements SEMI;

statements: 
    statement {
        varList.clear();
    } | 
    statements statement {
        varList.clear();
    };

statement: 
    create |
    show_create | 
    drop_table | 
    select |
    error SEMI { yyerrok; };

select:
     SELECT select_list FROM id WHERE where_condition

select_list: 
    ASTERISK |
    ASTERISK COMMA id;

where_condition: 
    id relation id;

relation:
    EQUAL |
    GREATER |
    GREATER_EQ |
    LESS |
    LESS_EQ |
    NOT_EQ;

create: 
    CREATE TABLE id LPAREN variables RPAREN {     
        std::vector<Node*> children;
        children.push_back(new Command(CommandType::create_table));
        children.push_back($3);
        children.push_back(new VarList(varList));

        parseTree = new Query(children);
    };

show_create: 
    SHOW CREATE TABLE id {
        std::vector<Node*> children;
        children.push_back(new Command(CommandType::show_create_table));
        children.push_back($4);

        parseTree = new Query(children);
    };

drop_table: 
    DROP TABLE id {
        std::vector<Node*> children;
        children.push_back(new Command(CommandType::drop_table));
        children.push_back($3);

        parseTree = new Query(children);
    };

variables: 
    variable {
        varList.push_back($1);
    } | 
    variables COMMA variable {
        varList.push_back($3);
    };

variable: 
    id type {
        $$ = new Variable($1->getName(), $2);
    } | id type constraints {
        $$ = new Variable($1->getName(), $2, constraintList);
        constraintList.clear();
    };

constraints: 
    constraint {
        constraintList.insert($1);
    } | 
    constraints constraint {
        constraintList.insert($2);
    };

constraint: 
    NOT_NULL { $$ = ColumnConstraint::not_null;  } |
    PRIMARY_KEY { $$ = ColumnConstraint::primary_key; }|
    UNIQUE { $$ = ColumnConstraint::unique; };

type: 
    INT { $$ = DataType::integer; } |
    REAL { $$ = DataType::real; } |
    TEXT { $$ = DataType::text; };

id: 
    ID { $$ = new Ident(*yylval.name); }

%%

void yyerror(const char *s) {
    fprintf (stderr, "%s\n", s);
}

void set_input_string(const char* in);
void end_lexical_scan(void);

void destroy() {
    varList.clear();
    constraintList.clear();
}

Query* parse_string(const char* in, exc::Exception* ex) {
    destroy();
    ex = nullptr;

    set_input_string(in);
    yyparse();
    end_lexical_scan();

    ex = exception;
    
    return parseTree;
}
