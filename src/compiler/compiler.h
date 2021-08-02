#pragma once

#ifndef COMPILER_H
#define COMPILER_H

#include <iostream>
#include <vector>
#include <memory>
#include "antlr4-runtime.h"
#include "PostLexer.h"
#include "PostParser.h"
#include "PostBaseVisitor.h"
#include "PostBaseListener.h"

#include "identifier.h"
#include "stable.h"

#include "utils.h"
#include "error.h"

using namespace antlr4;
using antlrcpp::Any;
using std::unique_ptr;

// forward declaration
class Compiler;

// this needs to be moved to its own file
class OperatorBase {
  public:

    OperatorBase(ParserRuleContext* c, SymbolTable* t, Identifier* f, Compiler* co) {
      ctx = c;
      table = t;
      func = f;
      comp = co;
    }

    virtual Instruction::Abstr getAbstr() { throw 1; } // this should always be overriden
    virtual Instruction::Cond getCond() { return Instruction::Cond::EQUAL; } // this is only important for conditional expressions
    virtual bool validType1(Type& t) { return true; }
    virtual bool validType2(Type& t) { return true; }

    // TODO this can be much improved -- currently it eats up any temp vars it passes while looking for matching ones
    virtual Identifier& manageTemps(Type& t) {
      Identifier ass;
      if (table->total_temps == 0) 
      {
        string tempname = "__temp_var_" + std::to_string(table->temp_vars) + "__";
        table->total_temps++;
        table->temp_vars++;
        ass.name = tempname;
        ass.dataType = t;
        table->AddSymbol(ass);
        return table->GetLast();
      }
      else
      {
        for (int i = table->temp_vars; i < table->total_temps; i++) {
          string tempname = "__temp_var_" + std::to_string(i) + "__";

          try {
            Identifier& ti = table->GetLocalSymbol(tempname);
            if (ti.dataType == t) {
              table->temp_vars = i + 1;
              return ti;
            }
          } catch (int e) {
            // pass
          }
        }

        string tempname = "__temp_var_" + std::to_string(table->total_temps) + "__";
        table->total_temps++;
        table->temp_vars = table->total_temps;
        ass.name = tempname;
        ass.dataType = t;
        table->AddSymbol(ass);
        return table->GetLast();
      }
    }

    /////////////////////////////////////////
    // Binary expressions
    /////////////////////////////////////////

    virtual void perform(long double v1, long double v2, Result& res) { throw long_double_; }
    virtual void perform(double v1, double v2, Result& res) { throw double_; }
    virtual void perform(float v1, float v2, Result& res) { throw float_; }
    virtual void perform(unsigned long long v1, unsigned long long v2, Result& res) { throw unsigned_long_long_; }
    virtual void perform(long long v1, long long v2, Result& res) { throw long_long_; }
    virtual void perform(unsigned long v1, unsigned long v2, Result& res) { throw unsigned_long_; }
    virtual void perform(long v1, long v2, Result& res) { throw long_; }
    virtual void perform(unsigned v1, unsigned v2, Result& res) { throw unsigned_; }
    virtual void perform(int v1, int v2, Result& res) { throw int_; }
    virtual void perform(unsigned short v1, unsigned short v2, Result& res) { throw short_; }
    virtual void perform(unsigned char v1, unsigned char v2, Result& res) { throw unsigned_char_; }
    virtual void perform(signed char v1, signed char v2, Result& res) { throw signed_char_; }
    virtual void perform(char v1, char v2, Result& res) { throw char_; }

