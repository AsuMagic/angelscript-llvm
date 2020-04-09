#include <angelscript-llvm/detail/builder.hpp>

#include <angelscript-llvm/detail/asinternalheaders.hpp>
#include <angelscript-llvm/detail/assert.hpp>
#include <angelscript-llvm/detail/jitcompiler.hpp>
#include <angelscript-llvm/detail/llvmglobals.hpp>
#include <angelscript-llvm/detail/modulecommon.hpp>
#include <angelscript.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

namespace asllvm::detail
{
Builder::Builder(JitCompiler& compiler) :
	m_context{setup_context()},
	m_compiler{compiler},
	m_pass_manager{setup_pass_manager()},
	m_ir_builder{*m_context},
	m_defs{setup_common_definitions()}
{}

llvm::Type* Builder::to_llvm_type(asCDataType& type) const
{
	if (type.IsPrimitive())
	{
		llvm::Type* base_type = nullptr;

		switch (type.GetTokenType())
		{
		case ttVoid: base_type = m_defs.tvoid; break;
		case ttBool: base_type = m_defs.i1; break;
		case ttInt8:
		case ttUInt8: base_type = m_defs.i8; break;
		case ttInt16:
		case ttUInt16: base_type = m_defs.i16; break;
		case ttInt:
		case ttUInt: base_type = m_defs.i32; break;
		case ttInt64:
		case ttUInt64: base_type = m_defs.i64; break;
		default: asllvm_assert(false && "provided primitive type not supported");
		}

		if (type.IsReference())
		{
			return base_type->getPointerTo();
		}

		return base_type;
	}

	if (type.IsReference() || type.IsObjectHandle())
	{
		return m_defs.pvoid;
	}

	if (type.IsObject())
	{
		std::array<llvm::Type*, 1> types{{llvm::ArrayType::get(m_defs.i8, type.GetSizeInMemoryBytes())}};
		asllvm_assert(type.GetTypeInfo() != nullptr);
		return llvm::StructType::create(types, type.GetTypeInfo()->GetName());
	}

	asllvm_assert(false && "type not supported");
}

bool Builder::is_script_type_64(asCDataType& type) const
{
	asllvm_assert(type.GetSizeOnStackDWords() <= 2);
	return type.GetSizeOnStackDWords() == 2;
}

llvm::legacy::PassManager& Builder::optimizer() { return m_pass_manager; }

llvm::LLVMContext& Builder::context() { return *m_context; }

std::unique_ptr<llvm::LLVMContext> Builder::extract_old_context()
{
	auto old_context = std::move(m_context);

	m_context = setup_context();
	m_defs    = setup_common_definitions();

	return old_context;
}

CommonDefinitions Builder::setup_common_definitions()
{
	CommonDefinitions defs;

	defs.tvoid = llvm::Type::getVoidTy(*m_context);
	defs.i1    = llvm::Type::getInt1Ty(*m_context);
	defs.i8    = llvm::Type::getInt8Ty(*m_context);
	defs.i16   = llvm::Type::getInt16Ty(*m_context);
	defs.i32   = llvm::Type::getInt32Ty(*m_context);
	defs.i64   = llvm::Type::getInt64Ty(*m_context);
	defs.iptr  = llvm::Type::getInt64Ty(*m_context); // TODO: determine pointer type from target machine

	defs.pvoid = llvm::Type::getInt8PtrTy(*m_context);
	defs.pi8   = llvm::Type::getInt8PtrTy(*m_context);
	defs.pi16  = llvm::Type::getInt16PtrTy(*m_context);
	defs.pi32  = llvm::Type::getInt32PtrTy(*m_context);
	defs.pi64  = llvm::Type::getInt64PtrTy(*m_context);

	{
		std::array<llvm::Type*, 8> types{{
			defs.pi32,  // programPointer
			defs.pi32,  // stackFramePointer
			defs.pi32,  // stackPointer
			defs.iptr,  // valueRegister
			defs.pvoid, // objectRegister
			defs.pvoid, // objectType - todo asITypeInfo
			defs.i1,    // doProcessSuspend
			defs.pvoid, // ctx - todo asIScriptContext
		}};

		defs.vm_registers = llvm::StructType::create(types, "asSVMRegisters");
	}

	return defs;
}

std::unique_ptr<llvm::LLVMContext> Builder::setup_context() { return std::make_unique<llvm::LLVMContext>(); }

llvm::legacy::PassManager Builder::setup_pass_manager()
{
	llvm::PassManagerBuilder pmb;
	pmb.OptLevel           = 3;
	pmb.Inliner            = llvm::createFunctionInliningPass(275);
	pmb.DisableUnrollLoops = false;
	pmb.LoopVectorize      = true;
	pmb.SLPVectorize       = true;

	llvm::legacy::PassManager pm;
	pm.add(llvm::createVerifierPass());
	pmb.populateModulePassManager(pm);
	pm.add(llvm::createVerifierPass());

	return pm;
}
} // namespace asllvm::detail
