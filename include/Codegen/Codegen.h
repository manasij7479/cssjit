#ifndef CSSJIT_CODEGEN_H
#define CSSJIT_CODEGEN_H

#include <map>
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

using namespace llvm;
namespace cssjit {
class Codegen {
public:
  Codegen(std::map<int, int> data_)
  : data(data_), Builder(TheContext) {
    TheModule = llvm::make_unique<Module>("main", TheContext);
    std::vector<llvm::Type*> args = {};
    auto FT = FunctionType::get(llvm::Type::getInt64Ty(TheContext), args, false);
    Function::Create(FT, Function::ExternalLinkage, "input", TheModule.get());

  }
  llvm::Module *operator()(std::string filename) {
    std::vector<llvm::Type*> args = {};
    auto FT = FunctionType::get(llvm::Type::getInt64Ty(TheContext), args, false);

    Function *F =
    Function::Create(FT, Function::ExternalLinkage,
                     "cssjit_main", TheModule.get());

    auto EntryBlock = BasicBlock::Create(TheContext, "entry", F);
    Builder.SetInsertPoint(EntryBlock);
    Builder.CreateRetVoid();

    std::error_code EC;
    llvm::raw_fd_ostream out(filename, EC);
    TheModule->print(out, nullptr);

    return TheModule.get();
  }
private:
  std::map<int, int> data;
  LLVMContext TheContext;
  IRBuilder<> Builder;
  std::unique_ptr<Module> TheModule;
  std::map<std::string, Value *> NamedValues;
};
}

#endif
