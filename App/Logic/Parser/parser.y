%{
    #include "../../App/Logic/Parser/Nodes/Node.h"
    #include "../../App/Logic/Parser/Nodes/Query.h"
    #include "../../App/Logic/Parser/Nodes/VarList.h"
    #include "../../App/Logic/Parser/Nodes/Variable.h"
    #include "../../App/Logic/Parser/Nodes/Ident.h"
    #include "../../App/Logic/Parser/Nodes/IntConstant.h"
    #include "../../App/Logic/Parser/Nodes/Constant.h"
    #include "../../App/Logic/Parser/Nodes/RealConstant.h"
    #include "../../App/Logic/Parser/Nodes/TextConstant.h"
    #include "../../App/Logic/Parser/Nodes/ConstantList.h"
    #include "../../App/Logic/Parser/Nodes/IdentList.h"
    #include "../../App/Logic/Parser/Nodes/Relation.h"
    #include "../../App/Logic/Parser/Nodes/NullConstant.h"
    #include "../../App/Logic/Parser/Nodes/SelectList.h"
    #include "../../App/Logic/Parser/Nodes/Expression.h"
    #include "../../App/Core/Exception.h"

    #include "../../App/Engine/Engine.h"
    #include "../../App/Engine/Column.h"

    #include <stdio.h>
    #include <string>
    #include <cstring>
    #include <iostream>
    #include <map>
    #include <sstream>

    #define VARCHAR_MAX_LEN (1024)

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
    std::vector<Node*> selectList;

    std::unique_ptr<exc::Exception> ex;
%}

%error-verbose

%token CREATE SHOW DROP SELECT INSERT UPDATE DELETE
%token TABLE TABLES VALUES INTO FROM WHERE SET
%token LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE SEMI DOT COMMA ASTERISK
%token EQUAL GREATER LESS GREATER_EQ LESS_EQ NOT_EQ
%token ID ICONST FCONST SCONST
%token INT REAL VARCHAR
%token NOT_NULL PRIMARY_KEY UNIQUE NULL_
%token AND OR DIVIDE PLUS MINUS NOT

%type<query> create show_create drop_table select insert
%type<ident> id 
%type<var> variable
%type<dataType> type
%type<constraint> constraint
%type<iConst> int_const
%type<rConst> real_const
%type<tConst> text_const
%type<nullConst> null_
%type<anyConstant> constant where_element select_list_element val_or_var
%type<exprUnit> logic_oper plus_minus mul_div relations
%type<expr> where_expr root_expr relation_expr exprssn term factor 

%start start_expression

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
    ExprUnit exprUnit;
    Expression *expr;

    int varcharLen;
}

%%

start_expression:
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
        if ($2 == DataType::varchar) {
            $$->addVarcharLen(yylval.varcharLen);
        }
    } | id type constraints {
        $$ = new Variable($1->getName(), $2, constraintList);
        if ($2 == DataType::varchar) {
            $$->addVarcharLen(yylval.varcharLen);
        }
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
     SELECT select_decl FROM id where_expr {
        std::map<NodeType, Node*> children;
        children[NodeType::ident] = $4;
        children[NodeType::select_list] = new SelectList(selectList);
        children[NodeType::expression] = $5;

        parseTree = new Query(children, CommandType::select);
     } ;

select_decl:
    asterisk | 
    asterisk COMMA select_list | 
    select_list;

select_list:
    select_list_element {
        selectList.push_back($1);
    } |
    select_list COMMA select_list_element {
        selectList.push_back($3);
    };

asterisk:
    ASTERISK {
    	selectList.push_back(new Ident("*"));
    };

select_list_element:
    id DOT id {
        $$ = new Ident((*$1).getName(), (*$3).getName());
    } |
    id {
        $$ = $1;
    } |
    constant {
        $$ = $1;
    } |
    exprssn {
        $$ = $1;
    };


where_expr:
    WHERE root_expr {
        $$ = $2;
    } | 
    /*empty*/ { $$ = nullptr; };
    
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
    UPDATE id SET assignings where_expr {
        std::map<NodeType, Node*> children;
        children[NodeType::ident] = $2;
        children[NodeType::ident_list] = new IdentList(identList);
        children[NodeType::constant_list] = new ConstantList(constantList);
        children[NodeType::expression] = $5;

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
    DELETE FROM id where_expr {
        std::map<NodeType, Node*> children;
        children[NodeType::ident] = $3;
        children[NodeType::ident_list] = new IdentList(identList);
        children[NodeType::constant_list] = new ConstantList(constantList);
        children[NodeType::expression] = $4;

        parseTree = new Query(children, CommandType::remove);
    };


root_expr: 
    root_expr logic_oper relation_expr {
        $$ = new Expression($1, $2, $3);
    } |
    relation_expr {
        $$ = $1;
    };

relation_expr: 
    relation_expr relations exprssn {
        $$ = new Expression($1, $2, $3);
    } | 
    exprssn {
        $$ = $1;
    };

exprssn: 
    exprssn plus_minus term {
        $$ = new Expression($1, $2, $3);
    } | 
    term {
        $$ = $1;
    };

term: 
    term mul_div factor {
        $$ = new Expression($1, $2, $3);
    } | 
    factor {
        $$ = $1;
    };

factor: 
    LPAREN root_expr RPAREN {
        $$ = $2;
    } | 
    NOT root_expr {
        $$ = new Expression(new Expression(new Ident("")), ExprUnit::not_, $2);
    } |
    MINUS factor {
        $$ = new Expression(new Expression(new IntConstant("0")), ExprUnit::sub, $2);
    } | 
    val_or_var {
        $$ = new Expression($1);
    };

relations: 
    EQUAL { $$ = ExprUnit::equal; } |
    NOT_EQ { $$ = ExprUnit::not_equal; } |
    GREATER { $$ = ExprUnit::greater; } |
    GREATER_EQ { $$ = ExprUnit::greater_eq; } |
    LESS { $$ = ExprUnit::less; } |
    LESS_EQ { $$ = ExprUnit::less_eq; };

plus_minus: 
    PLUS { $$ = ExprUnit::add; } |
    MINUS { $$ = ExprUnit::sub; };

mul_div:
    DIVIDE { $$ = ExprUnit::div; } |
    ASTERISK { $$ = ExprUnit::mul; }

val_or_var:
    constant { $$ = $1; } |
    id { $$ = $1; } |
    id DOT id { $$ = new Ident($1->getName(), $3->getName()); };

logic_oper:
    AND { $$ = ExprUnit::and_; } |
    OR { $$ = ExprUnit::or_; };

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
    VARCHAR { $$ = DataType::varchar; };

id:
    ID { $$ = new Ident(*yylval.name); }

%%

void yyerror(const char *s) {
    fprintf(stderr, "%s\n", s);
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
    yylval.varcharLen = 0;

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