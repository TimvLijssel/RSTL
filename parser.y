%{
	#include "node.h"
        #include <cstdio>
        #include <cstdlib>
	NBlock *programBlock; /* aller hoogste blok */

	extern int yylex();
	extern int yylineno;
	void yyerror(const char *s) { std::printf("Error rond lijn %d: %s\n",yylineno, s);std::exit(1); }
%}

/* basis nodes */
%union {
	Node *node;
	NBlock *block;
	NExpression *expr;
	NStatement *stmt;
	NIdentifier *ident;
	NVariableDeclaration *var_decl;
	std::vector<NVariableDeclaration*> *varvec;
	std::vector<NExpression*> *exprvec;
	std::string *string;
	int token;
}

/* tokens definieren, deze kloppen met de tokens uit de lexer
 */
%token <string> TIDENTIFIER TINTEGER TDOUBLE TBOOLWAAR TBOOLONWAAR /*TSTRING*/
%token <token> TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TCOMMA TDOT TQUOTE TENTER
%token <token> TPLUS TMINUS TMUL TDIV
%token <token> TRETURN TIF
%token <token> TAND TOR TXOR

/* Tussenstappen voor de AST aanmaken. 
 * Uiteindelijk wordt alles een token,
 * maar het gaat vaak via een type.
 */
%type <ident> ident
%type <expr> numeric expr boolean /*string*/
%type <varvec> func_decl_args
%type <exprvec> call_args
%type <block> program stmts block
%type <stmt> stmt var_decl func_decl if_stmt
%type <token> comparison

%left TPLUS TMINUS
%left TMUL TDIV

%start program

%%

program : stmts { programBlock = $1; }
		;
		
stmts : TENTER { $$ = new NBlock(); }
	  | stmt { $$ = new NBlock(); $$->statements.push_back($<stmt>1); }
	  | stmts TENTER stmt { $1->statements.push_back($<stmt>2); }
	  | stmts TENTER
	  | TENTER stmts { $$ = $2; }
	  ;

stmt : var_decl | func_decl | if_stmt
	 | expr { $$ = new NExpressionStatement(*$1); }
	 | TRETURN expr { $$ = new NReturnStatement(*$2); }
     ;

block : TLBRACE stmts TRBRACE { $$ = $2; }
	  | TLBRACE TRBRACE { $$ = new NBlock(); }
	  | TENTER block { $$ = $2; }
	  ;

var_decl : ident ident { $$ = new NVariableDeclaration(*$1, *$2); }
		 | ident ident TEQUAL expr { $$ = new NVariableDeclaration(*$1, *$2, $4); }
		 ;

func_decl : ident ident TLPAREN func_decl_args TRPAREN block 
			{ $$ = new NFunctionDeclaration(*$1, *$2, *$4, *$6); delete $4; }
		  ;
	
func_decl_args : /*blank*/  { $$ = new VariableList(); }
		  | var_decl { $$ = new VariableList(); $$->push_back($<var_decl>1); }
		  | func_decl_args TCOMMA var_decl { $1->push_back($<var_decl>3); }
		  ;

if_stmt : TIF TLPAREN expr TRPAREN block { $$ = new NIfStatement(*$3,*$5); }
	;

ident : TIDENTIFIER { $$ = new NIdentifier(*$1); delete $1; }
	  ;

numeric : TINTEGER { $$ = new NInteger(atol($1->c_str())); delete $1; }
		| TMINUS TINTEGER { $$ = new NInteger(-atol($2->c_str())); delete $2; }
		| TDOUBLE { $$ = new NDouble(atof($1->c_str())); delete $1; }
		;

boolean : TBOOLWAAR { $$ = new NBoolean(true); }
		| TBOOLONWAAR { $$ = new NBoolean(false); }
		;

/*string : TSTRING { $$ = new NString($1); delete $1; }
		;*/
	
expr : ident TEQUAL expr { $$ = new NAssignment(*$<ident>1, *$3); }
	 | ident TLPAREN call_args TRPAREN { $$ = new NMethodCall(*$1, *$3); delete $3; }
	 | ident { $<ident>$ = $1; }
	 | numeric
	 | boolean
         | expr TMUL expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
         | expr TDIV expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
         | expr TPLUS expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
         | expr TMINUS expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
 	 | expr comparison expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | TLPAREN expr TRPAREN { $$ = $2; }
	 ;
	
call_args : /*blank*/  { $$ = new ExpressionList(); }
		  | expr { $$ = new ExpressionList(); $$->push_back($1); }
		  | call_args TCOMMA expr  { $1->push_back($3); }
		  ;

comparison : TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE | TAND | TOR | TXOR;

%%
