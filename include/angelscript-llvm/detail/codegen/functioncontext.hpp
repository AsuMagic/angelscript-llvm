#pragma once

#include <angelscript-llvm/detail/asinternalheaders.hpp>
#include <angelscript-llvm/detail/fwd.hpp>
#include <llvm/IR/Function.h>

namespace asllvm::detail::codegen
{
struct FunctionContext
{
	JitCompiler*             compiler;
	ModuleBuilder*           module_builder;
	llvm::Function*          llvm_function;
	const asCScriptFunction* script_function;
};
} // namespace asllvm::detail::codegen