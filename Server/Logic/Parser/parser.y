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
    #include "../../Server/Logic/Parser/Nodes/NullConstant.h"
    #include "../../Server/Logic/Parser/Nodes/SelectList.h"
    #include "../../Server/Logic/Parser/Nodes/Expression.h"
    #include "../../Server/Logic/Parser/Nodes/RelExpr.h"
    #include "../../Server/Logic/Parser/Nodes/IndexNode.h"
    #include "../../Server/Logic/Parser/Nodes/Transaction.h"
    #include "../../Server/Logic/Parser/Nodes/Period.h"
    #include "../../Server/Logic/Parser/Nodes/SysTime.h"
    #include "../../Server/Logic/Parser/Nodes/With.h"
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

    std::vector<Query*> rootQuery;
    std::vector<Query*> queryList;
    std::vector<Variable *> varList;
    std::vector<ColumnConstraint> constraintList;
    std::pair<std::string, std::string> period;
    std::vector<Ident*> identList;
    std::vector<Node*> constantList;
    std::vector<Node*> selectList;

    std::unique_ptr<exc::Exception> ex;
%}

%error-verbose

%token CREATE SHOW DROP SELECT INSERT UPDATE DELETE
%token TABLE TABLES VALUES INTO FROM WHERE SET INDEX
%token LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE SEMI DOT COMMA ASTERISK
%token EQUAL GREATER LESS GREATER_EQ LESS_EQ NOT_EQ
%token ID ICONST FCONST SCONST
%token INT REAL VARCHAR DATETIME
%token NOT_NULL PRIMARY_KEY UNIQUE NULL_
%token AND OR DIVIDE PLUS MINUS NOT
%token LEFT RIGHT INNER OUTER FULL CROSS JOIN INTERSECT UNION AS ON
%token BEGIN_ COMMIT
%token VERSIONING WITH PERIOD SYSTEM_TIME FOR_ OFF TO ALL

%type<query> create_table create_index show_create drop_table select insert delete update statement statements
%type<ident> id col_ident
%type<var> variable
%type<dataType> type
%type<constraint> constraint
%type<iConst> int_const
%type<rConst> real_const
%type<tConst> text_const
%type<relOperNodeType> join_opers union_intersect
%type<nullConst> null_
%type<anyConstant> constant where_element select_list_element val_or_var
%type<exprUnit> logic_or logic_and plus_minus mul_div relations
%type<expr> where_expr root_expr relation_expr exprssn term factor under_root_expr join_cond
%type<relExpr> sub_rel_expr relational_expr
%type<is_versioned> on_or_off
%type<withCond> with_expr
%type<sysTimeExpr> sys_time_cond

%start start_expression

%union {
    ColumnConstraint constraint;
    RelOperNodeType relOperNodeType;
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
    RelExpr *relExpr;
    With *withCond;
    SysTime *sysTimeExpr;

    int varcharLen;
    bool is_versioned;
}

%%

start_expression:
    statements {
        destroy();
    } |
    BEGIN_ SEMI statements COMMIT SEMI {
        destroy();
    };

statements:
    statement SEMI {
        varList.clear();
        queryList.push_back($1);
    } |
    statements statement SEMI {
        queryList.push_back($2);
    }

statement:
    create_table |
    show_create |
    drop_table |
    select |
    insert |
    update |
    delete |
    create_index;

// ---- create index

create_index:
    CREATE INDEX ON id LPAREN id RPAREN {
	std::map<NodeType, Node*> children;
     	children[NodeType::index] = new IndexNode($4->getName(), $6->getName());

     	$$ = new Query(children, CommandType::create_index);
    };

// ---- create table

create_table:
    CREATE TABLE id LPAREN variables RPAREN {
        std::map<NodeType, Node*> children;
        children[NodeType::ident] = $3;
        children[NodeType::var_list] = new VarList(varList);
        children[NodeType::period_pair] = new Period(period);

        $$ = new Query(children, CommandType::create_table);
    };

with_expr: WITH LPAREN VERSIONING EQUAL on_or_off RPAREN { $$ = new With($5); }  | /*EMPTY*/;

on_or_off: ON { $$ = true; } | OFF { $$ = false; };


variables:
    variable {
        //varList.push_back($1);
    } |
    variables COMMA variable {
        //varList.push_back($3);
    };

variable:
    id type {
        $$ = new Variable($1->getName(), $2);
        if ($2 == DataType::varchar) {
            $$->addVarcharLen(yylval.varcharLen);
        }
        varList.push_back($$);
    } | id type constraints {
        $$ = new Variable($1->getName(), $2, constraintList);
        if ($2 == DataType::varchar) {
            $$->addVarcharLen(yylval.varcharLen);
        }
        varList.push_back($$);
        constraintList.clear();
    } | PERIOD FOR_ SYSTEM_TIME LPAREN id COMMA id RPAREN {
        period.first = $5->getName();
        period.second = $7->getName();
    }

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
    SELECT select_decl FROM relational_expr where_expr {
         std::map<NodeType, Node*> children;
         children[NodeType::select_list] = new SelectList(selectList);
         children[NodeType::relational_oper_expr] = $4;
         children[NodeType::expression] = $5;

         $$ = new Query(children, CommandType::select);
    } |
    SELECT select_decl FROM id where_expr {
        std::map<NodeType, Node*> children;
        children[NodeType::ident] = $4;
        children[NodeType::select_list] = new SelectList(selectList);
        children[NodeType::expression] = $5;

        $$ = new Query(children, CommandType::select);
    } |
    SELECT select_decl FROM id FOR_ SYSTEM_TIME sys_time_cond {
        std::map<NodeType, Node*> children;
        children[NodeType::ident] = $4;
        children[NodeType::select_list] = new SelectList(selectList);
        children[NodeType::sys_time] = $7;

        $$ = new Query(children, CommandType::select);
    };