    // Currently, in an operation between an lvalue and constant, the
    // constant is always converted to the lvalue's type (which may
    // not always be desirable?)
    virtual void perform(Identifier& id, Result& op2, Result& res) {

      if (!validType1(id.dataType)) throw id.dataType;
      if (!validType2(op2.type)) throw op2.type;

      int priority1 = fetchPriority(id.dataType);
      int priority2 = fetchPriority(op2.type);

      Result lhs;
      lhs.setValue(id);

      // string tempname = "__temp_var_" + std::to_string(table->temp_vars++) + "__";
      // Identifier ass;
      // ass.name = tempname;

      Identifier& ass = manageTemps(lhs.id->dataType);

      if (priority1 == priority2) {
        func->function.add(Instruction(ctx, getAbstr(), getCond(), lhs, op2, ass));
      }
      else {
        op2.to(id.dataType);
        func->function.add(Instruction(ctx, getAbstr(), getCond(), lhs, op2, ass));
      }
      // else if (priority1 < priority2) {
      //   op2.to(id.dataType);
      //   ass.dataType = lhs.id.dataType;
      //   table->AddSymbol(ass);
      //   func->function.add(Instruction(ctx, getAbstr(), lhs, op2, ass));
      // }
      // else
      // {
      //   ass.dataType = op2.type;
      //   Result temp;
      //   temp.setValue(ass);

      //   func->function.add(Instruction(ctx, Instruction::Abstr::CONVERT, lhs, op2, ass));
      //   func->function.add(Instruction(ctx, getAbstr(), temp, op2, ass));
      // }
      res.setValue(ass);
    }
    virtual void perform(Result& op1, Identifier& id, Result& res) {

      if (!validType1(op1.type)) throw op1.type;
      if (!validType2(id.dataType)) throw id.dataType;

      int priority2 = fetchPriority(id.dataType);
      int priority1 = fetchPriority(op1.type);

      Result rhs;
      rhs.setValue(id);

      // string tempname = "__temp_var_" + std::to_string(table->temp_vars++) + "__";
      // Identifier ass;
      // ass.name = tempname;
      Identifier* ass;

      if (priority1 == priority2) {
        ass = &manageTemps(rhs.id->dataType);
        func->function.add(Instruction(ctx, getAbstr(), getCond(), op1, rhs, *ass));        
      }
      else {
        op1.to(id.dataType);
        ass = &manageTemps(rhs.id->dataType);
        func->function.add(Instruction(ctx, getAbstr(), getCond(), op1, rhs, *ass));
      }
      // else if (priority1 < priority2) {

      //   ass.dataType = op1.type;
      //   Result temp;
      //   temp.setValue(ass);

      //   func->function.add(Instruction(ctx, Instruction::Abstr::CONVERT, rhs, op1, ass));
      //   func->function.add(Instruction(ctx, getAbstr(), op1, temp, ass));
      // }
      // else
      // {
      //   op1.to(id.dataType);
      //   ass.dataType = rhs.id.dataType;
      //   table->AddSymbol(ass);
      //   func->function.add(Instruction(ctx, getAbstr(), op1, rhs, ass));
      // }
      res.setValue(*ass);
    }
    virtual void perform(Identifier& id1, Identifier& id2, Result& res) {

      if (!validType1(id1.dataType)) throw id1.dataType;
      if (!validType2(id2.dataType)) throw id2.dataType;
      
      int priority1 = fetchPriority(id1.dataType);
      int priority2 = fetchPriority(id2.dataType);

      Result lhs;
      lhs.setValue(id1);
      Result rhs;
      rhs.setValue(id2);

      // string tempname = "__temp_var_" + std::to_string(table->temp_vars++) + "__";
      // Identifier ass;
      // ass.name = tempname;
      Identifier* ass;

      if (priority1 == priority2)
      {
        // ass.dataType = id1.dataType;
        ass = &manageTemps(id1.dataType);
        func->function.add(Instruction(ctx, getAbstr(), getCond(), lhs, rhs, *ass));
      }
      else if (priority1 < priority2)
      {
        // ass.dataType = id1.dataType;
        ass = &manageTemps(id1.dataType);
        Result temp;
        temp.setValue(*ass);

        func->function.add(Instruction(ctx, Instruction::Abstr::CONVERT, rhs, lhs, *ass));
        func->function.add(Instruction(ctx, getAbstr(), getCond(), lhs, temp, *ass));
      }
      else if (priority1 > priority2)
      {
        // ass.dataType = id2.dataType;
        ass = &manageTemps(id2.dataType);
        Result temp;
        temp.setValue(*ass);

        func->function.add(Instruction(ctx, Instruction::Abstr::CONVERT, lhs, rhs, *ass));
        func->function.add(Instruction(ctx, getAbstr(), getCond(), temp, rhs, *ass));
      }
      res.setValue(*ass);
    }

    /////////////////////////////////////////
    // Unary expressions
    /////////////////////////////////////////

    virtual void perform(long double v1, Result& res) { throw long_double_; }
    virtual void perform(double v1, Result& res) { throw double_; }
    virtual void perform(float v1, Result& res) { throw float_; }
    virtual void perform(unsigned long long v1, Result& res) { throw unsigned_long_long_; }
    virtual void perform(long long v1, Result& res) { throw long_long_; }
    virtual void perform(unsigned long v1, Result& res) { throw unsigned_long_; }
    virtual void perform(long v1, Result& res) { throw long_; }
    virtual void perform(unsigned v1, Result& res) { throw unsigned_; }
    virtual void perform(int v1, Result& res) { throw int_; }
    virtual void perform(unsigned short v1, Result& res) { throw unsigned_short_; }
    virtual void perform(unsigned char v1,  Result& res) { throw unsigned_char_; }
    virtual void perform(signed char v1, Result& res) { throw signed_char_; }
    virtual void perform(char v1, Result& res) { throw char_; }

