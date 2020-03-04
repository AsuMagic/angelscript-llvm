#pragma once

#include <llvm/IR/Function.h>

namespace asllvm::detail
{

class ModuleBuilder;

class FunctionBuilder
{
public:
	FunctionBuilder(ModuleBuilder& module_builder, llvm::Function* function);

private:
	ModuleBuilder* m_module_builder;
	llvm::Function* m_function;
};

}
