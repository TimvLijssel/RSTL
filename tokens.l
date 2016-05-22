%{
#include <string>
#include "node.h"
#include "parser.hpp"

// Handige functies voor het opslaan van tokens en strings
#define SAVE_TOKEN  yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t)    (yylval.token = t)
%}

%option noyywrap

%%
// Spaties en tabs negeren
[ \t\n]                   ;
//Woorden herkenen
"teruggave"               return TOKEN(TRETURN);
"als"                     return TOKEN(TIF);
"onwaar"                  return TOKEN(TBOOLONWAAR);
"waar"                    return TOKEN(TBOOLWAAR);
"en"                      return TOKEN(TAND);
"of"                      return TOKEN(TOR);
"exof"                    return TOKEN(TXOR);
// Alle andere woorden zijn identifiers
[a-zA-Z_][a-zA-Z0-9_]*    SAVE_TOKEN; return TIDENTIFIER;
[0-9]+\,[0-9]*            SAVE_TOKEN; return TDOUBLE;
[0-9]+                    SAVE_TOKEN; return TINTEGER;
// String staat uit
  /*"\""[^\"]*"\""        SAVE_TOKEN; return TSTRING;*/

// Tokens, waarvan er vrij veel ongebruikt zijn 
"="                       return TOKEN(TEQUAL);
"=="                      return TOKEN(TCEQ);
"!="                      return TOKEN(TCNE);
"<"                       return TOKEN(TCLT);
"<="                      return TOKEN(TCLE);
">"                       return TOKEN(TCGT);
">="                      return TOKEN(TCGE);

"("                       return TOKEN(TLPAREN);
")"                       return TOKEN(TRPAREN);
"{"                       return TOKEN(TLBRACE);
"}"                       return TOKEN(TRBRACE);

"\""                      return TOKEN(TQUOTE);

"."                       return TOKEN(TDOT);
","                       return TOKEN(TCOMMA);

"+"                       return TOKEN(TPLUS);
"-"                       return TOKEN(TMINUS);
"*"                       return TOKEN(TMUL);
"/"                       return TOKEN(TDIV);

// Commentaar eruit halen
"%-".*                    /* Commentaar */

// En al het andere kennen we niet
.                         printf("We kennen %s niet!\n", yytext); yyterminate();

%%
