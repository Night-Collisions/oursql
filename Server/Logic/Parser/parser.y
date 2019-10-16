%{
    #include "../../Server/Logic/Parser/Nodes/Node.h"
    #include "../../Server/Logic/Parser/Nodes/Query.h"
    #include "../../Server/Logic/Parser/Nodes/VarList.h"
    #include "../../Server/Logic/Parser/Nodes/Variable.h"
    #include "../../Server/Logic/Parser/Nodes/Ident.h"
    #include "../../Server/Logic/Parser/Nodes/IntConstant.h"
    #include "../../Server/Logic/Parser/Nodes/Constant.h"
    #include "../../Server/Logic/Parser/Nodes/RealConstant.h"
    #include "../../Server/Logic/Parser/Nodes/TextConstant.h"
    #include "../../Server/Logic/Parser/Nodes/ConstantList.h"
    #include "../../Server/Logic/Parser/Nodes/IdentList.h"
    #include "../../Server/Logic/Parser/Nodes/Relation.h"
    #include "../../Server/Logic/Parser/Nodes/NullConstant.h"
    #include "../../Server/Logic/Parser/Nodes/SelectList.h"
    #include "../../Server/Core/Exception.h"

    #include "../../Server/Engine/Engine.h"
    #include "../../Server/Engine/Column.h"

    #include <stdio.h>
    #include <string>
    #include <cstring>
    #include <iostream>
    #include <map>
    #include <sstream>

    extern FILE *yyin;
    extern FILE *yyout;

    extern int lineno;
    extern int yylex();
    void yyerror(const char *s);
    void destroy();

    Query *parseTree;
    std::vector<Variable *> varList;
    std::vector<ColumnConstraint> constraintList;
    std::vector<Ident*> identList;
    std::vector<Node*> constantList;
    std::vector<Ident> selectList;

    std::unique_ptr<exc::Exception> ex;
%}

%error-verbose

%token CREATE SHOW DROP SELECT INSERT UPDATE DELETE
%token TABLE TABLES VALUES INTO FROM WHERE SET
%token LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE SEMI DOT COMMA ASTERISK
%token EQUAL GREATER LESS GREATER_EQ LESS_EQ NOT_EQ
%token ID ICONST FCONST SCONST
%token INT REAL TEXT
%token NOT_NULL PRIMARY_KEY UNIQUE NULL_
%token DIVIDE MINUS PLUS

%type<query> create show_create drop_table select insert
%type<ident> id select_list_element
%type<var> variable
%type<dataType> type
%type<constraint> constraint
%type<iConst> int_const
%type<rConst> real_const
%type<tConst> text_const
%type<nullConst> null_
%type<anyConstant> constant where_element
%type<relation> where_condition
%type<relType> relation

%start expression

%union {
    ColumnConstraint constraint;
    Ident *ident;
    Query *query;
    Variable *var;
    DataType dataType;
    std::string *name;
    IntConstant *iConst;
    RealConstant *rConst;
    TextConstant *tConst;
    NullConstant *nullConst;
    Node *anyConstant;
    Relation *relation;
    RelationType relType;
}

%%

expression:
    statements SEMI {
        varList.clear();
        constraintList.clear();
        identList.clear();
        selectList.clear();
        constantList.clear();
    };

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
    insert |
    update |
    delete ;

// ---- create table

create:
    CREATE TABLE id LPAREN variables RPAREN {
        std::map<NodeType, Node*> children;
        children[NodeType::ident] = $3;
        children[NodeType::var_list] = new VarList(varList);

        parseTree = new Query(children, CommandType::create_table);
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
     SELECT select_decl FROM id where_condition {
        std::map<NodeType, Node*> children;
        children[NodeType::ident] = $4;
        children[NodeType::select_list] = new SelectList(selectList);
        children[NodeType::relation] = $5;

        parseTree = new Query(children, CommandType::select);
     } ;

select_decl:
    asterisk | 
    asterisk COMMA select_list | 
    select_list;

select_list:
    select_list_element {
        selectList.push_back(*$1);
    } |
    select_list COMMA select_list_element {
        selectList.push_back(*$3);
    };

asterisk:
    ASTERISK {
    	selectList.push_back(Ident("*"));
    };

select_list_element:
    id DOT id {
        $$ = new Ident((*$1).getName(), (*$3).getName());
    } |
    id {
        $$ = new Ident((*$1).getName());
    };

where_condition:
    WHERE where_element relation where_element {
        $$ = new Relation($2, $3, $4);
    } | 
    /*empty*/{ $$ = nullptr; };

where_element:
    id { $$ = new Ident(*$1); } |
    constant { $$ = $1; };

logic_operator:


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
        std::map<NodeType, Node*> children;
        children[NodeType::ident] = $4;

        parseTree = new Query(children, CommandType::show_create_table);
    };

// --- drop table

drop_table:
    DROP TABLE id {
        std::map<NodeType, Node*> children;
        children[NodeType::ident] = $3;

        parseTree = new Query(children, CommandType::drop_table);
    };

// --- insert

insert:
    INSERT INTO id column_decl VALUES LPAREN value_list RPAREN {
        std::map<NodeType, Node*> children;
        children[NodeType::ident] = $3;
        children[NodeType::ident_list] = new IdentList(identList);
        children[NodeType::constant_list] = new ConstantList(constantList);

        parseTree = new Query(children, CommandType::insert);
    };

column_decl:
    LPAREN column_list RPAREN | 
    /*empty*/ { };

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


// --- update
update:
    UPDATE id SET assignings where_condition {
        std::map<NodeType, Node*> children;
        children[NodeType::ident] = $2;
        children[NodeType::ident_list] = new IdentList(identList);
        children[NodeType::constant_list] = new ConstantList(constantList);
        children[NodeType::relation] = $5;

        parseTree = new Query(children, CommandType::update);
    };


assignings:
    assigning |
    assignings COMMA assigning;

assigning:
    id EQUAL constant {
        identList.push_back($1);
        constantList.push_back($3);
    };


// --- delete

delete:
    DELETE FROM id where_condition {
        std::map<NodeType, Node*> children;
        children[NodeType::ident] = $3;
        children[NodeType::ident_list] = new IdentList(identList);
        children[NodeType::constant_list] = new ConstantList(constantList);
        children[NodeType::relation] = $4;

        parseTree = new Query(children, CommandType::remove);
    };

// ---

constant:
    int_const { $$ = $1; } |
    real_const { $$ = $1; } |
    text_const { $$ = $1; } |
    null_ { $$ = $1; };

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

null_:
    NULL_ {
        $$ = new NullConstant();
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
    identList.clear();
    selectList.clear();
    constantList.clear();
    ex = nullptr;

    parseTree = nullptr;
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