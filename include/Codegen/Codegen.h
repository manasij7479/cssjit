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

    args = {llvm::Type::getInt64Ty(TheContext)};
    FT = FunctionType::get(llvm::Type::getVoidTy(TheContext), args, false);
    Function::Create(FT, Function::ExternalLinkage, "printint", TheModule.get());
  }
  llvm::Module *operator()(std::string filename) {
    std::vector<llvm::Type*> args = {};
    auto FT = FunctionType::get(llvm::Type::getVoidTy(TheContext), args, false);

    Function *F =
    Function::Create(FT, Function::ExternalLinkage,
                     "cssjit_main", TheModule.get());

    auto EntryBlock = BasicBlock::Create(TheContext, "entry", F);
    Builder.SetInsertPoint(EntryBlock);

    auto Sum = Builder.CreateAlloca(llvm::Type::getIntNTy(TheContext, 64),
             llvm::ConstantInt::get(TheContext, llvm::APInt(64, 0, false)));
    Builder.CreateStore(llvm::ConstantInt::get(TheContext, llvm::APInt(64, 0, false)), Sum);
    // sum = 0

    // loop head
    auto HeadBlock = BasicBlock::Create(TheContext, "head", F);
    auto BodyBlock = BasicBlock::Create(TheContext, "body", F);
    auto ExitBlock = BasicBlock::Create(TheContext, "exit", F);
    Builder.CreateBr(HeadBlock);
    Builder.SetInsertPoint(HeadBlock);

    auto Input = Builder.CreateCall(TheModule->getFunction("input"));
    // i = input()
    auto Zero = llvm::ConstantInt::get(TheContext,
                                       llvm::APInt(64, 0, false));
    auto Cond = Builder.CreateICmpEQ(Input, Zero, "cmp");
    // i == 0 ?

    Builder.CreateCondBr(Cond, ExitBlock, BodyBlock);
    // if i == 0, goto exit, else goto loop body

    // loop body, basically a switch statement
    Builder.SetInsertPoint(BodyBlock);
    auto Switch = Builder.CreateSwitch(Input, HeadBlock, data.size());

    std::map<int, BasicBlock *> BlockMap;
    for (auto P : data) {
      auto Key = llvm::ConstantInt::get(TheContext,
                                         llvm::APInt(64 , P.first, false));
      auto Value = llvm::ConstantInt::get(TheContext,
                                       llvm::APInt(64, P.second, false));

      auto BB = BasicBlock::Create(TheContext, "b_" + std::to_string(P.first), F);
      BlockMap[P.first] = BB;
      Switch->addCase(Key, BB);
      Builder.SetInsertPoint(BB);

      auto x = Builder.CreateLoad(Sum);
      auto NewVal = Builder.CreateAdd(x, Value);
      Builder.CreateStore(NewVal, Sum);
      Builder.CreateBr(HeadBlock);
    }

    // exit
    Builder.SetInsertPoint(ExitBlock);
    auto Result = Builder.CreateLoad(Sum);
    std::vector<Value*> fooargs = {Result};
    Builder.CreateCall(TheModule->getFunction("printint"), fooargs);
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
