#include "node.h"
#include "codegen.h"
#include "parser.hpp"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;
using namespace std;

static IRBuilder<> Builder(getGlobalContext());

/* AST naar module */
void CodeGenContext::generateCode(NBlock& root)
{
	std::cout << "Code genereren...\n";
	
	vector<Type*> argTypes;
	FunctionType *ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), makeArrayRef(argTypes), false);
	mainFunction = Function::Create(ftype, GlobalValue::InternalLinkage, "main", module);
	BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", mainFunction, 0);
	
	/* Het hoofd block */
	pushBlock(bblock);
	root.codeGen(*this); /* hoofd block genereren */
	ReturnInst::Create(getGlobalContext(), bblock);
	popBlock();
	
	std::cout << "Klaar met genereren!\n";
	PassManager<Module> pm;
	pm.addPass(PrintModulePass(outs()));
	pm.run(*module);
}

GenericValue CodeGenContext::runCode() {
	std::cout << "Code aan het draaien...\n";
	ExecutionEngine *ee = EngineBuilder( unique_ptr<Module>(module) ).create();
	ee->finalizeObject();
	vector<GenericValue> noargs;
	GenericValue v = ee->runFunction(mainFunction, noargs);
	std::cout << "Klaar met draaien.\n";
	return v;
}

static Type *typeOf(const NIdentifier& type) 
{
	if (type.name.compare("geh") == 0) {
		return Type::getInt64Ty(getGlobalContext());
	}
	else if (type.name.compare("rat") == 0) {
		return Type::getDoubleTy(getGlobalContext());
	}
	else if (type.name.compare("boo") == 0) {
		return Type::getInt1Ty(getGlobalContext());
	}
	else if (type.name.compare("tkr") == 0) {
		std::cout << "String gevonden!" << endl;
		std::cout << "String wordt niet ondersteund!" << endl;
		std::cout << "Een leeg object wordt gebruikt" << endl;
		return Type::getVoidTy(getGlobalContext());
		/*Type* I = IntegerType::getInt32Ty(getGlobalContext());
		std::cout << "De lengte wordt: " << I.getBitWidth() << " en natuurlijk  " << I.getBitWidth()/32 <<endl;
		ArrayType* arrayType = ArrayType::get(I, I.getBitWidth()/32);
		return arrayType;*/
	}
	return Type::getVoidTy(getGlobalContext());
}

/* -- Code aanmaken -- */

Value* NInteger::codeGen(CodeGenContext& context)
{
	std::cout << "Geheel getal aanmaken: " << value << endl;
	return ConstantInt::get(Type::getInt64Ty(getGlobalContext()), value, true);
}

/*Value* NString::codeGen(CodeGenContext& context)
{
	std::cout << "String aanmaken :O : " << value << endl;
	llvm::Value* v = llvm::ConstantArray::get(getGlobalContext(), value.c_str());
	return v;
}*/

Value* NDouble::codeGen(CodeGenContext& context)
{
	std::cout << "Rationeel getal aanmaken: " << value << endl;
	return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), value);
}

Value* NBoolean::codeGen(CodeGenContext& context)
{
	std::cout << "Boolean aanmaken: " << value << endl;
	//return ConstantInt::get(Type::getInt1Ty(getGlobalContext()), value, true); // Ik denk dat het Int1 moet zijn voor een 0 of 1
	return value ? ConstantInt::getTrue(getGlobalContext()) : ConstantInt::getFalse(getGlobalContext());
	
}

Value* NIdentifier::codeGen(CodeGenContext& context)
{
	std::cout << "Identifier referentie aanmaken: " << name << endl;
	if (context.locals().find(name) == context.locals().end()) {
		std::cerr << "niet gedeclareerde variable: " << name << endl;
		return NULL;
	}
	return new LoadInst(context.locals()[name], "", false, context.currentBlock());
}

Value* NMethodCall::codeGen(CodeGenContext& context)
{
	Function *function = context.module->getFunction(id.name.c_str());
	if (function == NULL) {
		std::cerr << "functie niet gevonden: " << id.name << endl;
	}
	std::vector<Value*> args;
	ExpressionList::const_iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++) {
		args.push_back((**it).codeGen(context));
	}
	CallInst *call = CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
	std::cout << "Functie aanroep aanmaken: " << id.name << endl;
	return call;
}

Value* NBinaryOperator::codeGen(CodeGenContext& context)
{
	std::cout << "Binaire operatie aanmaken " << op << endl;
	Instruction::BinaryOps instr;
	switch (op) {
		case TPLUS: 	instr = Instruction::Add; goto math;
		case TMINUS: 	instr = Instruction::Sub; goto math;
		case TMUL: 		instr = Instruction::Mul; goto math;
		case TDIV: 		instr = Instruction::SDiv; goto math;
				
		case TAND: 	instr = Instruction::And; goto math;
		case TOR: 		instr = Instruction::Or; goto math;
		case TXOR: 		instr = Instruction::Xor; goto math;
		
		default: assert(0);
	}

	return NULL;
math:
	return BinaryOperator::Create(instr, lhs.codeGen(context), 
		rhs.codeGen(context), "", context.currentBlock());
}

