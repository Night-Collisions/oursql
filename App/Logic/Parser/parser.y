%{	#include<stdio.h>	#include<stdlib.h>	int yylex();	int yyerror(char *s);%}	%token STRING NUM OTHER SEMICOLON DDLSTRING TABLE%type <name> STRING%type <number> NUM%type <name> DDLSTRING%type <name> TABLE%union{	char name[20];	int number; }%%prog:	stmts;stmts:	| request stmts	request:	create_table SEMICOLON create_table: 	DDLSTRING TABLE STRING{		printf("%s TABLE %s", $1, $3);	}	| DDLSTRING{		printf("CREATE!!");	} 	| NUM{		printf("YOUR NUM is - %d", $1);	}	| STRING{		printf("STRING KEK %s", $1);	}	| OTHER;%%	int yyerror(char *s){	printf("Syntax Error on line %s\n", s);	return 0;}