    virtual void perform(Identifier& id, Result& res) {

      if (!validType1(id.dataType)) throw id.dataType;

      Result lhs;
      lhs.setValue(id);

      // string tempname = "__temp_var_" + std::to_string(table->temp_vars++) + "__";
      // Identifier ass;
      // ass.name = tempname;

      // ass.dataType = lhs.id.dataType;
      Identifier& ass = manageTemps(lhs.id->dataType);
      func->function.add(Instruction(ctx, getAbstr(), lhs, ass));

      res.setValue(ass);
    }

    // needs access to the symbols table!
    SymbolTable* table;
    // will add the instruction to the current function
    Identifier* func;
    // Needs the parse tree node
    ParserRuleContext* ctx;
    // Also needs pointer to the compiler object for easy error reporting
    Compiler* comp;
};

class Compiler : PostBaseVisitor {

  public:
    Compiler(ProcessedCode* code_, Error* err_, bool g = false);
    ~Compiler() {}

    // void Process(ProcessedCode* code_, Error* err_);
    void Complete();
    void EnableGraph(bool enable) { graphing = enable; }

    void addRuleErr(ParserRuleContext* rule, string errmess);
    void addRuleWarn(ParserRuleContext* rule, string warnmess);

  // private:

    tree::ParseTreeProperty<string> storage;
    tree::ParseTreeProperty<Result> results;

    ANTLRInputStream stream;
    PostLexer lexer;
    CommonTokenStream tokens;
    PostParser parser;

    ProcessedCode* code;
    Error* err;
    unique_ptr<SymbolTable> globalTable;
    SymbolTable* currentScope;
    Identifier* currentFunction;
    // TODO -- This should probably be refactored later to not suck
    std::vector<Identifier*> currentId;
    std::vector<Type*> currentType;

    Graph graph;
    bool graphing = false;
    bool inherit = false;
    bool func_decl_err;
    int unnamed_inc = 0;

    void pushScope(SymbolTable::Scope scope = SymbolTable::Scope::LOCAL);
    void popScope();

    void operation(antlr4::tree::ParseTree* ctx, Result op1, Result op2, OperatorBase& oper);
    // unary
    void operation(antlr4::tree::ParseTree* ctx, Result op1, OperatorBase& oper);

    Any visitParse(PostParser::ParseContext* ctx) override;
    // Any visitTopDecl(PostParser::TopDeclContext* ctx) override;
    // Any visitTopFunc(PostParser::TopFuncContext* ctx) override;
    Any visitBlockDecl(PostParser::BlockDeclContext* ctx) override;
    Any visitBlockStat(PostParser::BlockStatContext* ctx) override;
    Any visitStat_compound(PostParser::Stat_compoundContext* ctx) override;
    Any visitParamList(PostParser::ParamListContext* ctx) override;
    Any visitTypeStd(PostParser::TypeStdContext* ctx) override;
    Any visitTypeStructUnion(PostParser::TypeStructUnionContext* ctx) override;
    Any visitTypeEnum(PostParser::TypeEnumContext* ctx) override;
    Any visitTypeTypedef(PostParser::TypeTypedefContext* ctx) override;
    Any visitFuncSpecifier(PostParser::FuncSpecifierContext* ctx) override;
    Any visitStorageSpecifier(PostParser::StorageSpecifierContext* ctx) override;
    Any visitDirId(PostParser::DirIdContext* ctx) override;
    Any visitPointer_item(PostParser::Pointer_itemContext* ctx) override;

    // Declarations
    Any visitDeclaration(PostParser::DeclarationContext* ctx) override;
    Any visitDeclarator(PostParser::DeclaratorContext* ctx) override;
    Any visitDirFunc(PostParser::DirFuncContext* ctx) override;
    Any visitFunc_def(PostParser::Func_defContext* ctx) override;
    Any visitParamDecl(PostParser::ParamDeclContext* ctx) override;
    Any visitParamAbst(PostParser::ParamAbstContext* ctx) override;

