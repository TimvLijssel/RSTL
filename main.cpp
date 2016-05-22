#include <iostream>
#include "codegen.h"
#include "node.h"

using namespace std;

extern int yyparse();
extern NBlock* programBlock;

void createCoreFunctions(CodeGenContext& context);

int main(int argc, char **argv)
{
	yyparse();
	cout << programBlock << endl;
    	// LLVM initaliseren
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();
	CodeGenContext context;
	// print functie aanmaken
	createCoreFunctions(context);
	// code zelf aanmaken
	context.generateCode(*programBlock);
	// en draaien
	context.runCode();
	
	return 0;
}

