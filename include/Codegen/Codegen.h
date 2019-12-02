#ifndef CSSJIT_CODEGEN_H
#define CSSJIT_CODEGEN_H

#include <map>
#include <unordered_map>
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
#include "Common/Common.h"

using namespace llvm;
namespace cssjit {

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
    genTop();
    genBottom();
    genLeft();
    genDelta();

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
  void genTop() {
    auto CharStar = llvm::Type::getInt8PtrTy(TheContext);
    std::vector<llvm::Type*> args = {CharStar};
    auto FT = FunctionType::get(llvm::Type::getInt32Ty(TheContext), args, false);

    Function *F = Function::Create(FT, Function::ExternalLinkage, "getTop_c", TheModule.get());

    auto EntryBlock = BasicBlock::Create(TheContext, "entry", F);
    Builder.SetInsertPoint(EntryBlock);

    auto Result = Builder.CreateAlloca(llvm::Type::getInt32Ty(TheContext));
    Builder.CreateStore(Builder.getInt32(3), Result);

    auto ExitBlock = BasicBlock::Create(TheContext, "exit", F);
    Builder.SetInsertPoint(ExitBlock);
    auto V = Builder.CreateLoad(Result);
    Builder.CreateRet(V);


    std::map<std::string, BasicBlock *> RuleBlocks;
    for (auto &&Rule : Rules) {

      if (RuleBlocks.find(Rule.first) == RuleBlocks.end()) {
        auto BB = BasicBlock::Create(TheContext, Rule.first, F);
        RuleBlocks[Rule.first] = BB;
      }

      auto BB = RuleBlocks[Rule.first];
      Builder.SetInsertPoint(BB);

      for (auto &&KV : Rule.second) {
        int C = std::stoi(KV.second);
        if (KV.first == "tm" || KV.first == "tb" || KV.first == "tp") {
          auto V = Builder.CreateLoad(Result);
          auto VX = Builder.CreateAdd(V, Builder.getInt32(C - 1));
          Builder.CreateStore(VX, Result);
        }
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
  }

  void genBottom() {
    auto CharStar = llvm::Type::getInt8PtrTy(TheContext);
    std::vector<llvm::Type*> args = {CharStar};
    auto FT = FunctionType::get(llvm::Type::getInt32Ty(TheContext), args, false);

    Function *F = Function::Create(FT, Function::ExternalLinkage, "getBottom_c", TheModule.get());

    auto EntryBlock = BasicBlock::Create(TheContext, "entry", F);
    Builder.SetInsertPoint(EntryBlock);

    auto Result = Builder.CreateAlloca(llvm::Type::getInt32Ty(TheContext));
    Builder.CreateStore(Builder.getInt32(3), Result);

    auto ExitBlock = BasicBlock::Create(TheContext, "exit", F);
    Builder.SetInsertPoint(ExitBlock);
    auto V = Builder.CreateLoad(Result);
    Builder.CreateRet(V);


    std::map<std::string, BasicBlock *> RuleBlocks;
    for (auto &&Rule : Rules) {

      if (RuleBlocks.find(Rule.first) == RuleBlocks.end()) {
        auto BB = BasicBlock::Create(TheContext, Rule.first, F);
        RuleBlocks[Rule.first] = BB;
      }

      auto BB = RuleBlocks[Rule.first];
      Builder.SetInsertPoint(BB);

      for (auto &&KV : Rule.second) {
        int C = std::stoi(KV.second);
        if (KV.first == "bm" || KV.first == "bb" || KV.first == "bp") {
          auto V = Builder.CreateLoad(Result);
          auto VX = Builder.CreateAdd(V, Builder.getInt32(C - 1));
          Builder.CreateStore(VX, Result);
        }
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
  }

    void genLeft() {
    auto CharStar = llvm::Type::getInt8PtrTy(TheContext);
    std::vector<llvm::Type*> args = {CharStar};
    auto FT = FunctionType::get(llvm::Type::getInt32Ty(TheContext), args, false);

    Function *F = Function::Create(FT, Function::ExternalLinkage, "getLeft_c", TheModule.get());

    auto EntryBlock = BasicBlock::Create(TheContext, "entry", F);
    Builder.SetInsertPoint(EntryBlock);

    auto Result = Builder.CreateAlloca(llvm::Type::getInt32Ty(TheContext));
    Builder.CreateStore(Builder.getInt32(3), Result);

    auto ExitBlock = BasicBlock::Create(TheContext, "exit", F);
    Builder.SetInsertPoint(ExitBlock);
    auto V = Builder.CreateLoad(Result);
    Builder.CreateRet(V);


    std::map<std::string, BasicBlock *> RuleBlocks;
    for (auto &&Rule : Rules) {

      if (RuleBlocks.find(Rule.first) == RuleBlocks.end()) {
        auto BB = BasicBlock::Create(TheContext, Rule.first, F);
        RuleBlocks[Rule.first] = BB;
      }

      auto BB = RuleBlocks[Rule.first];
      Builder.SetInsertPoint(BB);

      for (auto &&KV : Rule.second) {
        int C = std::stoi(KV.second);
        if (KV.first == "lm" || KV.first == "lb" || KV.first == "lp") {
          auto V = Builder.CreateLoad(Result);
          auto VX = Builder.CreateAdd(V, Builder.getInt32(C - 1));
          Builder.CreateStore(VX, Result);
        }
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
  }

  void genDelta() {
    auto CharStar = llvm::Type::getInt8PtrTy(TheContext);
    std::vector<llvm::Type*> args = {CharStar};
    auto FT = FunctionType::get(llvm::Type::getInt32Ty(TheContext), args, false);

    Function *F = Function::Create(FT, Function::ExternalLinkage, "getDelta_c", TheModule.get());

    auto EntryBlock = BasicBlock::Create(TheContext, "entry", F);
    Builder.SetInsertPoint(EntryBlock);

    auto A = Builder.CreateAlloca(llvm::Type::getInt32Ty(TheContext));
    Builder.CreateStore(Builder.getInt32(1), A);

    auto B = Builder.CreateAlloca(llvm::Type::getInt32Ty(TheContext));
    Builder.CreateStore(Builder.getInt32(1), B);

    auto ExitBlock = BasicBlock::Create(TheContext, "exit", F);
    Builder.SetInsertPoint(ExitBlock);

    auto AV = Builder.CreateLoad(A);
    auto BV = Builder.CreateLoad(B);
    auto Cmp = Builder.CreateICmpULT(AV, BV);
    Builder.CreateRet(Builder.CreateSelect(Cmp, AV, BV));

    std::map<std::string, BasicBlock *> RuleBlocks;
    for (auto &&Rule1 : Rules) {
      for (auto &&Rule2 : Rules) {
        if (Rule1.first == Rule2.first) {
          continue;
        }
        auto str = Rule1.first+"."+Rule2.first;
        auto BB = BasicBlock::Create(TheContext, str, F);
        RuleBlocks[str] = BB;

        Builder.SetInsertPoint(BB);

        for (auto &&KV : Rule1.second) {
          int C = std::stoi(KV.second);
          if (KV.first == "bm") {
            Builder.CreateStore(Builder.getInt32(C), A);
          }
        }

        for (auto &&KV : Rule2.second) {
          int C = std::stoi(KV.second);
          if (KV.first == "tm") {
            Builder.CreateStore(Builder.getInt32(C), B);
          }
        }
        Builder.CreateBr(ExitBlock);
      }
    }

    Builder.SetInsertPoint(EntryBlock);

    llvm::Argument *Arg = F->arg_begin();

    std::vector<llvm::Value *> hash_args = {Arg};
    auto Hash = Builder.CreateCall(TheModule->getFunction("hash"), hash_args);

    auto Switch = Builder.CreateSwitch(Hash, ExitBlock, RuleBlocks.size());
    for (auto Pair : RuleBlocks) {
      auto CHash = Builder.getInt64(hash(const_cast<char *>(Pair.first.c_str())));
      Switch->addCase(CHash, Pair.second);
    }
  }
  LLVMContext TheContext;
  IRBuilder<> Builder;
  std::unique_ptr<Module> TheModule;
  std::map<std::string, Value *> NamedValues;
  std::unordered_map<std::string, llvm::Value *> StringTab;
};
}

#endif
