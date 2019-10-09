%{
    #include "../../App/Logic/Parser/Nodes/Node.h"
    #include "../../App/Logic/Parser/Nodes/Query.h"
    #include "../../App/Logic/Parser/Nodes/VarList.h"
    #include "../../App/Logic/Parser/Nodes/Command.h"
    #include "../../App/Logic/Parser/Nodes/Variable.h"
    #include "../../App/Logic/Parser/Nodes/Ident.h"
    #include "../../App/Logic/Parser/Nodes/IntConstant.h"
    #include "../../App/Logic/Parser/Nodes/RealConstant.h"
    #include "../../App/Logic/Parser/Nodes/TextConstant.h"
    #include "../../App/Logic/Parser/Nodes/ConstantList.h"
    #include "../../App/Logic/Parser/Nodes/IdentList.h"
    #include "../../App/Logic/Parser/Nodes/Relation.h"
    #include "../../App/Logic/Parser/Nodes/SelectList.h"
    #include "../../App/Core/Exception.h"

    #include "../../App/Engine/Engine.h"
    #include "../../App/Engine/Column.h"

    #include <stdio.h>
    #include <string>
    #include <cstring>
    #include <iostream>
    #include <sstream>

    extern FILE *yyin;
    extern FILE *yyout;

    extern int lineno;
    extern int yylex();
    void yyerror(const char *s);

    Query *parseTree;
    std::vector<Variable *> varList;
    std::vector<ColumnConstraint> constraintList;
    std::vector<Ident*> identList;
    std::vector<Node*> constantList;
    std::vector<Ident> selectList;

    std::unique_ptr<exc::Exception> ex;
%}

%error-verbose

%token CREATE SHOW DROP SELECT INSERT
%token TABLE TABLES VALUES INTO FROM WHERE
%token LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE SEMI DOT COMMA ASTERISK
%token EQUAL GREATER LESS GREATER_EQ LESS_EQ NOT_EQ 
%token ID ICONST FCONST SCONST
%token INT REAL TEXT
%token NOT_NULL PRIMARY_KEY UNIQUE

%type<query> create show_create drop_table select insert
%type<ident> id select_list_element
%type<var> variable
%type<dataType> type
%type<constraint> constraint
%type<iConst> int_const
%type<rConst> real_const
%type<tConst> text_const
%type<anyConstant> constant where_element
%type<relation> where_condition
%type<relType> relation

%start expression

%union {
    ColumnConstraint constraint;
    Ident *ident;
    Command *command;
    Query *query;
    Variable *var;
    DataType dataType;
    std::string *name;
    IntConstant *iConst;
    RealConstant *rConst;
    TextConstant *tConst;
    Node *anyConstant;
    Relation *relation;
    RelationType relType;
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
    insert;

// ---- create table

create: 
    CREATE TABLE id LPAREN variables RPAREN {     
        std::vector<Node*> children;
        children.push_back(new Command(CommandType::create_table));
        children.push_back($3);
        children.push_back(new VarList(varList));

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
        constraintList.push_back($1);
    } | 
    constraints constraint {
        constraintList.push_back($2);
    };

constraint: 
    NOT_NULL { $$ = ColumnConstraint::not_null;  } |
    PRIMARY_KEY { $$ = ColumnConstraint::primary_key; }|
    UNIQUE { $$ = ColumnConstraint::unique; };

// --- select

select:
     SELECT select_list FROM id WHERE where_condition {
        std::vector<Node*> children;
        children.push_back(new Command(CommandType::select));
        children.push_back($4);
        children.push_back(new SelectList(selectList));
        children.push_back($6);

        parseTree = new Query(children);
     } |
    SELECT asterisk COMMA select_list FROM id WHERE where_condition {
        std::vector<Node*> children;
        children.push_back(new Command(CommandType::select));
        children.push_back($6);
        children.push_back(new SelectList(selectList));
        children.push_back($8);

        parseTree = new Query(children);
    };

asterisk:
    ASTERISK {
    	selectList.push_back(Ident("*"));
    };

select_list: 
    select_list_element {
        selectList.push_back(*$1);
    } |
    select_list COMMA select_list_element {
        selectList.push_back(*$3);
    };

select_list_element:
    id DOT id {
        $$ = new Ident((*$1).getTableName(), (*$3).getName());
    } | 
    id {
        $$ = new Ident((*$1).getName());
    };

where_condition: 
    where_element relation where_element {
        $$ = new Relation($1, $2, $3);
    };

where_element:
    id { $$ = new Ident(*$1); } |
    constant { $$ = $1; };

relation:
    EQUAL { $$ = RelationType::equal; } |
    GREATER { $$ = RelationType::greater;} |
    GREATER_EQ { $$ = RelationType::greater_eq;} |
    LESS { $$ = RelationType::less; } |
    LESS_EQ { $$ = RelationType::less_eq;} |
    NOT_EQ { $$ = RelationType::not_equal;};

// --- show create table

show_create: 
    SHOW CREATE TABLE id {
        std::vector<Node*> children;
        children.push_back(new Command(CommandType::show_create_table));
        children.push_back($4);

        parseTree = new Query(children);
    };

// --- drop table

drop_table: 
    DROP TABLE id {
        std::vector<Node*> children;
        children.push_back(new Command(CommandType::drop_table));
        children.push_back($3);

        parseTree = new Query(children);
    };

// --- insert

insert: 
    INSERT INTO id LPAREN column_list RPAREN VALUES LPAREN value_list RPAREN {
        std::vector<Node*> children;
        children.push_back(new Command(CommandType::insert));
        children.push_back(new IdentList(identList));
        children.push_back(new ConstantList(constantList));

        parseTree = new Query(children);
    } |
    INSERT INTO id VALUES LPAREN value_list RPAREN {
        std::vector<Node*> children;
        children.push_back(new Command(CommandType::insert));
        children.push_back(nullptr);
        children.push_back(new ConstantList(constantList));

        parseTree = new Query(children);
    };

column_list: 
    id {
        identList.push_back(new Ident(*$1));
    } | column_list COMMA id {
        identList.push_back(new Ident(*$3));
    };

value_list:
    constant {
        constantList.push_back($1);
    } |
    value_list COMMA constant {
        constantList.push_back($3);
    };


// ---

constant:
    int_const { $$ = $1; } |
    real_const { $$ = $1; } |
    text_const { $$ = $1; };

int_const:
    ICONST {
        $$ = yylval.iConst;
    };

real_const:
    FCONST {
	$$ = yylval.rConst;
    };

text_const:
    SCONST {
	$$ = yylval.tConst;
    };

type: 
    INT { $$ = DataType::integer; } |
    REAL { $$ = DataType::real; } |
    TEXT { $$ = DataType::text; };

id: 
    ID { $$ = new Ident(*yylval.name); }

%%

void yyerror(const char *s) {
    ex.reset(new exc::SyntaxException());
}

void set_input_string(const char* in);
void end_lexical_scan(void);

void destroy() {
    varList.clear();
    constraintList.clear();
}

Query* parse_string(const char* in, std::unique_ptr<exc::Exception>& exception) {
    destroy();
    exception.reset(nullptr);

    set_input_string(in);
    yyparse();
    end_lexical_scan();

    exception = std::move(ex);
    
    return parseTree;
}