    // Expressions
    Any visitDereference(PostParser::DereferenceContext *ctx) override;
    Any visitIdentifier(PostParser::IdentifierContext *ctx) override;
    Any visitNegation(PostParser::NegationContext *ctx) override;
    Any visitIncrementUnary(PostParser::IncrementUnaryContext *ctx) override;
    Any visitAddress(PostParser::AddressContext *ctx) override;
    Any visitConstInt(PostParser::ConstIntContext *ctx) override;
    Any visitConstFlt(PostParser::ConstFltContext *ctx) override;
    Any visitString(PostParser::StringContext *ctx) override;
    Any visitIndexing(PostParser::IndexingContext *ctx) override;
    Any visitIncrementPost(PostParser::IncrementPostContext *ctx) override;
    Any visitSizeof(PostParser::SizeofContext *ctx) override;
    Any visitPositive(PostParser::PositiveContext *ctx) override;
    Any visitDecrementPost(PostParser::DecrementPostContext *ctx) override;
    Any visitDecrementUnary(PostParser::DecrementUnaryContext *ctx) override;
    Any visitCall(PostParser::CallContext *ctx) override;
    Any visitSizeofType(PostParser::SizeofTypeContext *ctx) override;
    Any visitIndirectMember(PostParser::IndirectMemberContext *ctx) override;
    Any visitNegative(PostParser::NegativeContext *ctx) override;
    Any visitNot(PostParser::NotContext *ctx) override;
    Any visitMember(PostParser::MemberContext *ctx) override;
    Any visitGroup(PostParser::GroupContext *ctx) override;
    Any visitExprCast(PostParser::ExprCastContext *ctx) override;
    Any visitCast(PostParser::CastContext *ctx) override;
    Any visitMinus(PostParser::MinusContext *ctx) override;
    Any visitMult(PostParser::MultContext *ctx) override;
    Any visitMod(PostParser::ModContext *ctx) override;
    Any visitOr(PostParser::OrContext *ctx) override;
    Any visitExprMulti(PostParser::ExprMultiContext *ctx) override;
    Any visitNotEqual(PostParser::NotEqualContext *ctx) override;
    Any visitLess(PostParser::LessContext *ctx) override;
    Any visitBit_or(PostParser::Bit_orContext *ctx) override;
    Any visitPlus(PostParser::PlusContext *ctx) override;
    Any visitGreater_equal(PostParser::Greater_equalContext *ctx) override;
    Any visitDiv(PostParser::DivContext *ctx) override;
    Any visitEqual(PostParser::EqualContext *ctx) override;
    Any visitShiftLeft(PostParser::ShiftLeftContext *ctx) override;
    Any visitShiftRight(PostParser::ShiftRightContext *ctx) override;
    Any visitBit_xor(PostParser::Bit_xorContext *ctx) override;
    Any visitAnd(PostParser::AndContext *ctx) override;
    Any visitBit_and(PostParser::Bit_andContext *ctx) override;
    Any visitGreater(PostParser::GreaterContext *ctx) override;
    Any visitLess_equal(PostParser::Less_equalContext *ctx) override;
    Any visitExprTern(PostParser::ExprTernContext *ctx) override;
    Any visitTernary(PostParser::TernaryContext *ctx) override;
    Any visitExprAssi(PostParser::ExprAssiContext *ctx) override;
    Any visitAssignment(PostParser::AssignmentContext *ctx) override;
    Any visitAssignmentMult(PostParser::AssignmentMultContext *ctx) override;
    Any visitAssignmentDiv(PostParser::AssignmentDivContext *ctx) override;
    Any visitAssignmentMod(PostParser::AssignmentModContext *ctx) override;
    Any visitAssignmentPlus(PostParser::AssignmentPlusContext *ctx) override;
    Any visitAssignmentMinus(PostParser::AssignmentMinusContext *ctx) override;
    Any visitAssignmentShiftLeft(PostParser::AssignmentShiftLeftContext *ctx) override;
    Any visitAssignmentShiftRight(PostParser::AssignmentShiftRightContext *ctx) override;
    Any visitAssignmentBitAnd(PostParser::AssignmentBitAndContext *ctx) override;
    Any visitAssignmentBitXor(PostParser::AssignmentBitXorContext *ctx) override;
    Any visitAssignmentBitOr(PostParser::AssignmentBitOrContext *ctx) override;
    Any visitArglist(PostParser::ArglistContext *ctx) override;
    Any visitExprList(PostParser::ExprListContext *ctx) override;
    Any visitComma(PostParser::CommaContext *ctx) override;
    Any visitExprExpression(PostParser::ExprExpressionContext *ctx) override;
    Any visitExpr_const(PostParser::Expr_constContext *ctx) override;

    Any visitInitAssign(PostParser::InitAssignContext* ctx) override;
    // Any visitInitList(PostParser::InitListContext* ctx) override;
    Any visitInitDeclAssigned(PostParser::InitDeclAssignedContext* ctx) override;

};

// class CompilerListener : PostBaseListener {

//   public:
//     CompilerListener() {}
//     ~CompilerListener() {}

//     void Process(ANTLRInputStream* stream);

//     void enterParse(PostParser::ParseContext* ctx) override;
//     void enterVariable_init(PostParser::Variable_initContext* ctx) override;

//   private:

//     std::vector<Variable> globals;
//     PostParser* parse;

// };

#endif