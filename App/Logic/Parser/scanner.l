%{
	#include <stdio.h>
	#include <string.h>
	#include "parser.hpp"
	void showError();
%}
%option noinput noyywrap nounput
numbers ([0-9])+
alpha	([a-zA-Z])+
ddlstring "CREATE"|"DELETE"

%%
{ddlstring}	{sscanf(yytext, "%s", yylval.name); return(DDLSTRING); }
"TABLE"		{return(TABLE); }
{alpha} 	{sscanf(yytext, "%s", yylval.name); return (STRING);}
{numbers}	{yylval.number = atoi(yytext); return (NUM);}
";"			{return (SEMICOLON);}

%%

void showError(){
	printf("Other input");
}