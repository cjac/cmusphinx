%{ /* -*- mode: text -*- */

#include "jsgf.h"
#include "jsgf.tab.h"

%}

%s COMMENT

ws       [ \t\n]
rulename \<[^<>]+\>
tag      \{(\\.|[^\}]+)*\}
weight   \/[0-9]*(\.[0-9]+)?\/
token    [^ \t\n;=|*+<>()\[\]{}*/]+
qstring  \"(\\.|[^"]+)*\"

%%

{ws} ;                     /* ignore whitespace */
\/\/.*\n ;                 /* ignore comments */
\/\* { BEGIN COMMENT; }
<COMMENT>\*\/ { BEGIN 0; }
<COMMENT>. ;               /* Ignore stuff in comment mode */

#JSGF return HEADER;
grammar return GRAMMAR;
import return IMPORT;
public return PUBLIC;

{rulename} { yylval.name = strdup(yytext); return RULENAME; }
{tag}      { yylval.name = strdup(yytext); return TAG; }
{token}    { yylval.name = strdup(yytext); return TOKEN; }
{qstring}  { yylval.name = strdup(yytext); return TOKEN; }
{weight}   { yylval.weight = atof(yytext+1); return WEIGHT; }

. return yytext[0];        /* Single-character tokens */

%%
