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
  using CSSMap = std::map<std::string, std::vector<std::pair<std::string, std::string>>>;

  int64_t hash(char *key) {
    std::string str(key);
    return std::hash<std::string>()(str);
  }

class Codegen {
public:
  Codegen(CSSMap Rules_)
  : Rules(Rules_), Builder(TheContext) {
    TheModule = llvm::make_unique<Module>("main", TheContext);

    std::vector<llvm::Type*> args = {};
//     auto FT = FunctionType::get(llvm::Type::getInt64Ty(TheContext), args, false);
//     Function::Create(FT, Function::ExternalLinkage, "input", TheModule.get());

//     args = {llvm::Type::getInt64Ty(TheContext)};
//     FT = FunctionType::get(llvm::Type::getVoidTy(TheContext), args, false);
//     Function::Create(FT, Function::ExternalLinkage, "printint", TheModule.get());

    auto CharStar = llvm::Type::getInt8PtrTy(TheContext);
    args = {CharStar, CharStar};
    auto FT = FunctionType::get(llvm::Type::getVoidTy(TheContext), args, false);
    Function::Create(FT, Function::ExternalLinkage, "printrule", TheModule.get());

    args = {CharStar};
    FT = FunctionType::get(llvm::Type::getInt64Ty(TheContext), args, false);
    Function::Create(FT, Function::ExternalLinkage, "hash", TheModule.get());

    FT = FunctionType::get(llvm::Type::getInt32Ty(TheContext), args, false);
    Function::Create(FT, Function::ExternalLinkage, "eq", TheModule.get());
  }

  llvm::Module *operator()(std::string filename) {
    auto CharStar = llvm::Type::getInt8PtrTy(TheContext);
    std::vector<llvm::Type*> args = {CharStar};
    auto FT = FunctionType::get(llvm::Type::getVoidTy(TheContext), args, false);

    Function *F = Function::Create(FT, Function::ExternalLinkage, "match", TheModule.get());

    std::map<std::string, BasicBlock *> RuleBlocks;

    auto EntryBlock = BasicBlock::Create(TheContext, "entry", F);

    auto ExitBlock = BasicBlock::Create(TheContext, "exit", F);
    Builder.SetInsertPoint(ExitBlock);
    Builder.CreateRetVoid();


    for (auto &&Rule : Rules) {
      auto BB = BasicBlock::Create(TheContext, Rule.first, F);
      RuleBlocks[Rule.first] = BB;
      Builder.SetInsertPoint(BB);

      for (auto &&KV : Rule.second) {
        auto K = getString(KV.first);
        auto V = getString(KV.second);
        std::vector<llvm::Value *> args = {K, V};
        Builder.CreateCall(TheModule->getFunction("printrule"), args);
      }
      Builder.CreateBr(ExitBlock);
    }

    Builder.SetInsertPoint(EntryBlock);

    llvm::Argument *A = F->arg_begin();
    std::vector<llvm::Value *> hash_args = {A};
    auto Hash = Builder.CreateCall(TheModule->getFunction("hash"), hash_args);

    auto Switch = Builder.CreateSwitch(Hash, ExitBlock, RuleBlocks.size());
    for (auto Pair : RuleBlocks) {
      auto CHash = Builder.getInt64(hash(const_cast<char *>(Pair.first.c_str())));
      Switch->addCase(CHash, Pair.second);
    }

    std::error_code EC;
    llvm::raw_fd_ostream out(filename, EC);
    TheModule->print(out, nullptr);

    return TheModule.get();
  }

  llvm::Value *getString(std::string Str) {
    if (StringTab.find(Str) != StringTab.end()) {
      return StringTab[Str];
    }
    StringTab[Str] = Builder.CreateGlobalStringPtr(Str);
    return StringTab[Str];
  }
private:
  CSSMap Rules;

  LLVMContext TheContext;
  IRBuilder<> Builder;
  std::unique_ptr<Module> TheModule;
  std::map<std::string, Value *> NamedValues;
  std::unordered_map<std::string, llvm::Value> StringTab;
};
}

#endif