Value* NAssignment::codeGen(CodeGenContext& context)
{
	std::cout << "Toewijzing aanmaken " << lhs.name << endl;
	if (context.locals().find(lhs.name) == context.locals().end()) {
		std::cerr << "niet gedeclareerde variable: " << lhs.name << endl;
		return NULL;
	}
	return new StoreInst(rhs.codeGen(context), context.locals()[lhs.name], false, context.currentBlock());
}

Value* NBlock::codeGen(CodeGenContext& context)
{
	StatementList::const_iterator it;
	Value *last = NULL;
	for (it = statements.begin(); it != statements.end(); it++) {
		std::cout << "Code aanmaken voor " << typeid(**it).name() << endl;
		last = (**it).codeGen(context);
	}
	std::cout << "Blok aanmaken" << endl;
	return last;
}

Value* NExpressionStatement::codeGen(CodeGenContext& context)
{
	std::cout << "Code aanmaken voor" << typeid(expression).name() << endl;
	return expression.codeGen(context);
}

Value* NReturnStatement::codeGen(CodeGenContext& context)
{
	std::cout << "Terugvoer aanmaken voor " << typeid(expression).name() << endl;
	Value *returnValue = expression.codeGen(context);
	context.setCurrentReturnValue(returnValue);
	return returnValue;
}

Value* NVariableDeclaration::codeGen(CodeGenContext& context)
{
	std::cout << "Variable declaratie aanmaken " << type.name << " " << id.name << endl;
	AllocaInst *alloc = new AllocaInst(typeOf(type), id.name.c_str(), context.currentBlock());
	context.locals()[id.name] = alloc;
	if (assignmentExpr != NULL) {
		NAssignment assn(id, *assignmentExpr);
		assn.codeGen(context);
	}
	return alloc;
}

Value* NFunctionDeclaration::codeGen(CodeGenContext& context)
{
	vector<Type*> argTypes;
	VariableList::const_iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++) {
		argTypes.push_back(typeOf((**it).type));
	}
	FunctionType *ftype = FunctionType::get(typeOf(type), makeArrayRef(argTypes), false);
	Function *function = Function::Create(ftype, GlobalValue::InternalLinkage, id.name.c_str(), context.module);
	BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", function, 0);

	context.pushBlock(bblock);

	Function::arg_iterator argsValues = function->arg_begin();
    Value* argumentValue;

	for (it = arguments.begin(); it != arguments.end(); it++) {
		(**it).codeGen(context);
		
		argumentValue = argsValues++;
		argumentValue->setName((*it)->id.name.c_str());
		StoreInst *inst = new StoreInst(argumentValue, context.locals()[(*it)->id.name], false, bblock);
	}
	
	block.codeGen(context);
	ReturnInst::Create(getGlobalContext(), context.getCurrentReturnValue(), bblock);

	context.popBlock();
	std::cout << "Functie aanmaken: " << id.name << endl;
	return function;
}

Value* NIfStatement::codeGen(CodeGenContext& context)
{
	std::cout << "If-statement aanmaken (WERKT NIET!)" << endl;
	Value *CondV = condition.codeGen(context);
	if (!CondV)
		return nullptr;
	
	// Convert condition to a bool by comparing equal to 0.0.
	/*CondV = Builder.CreateFCmpONE(
		CondV, ConstantFP::get(getGlobalContext(),APFloat(0.0)), "ifcond");*/
	
	/*Function *TheFunction = Builder.GetInsertBlock()->getParent();*/
	
	// Create block for the then block.
	BasicBlock *ThenBB = BasicBlock::Create(getGlobalContext(), "then"/*, TheFunction*/);
	BasicBlock *MergeBB = BasicBlock::Create(getGlobalContext(), "ifcont");
	BasicBlock *EmptyBB = BasicBlock::Create(getGlobalContext());
	
	Builder.CreateCondBr(CondV, ThenBB, EmptyBB);
	
	// Emit then value.
	Builder.SetInsertPoint(ThenBB);
	
	Value *ThenV = then.codeGen(context);
	if (!ThenV)
		return nullptr;
		
	Builder.CreateBr(MergeBB);
	// Codegen of 'Then' can change the current block, update ThenBB for the PHI.
	ThenBB = Builder.GetInsertBlock();
	
	// Emit merge block.
	/*TheFunction->getBasicBlockList().push_back(MergeBB);*/
	Builder.SetInsertPoint(MergeBB);
	/*PHINode *PN = Builder.CreatePHI(Type::getDoubleTy(getGlobalContext()), 2, "iftmp");
	
	PN->addIncoming(ThenV, ThenBB);
	return PN;*/
	return ThenV;
}