sys_time_cond: ALL {
        $$ = new SysTime();
    } | 
    FROM text_const TO text_const {
        auto s1 = $2->getValue();
        auto s2 = $4->getValue();
        delete $2;
        delete $4;

        $$ = new SysTime(s1, s2, ex);
    };

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
    	selectList.push_back(new Expression(new Ident("*")));
    };

select_list_element:
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

// --- show create table

show_create:
    SHOW CREATE TABLE id {
        std::map<NodeType, Node*> children;
        children[NodeType::ident] = $4;

        $$ = new Query(children, CommandType::show_create_table);
    };

// --- drop table

drop_table:
    DROP TABLE id {
        std::map<NodeType, Node*> children;
        children[NodeType::ident] = $3;

        $$ = new Query(children, CommandType::drop_table);
    };

// --- insert

insert:
    INSERT INTO id column_decl VALUES LPAREN value_list RPAREN {
        std::map<NodeType, Node*> children;
        children[NodeType::ident] = $3;
        children[NodeType::ident_list] = new IdentList(identList);
        children[NodeType::constant_list] = new ConstantList(constantList);

        $$ = new Query(children, CommandType::insert);
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

        $$ = new Query(children, CommandType::update);
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

        $$ = new Query(children, CommandType::remove);
    };


root_expr: 
    root_expr logic_or under_root_expr {
        $$ = new Expression($1, $2, $3);
    } |
    under_root_expr {
        $$ = $1;
    };

under_root_expr:
    under_root_expr logic_and relation_expr {
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
    NOT factor {
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
    col_ident { $$ = $1; };

logic_or:
        OR { $$ = ExprUnit::or_; };

logic_and:
    AND { $$ = ExprUnit::and_; };

constant:
    int_const { $$ = $1; } |
    real_const { $$ = $1; } |
    text_const { $$ = $1; } |
    null_ { $$ = $1; };

int_const:
    ICONST {
        $$ = yylval.iConst;
    } | 
    MINUS ICONST {
        $$ = new IntConstant("-" + yylval.iConst->getValue());
    };

real_const:
    FCONST {
	    $$ = yylval.rConst;
    } |
    MINUS FCONST {
        $$ = new RealConstant("-" + yylval.rConst->getValue());
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
    VARCHAR { $$ = DataType::varchar; } | 
    DATETIME { $$ = DataType::datetime; };

col_ident: 
    id { $$ = $1; } |
    id DOT col_ident { 
        if ( $3->getTableName().empty()) {
            $$ = new Ident($1->getName(), $3->getName()); 
        } else {
            $$ = new Ident($1->getName(), $3->getTableName() + "." + $3->getName()); 
        }
    };

id:
    ID { $$ = new Ident(*yylval.name); };

sub_rel_expr:
    id { $$ = new RelExpr($1, ""); } |
    id AS id { $$ = new RelExpr($1, $3->getName()); } |
    LPAREN id AS id RPAREN { $$ = new RelExpr($2, $4->getName()); } |
    LPAREN relational_expr RPAREN AS id { 
        $2->setAlias($5->getName());
        $$ = $2;
    } /*|
    relational_expr {
        $$ = $1;
    };*/

relational_expr:
    relational_expr join_opers sub_rel_expr join_cond {
        $$ = new RelExpr($1, $3, $2, $4);
    } |
    relational_expr union_intersect sub_rel_expr {
        $$ = new RelExpr($1, $3, $2, nullptr);
    } |
    sub_rel_expr { $$ = $1; };

join_opers:
    JOIN { $$ = RelOperNodeType::join; } |
    LEFT JOIN { $$ = RelOperNodeType::left_join; } |
    RIGHT JOIN { $$ = RelOperNodeType::right_join; } |
    INNER JOIN { $$ = RelOperNodeType::inner_join; } |
    FULL JOIN { $$ = RelOperNodeType::full_join; };

union_intersect:
    UNION { $$ = RelOperNodeType::union_; } |
    INTERSECT { $$ = RelOperNodeType::intersect; };

join_cond:
    ON root_expr { $$ = $2; } |
    /*empty*/ { $$ = nullptr; };

%%

void yyerror(const char *s) {
    fprintf(stderr, "\n%s\n", s);
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
    yylval.varcharLen = 0;
}

std::vector<Query*> parse_string(const char* in, std::unique_ptr<exc::Exception>& exception) {
    destroy();
    exception.reset(nullptr);

    set_input_string(in);
    yyparse();
    end_lexical_scan();

    exception = std::move(ex);
    return std::move(queryList);
}