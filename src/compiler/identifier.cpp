
#include <iostream>
#include "identifier.h"
#include "antlr4-runtime.h"

using antlr4::tree::ParseTree;
using std::cout;
using std::string;

void FuncComp::add(Instruction inst)
{
  instructions.push_back(inst);
}

void FuncComp::clean()
{
  // Removing unnecessary temporary variables of the form
  // temp = a * b
  // a = temp
  if (instructions.size() > 1)
  {
    for (int i = instructions.size() - 2; i > -1; i--)
    {
      if (
        instructions[i + 1].instr == Instruction::Abstr::ASSIGN &&
        instructions[i].assignment == instructions[i + 1].operand1.id
      )
      {
        instructions[i].assignment = instructions[i + 1].assignment;
        instructions.erase(instructions.begin() + i + 1);
      }
    }
  }
}

string FuncComp::to_string()
{
  string s = "";
  for (auto& inst : instructions)
  {
    s += inst.to_string() + "\n";
  }
  return s;
}

Identifier::Identifier(const Identifier& other)
{
  type = other.type;
  name = other.name;
  // We can optimize this later
  dataType = other.dataType;
  returnType = other.returnType;
  initialized = other.initialized;
  members = other.members;
  initializers = other.initializers;
  function = other.function;
  funcTable = other.funcTable;
  latest = other.latest;
}

bool Identifier::equal(Identifier& other)
{
  switch(type)
  {
    case IdType::FUNCTION:
      {
        bool equal = other.type == IdType::FUNCTION;
        equal = equal && other.dataType == dataType;
        equal = equal && other.returnType == returnType;
        // TODO -- this needs expansion:
        // it needs to check if the parameter order and 
        // types are the same, not names etc.
        if (members.size() != other.members.size()) return false;
        for (int i = 0; i < members.size(); i++)
        {
          if (members[i].dataType != other.members[i].dataType)
            return false;
        }
        return equal;
      }
    case IdType::STRUCT:
      {
        bool equal = other.type == IdType::STRUCT;
        equal = equal && other.dataType == dataType;
        equal = equal && EqualVectors(other.members, members);
        return equal;
      }
    case IdType::UNION:
      {
        bool equal = other.type == IdType::UNION;
        equal = equal && other.dataType == dataType;
        equal = equal && EqualVectors(other.members, members);
        return equal;
      }
    case IdType::VARIABLE:
      {
        bool equal = other.type == IdType::VARIABLE;
        return equal && other.dataType == dataType && other.name == name;
      }
    case IdType::ARRAY:
      {
        bool equal = other.type == IdType::FUNCTION;
        equal = equal && other.dataType == dataType;
        equal = equal && other.returnType == returnType;
        equal = equal && EqualVectors(other.members, members);
        return equal;
      }
    default:
      return false;
  }
}

bool Identifier::equal(Identifier* other)
{
  switch(type)
  {
    case IdType::FUNCTION:
      {
        bool equal = other->type == IdType::FUNCTION;
        equal = equal && other->dataType == dataType;
        equal = equal && other->returnType == returnType;
        // TODO -- this needs expansion:
        // it needs to check if the parameter order and 
        // types are the same, not names etc.
        if (members.size() != other->members.size()) return false;
        for (int i = 0; i < members.size(); i++)
        {
          if (members[i].dataType != other->members[i].dataType)
            return false;
        }
        return equal;
      }
    case IdType::STRUCT:
      {
        bool equal = other->type == IdType::STRUCT;
        equal = equal && other->dataType == dataType;
        equal = equal && EqualVectors(other->members, members);
        return equal;
      }
    case IdType::UNION:
      {
        bool equal = other->type == IdType::UNION;
        equal = equal && other->dataType == dataType;
        equal = equal && EqualVectors(other->members, members);
        return equal;
      }
    case IdType::VARIABLE:
      {
        bool equal = other->type == IdType::VARIABLE;
        return equal && other->dataType == dataType && other->name == name;
      }
    case IdType::ARRAY:
      {
        bool equal = other->type == IdType::FUNCTION;
        equal = equal && other->dataType == dataType;
        equal = equal && other->returnType == returnType;
        equal = equal && EqualVectors(other->members, members);
        return equal;
      }
    default:
      return false;
  }
}

Result::Result(const Result& other)
{
  if (other.kind == Kind::ID)
    id = other.id;
  else
  {
    for (int i = 0; i < buff_size; i++)
      value[i] = other.value[i];
    type = other.type;
  }
  kind = other.kind;
}

#define T_EQUAL(T) \
T temp = as<T>(); \
T oth = other.as<T>(); \
return temp == oth;

bool Result::equal(Result& other, bool permissive)
{
  
  if (kind != other.kind)
    return false;
  
  if (kind == Kind::ID)
    return other.id == id; // see if they point to the same place

  // If they're not the same type (and we're not permissive...)
  if (type != other.type && !permissive)
    return false;
  
  // Otherwise...
  if (type == long_double_) {
    T_EQUAL(long double)
  } else if (type == double_) {
    T_EQUAL(double)
  } else if (type == float_) {
    T_EQUAL(float)
  } else if (type == unsigned_long_long_) {
    T_EQUAL(unsigned long long)
  } else if (type == long_long_) {
    T_EQUAL(long long)
  } else if (type == unsigned_long_) {
    T_EQUAL(unsigned long)
  } else if (type == long_) {
    T_EQUAL(long)
  } else if (type == unsigned_) {
    T_EQUAL(unsigned)
  } else if (type == int_) {
    T_EQUAL(int)
  } else if (type == unsigned_short_) {
    T_EQUAL(unsigned short)
  } else if (type == short_) {
    T_EQUAL(short)
  } else if (type == unsigned_char_) {
    T_EQUAL(unsigned char)
  } else if (type == signed_char_) {
    T_EQUAL(signed char)
  } else if (type == char_) {
    T_EQUAL(char)
  } else if (type == void_) {
    throw 2;
  }
  throw 3;
}

void Result::to(Type t)
{
  if (kind == Kind::ID)
    throw 1;
  if (t == long_double_) {
    auto temp = as<long double>();
    setValue(temp);
  } else if (t == double_) {
    auto temp = as<double>();
    setValue(temp);
  } else if (t == float_) {
    auto temp = as<float>();
    setValue(temp);
  } else if (t == unsigned_long_long_) {
    auto temp = as<unsigned long long>();
    setValue(temp);
  } else if (t == long_long_) {
    auto temp = as<long long>();
    setValue(temp);
  } else if (t == unsigned_long_) {
    auto temp = as<unsigned long>();
    setValue(temp);
  } else if (t == long_) {
    auto temp = as<long>();
    setValue(temp);
  } else if (t == unsigned_) {
    auto temp = as<unsigned>();
    setValue(temp);
  } else if (t == int_) {
    auto temp = as<int>();
    setValue(temp);
  } else if (t == unsigned_short_) {
    auto temp = as<unsigned short>();
    setValue(temp);
  } else if (t == short_) {
    auto temp = as<short>();
    setValue(temp);
  } else if (t == unsigned_char_) {
    auto temp = as<unsigned char>();
    setValue(temp);
  } else if (t == signed_char_) {
    auto temp = as<signed char>();
    setValue(temp);
  } else if (t == char_) {
    auto temp = as<char>();
    setValue(temp);
  } else if (t == void_) {
    throw 2;
  }
}

string Result::to_string()
{
  string s = "";
  if (kind == Kind::ID)
    s = id->name;
  else if (type == long_double_) {
    s = std::to_string(as<long double>());
  } else if (type == double_) {
    s = std::to_string(as<double>());
  } else if (type == float_) {
    s = std::to_string(as<float>());
  } else if (type == unsigned_long_long_) {
    s = std::to_string(as<unsigned long long>());
  } else if (type == long_long_) {
    s = std::to_string(as<long long>());
  } else if (type == unsigned_long_) {
    s = std::to_string(as<unsigned long>());
  } else if (type == long_) {
    s = std::to_string(as<long>());
  } else if (type == unsigned_) {
    s = std::to_string(as<unsigned>());
  } else if (type == int_) {
    s = std::to_string(as<int>());
  } else if (type == unsigned_short_) {
    s = std::to_string(as<unsigned short>());
  } else if (type == short_) {
    s = std::to_string(as<short>());
  } else if (type == unsigned_char_) {
    s = std::to_string(as<unsigned char>());
  } else if (type == signed_char_) {
    s = std::to_string(as<signed char>());
  } else if (type == char_) {
    s = std::to_string(as<char>());
  } else if (type == void_) {
    s = "void";
  } else {
    s = "<error-val>";
  }
  return s;
}

void Result::setValue(long double val)
{
  kind = Kind::STANDARD;
  type.type_specifiers = {"double", "long"};
  long double* ptr = (long double*) value;
  *ptr = val;
}
void Result::setValue(double val)
{
  kind = Kind::STANDARD;
  type.type_specifiers = {"double"};
  double* ptr = (double*) value;
  *ptr = val;
}
void Result::setValue(float val)
{
  kind = Kind::STANDARD;
  type.type_specifiers = {"float"};
  float* ptr = (float*) value;
  *ptr = val;
}
void Result::setValue(unsigned long long val)
{
  kind = Kind::STANDARD;
  type.type_specifiers = {"long", "long", "unsigned"};
  long long unsigned* ptr = (long long unsigned*) value;
  *ptr = val;
}
void Result::setValue(long long val)
{
  kind = Kind::STANDARD;
  type.type_specifiers = {"long", "long"};
  long long* ptr = (long long*) value;
  *ptr = val;
}
void Result::setValue(unsigned long val)
{
  kind = Kind::STANDARD;
  type.type_specifiers = {"long", "unsigned"};
  unsigned long* ptr = (unsigned long*) value;
  *ptr = val;
}
void Result::setValue(long val)
{
  kind = Kind::STANDARD;
  type.type_specifiers = {"long"};
  long* ptr = (long*) value;
  *ptr = val;
}
void Result::setValue(unsigned val)
{
  kind = Kind::STANDARD;
  type.type_specifiers = {"unsigned"};
  unsigned* ptr = (unsigned*) value;
  *ptr = val;
}
void Result::setValue(int val)
{
  kind = Kind::STANDARD;
  type.type_specifiers = {"int"};
  int* ptr = (int*) value;
  *ptr = val;
}
void Result::setValue(unsigned short val)
{
  kind = Kind::STANDARD;
  type.type_specifiers = {"short", "unsigned"};
  unsigned short* ptr = (unsigned short*) value;
  *ptr = val;
}
void Result::setValue(short val)
{
  kind = Kind::STANDARD;
  type.type_specifiers = {"short"};
  short* ptr = (short*) value;
  *ptr = val;
}
void Result::setValue(unsigned char val)
{
  kind = Kind::STANDARD;
  type.type_specifiers = {"char", "unsigned"};
  unsigned char* ptr = (unsigned char*) value;
  *ptr = val;
}
void Result::setValue(signed char val)
{
  kind = Kind::STANDARD;
  type.type_specifiers = {"char", "signed"};
  signed char* ptr = (signed char*) value;
  *ptr = val;
}
void Result::setValue(char val)
{
  kind = Kind::STANDARD;
  type.type_specifiers = {"char"};
  char* ptr = (char*) value;
  *ptr = val;
}

Instruction::Instruction(ParserRuleContext* c, Abstr i, Result op1, Identifier& ass) {
  ctx = c;
  instr = i;
  operand1 = op1;
  single = true;
  assignment = &ass;
}

Instruction::Instruction(ParserRuleContext* c, Abstr i, Result op1, Result op2, Identifier& ass) {
  ctx = c;
  instr = i;
  operand1 = op1;
  operand2 = op2;
  single = false;
  assignment = &ass;
}

Instruction::Instruction(ParserRuleContext* c, Abstr i, Cond co, Result op1, Result op2, Identifier& ass) {
  ctx = c;
  instr = i;
  condition = co;
  operand1 = op1;
  operand2 = op2;
  single = false;
  assignment = &ass;
}

Instruction::Instruction(const Instruction& other) {
  ctx = other.ctx;
  instr = other.instr;
  operand1 = other.operand1;
  single = other.single;
  condition = other.condition;
  if (!single)
    operand2 = other.operand2;
  assignment = other.assignment;
}

string Instruction::name() {
  switch (instr) {
    case DEREF:
      return "dereference";
    case NOT:
      return "bitwise not";
    case NEGATE:
      return "logical negation";
    case NEGATIVE:
      return "arithmetic negation";
    case CONVERT:
      return "type conversion";
    case ASSIGN:
      return "assignment";
    case ADD:
      return "addition";
    case SUB:
      return "subtraction";
    case MULT:
      return "multiplication";
    case DIV:
      return "division";
    case MOD:
      return "modulo";
    case SHIFT_L:
      return "logical shift left";
    case SHIFT_R:
      return "logical shift right";
    case BIT_AND:
      return "bitwise and";
    case BIT_XOR:
      return "bitwise exclusive or";
    case BIT_OR:
      return "bitwise or";
    case AND:
      return "logical and";
    case OR:
      return "logical or";
    case CMP:
    {
      switch (condition)
      {
        default:
        case EQUAL:
          return "equality comparison";
        case NOT_EQUAL:
          return "inequality comparison";
        case GREATER:
          return "greater than comparison";
        case LESS:
          return "less than comparison";
        case GREATER_EQUAL:
          return "greater than or equal to comparison";
        case LESS_EQUAL:
          return "less than or equal to comparison";
      }
    }
    default:
      return "<unkown>";
  }
}

string Instruction::to_string() {
  string s = assignment->name + " = ";
  switch (instr) {
    case DEREF:
    {
      if (single)
        s +=  "*" + operand1.to_string();
      else
        s +=  "*(" + operand1.to_string() + " + " + operand2.to_string() + ")";
    }
    break;
    case NOT:
    {
      s +=  "~" + operand1.to_string();
    }
    break;
    case NEGATE:
    {
      s +=  "!" + operand1.to_string();
    }
    break;
    case NEGATIVE:
    {
      s +=  "-" + operand1.to_string();
    }
    break;
    case CONVERT:
    {
      if (operand1.isConst())
      {
        s +=  operand1.to_string() + " to " + assignment->dataType.to_string();
      }
      else
      {
        s +=  operand1.to_string() + " (" + operand1.id->dataType.to_string();
        s +=  ") to " + assignment->dataType.to_string();
      }
    }
    break;
    case ASSIGN:
    {
      s +=  operand1.to_string();
    }
    break;
    case ADD:
    {
      s +=  operand1.to_string() + " + " + operand2.to_string();
    }
    break;
    case SUB:
    {
      s +=  operand1.to_string() + " - " + operand2.to_string();
    }
    break;
    case MULT:
    {
      s +=  operand1.to_string() + " * " + operand2.to_string();
    }
    break;
    case DIV:
    {
      s +=  operand1.to_string() + " / " + operand2.to_string();
    }
    break;
    case MOD:
    {
      s +=  operand1.to_string() + " % " + operand2.to_string();
    }
    break;
    case SHIFT_L:
    {
      s +=  operand1.to_string() + " + " + operand2.to_string();
    }
    break;
    case SHIFT_R:
    {
      s +=  operand1.to_string() + " >> " + operand2.to_string();
    }
    break;
    case BIT_AND:
    {
      s +=  operand1.to_string() + " & " + operand2.to_string();
    }
    break;
    case BIT_XOR:
    {
      s +=  operand1.to_string() + " ^ " + operand2.to_string();
    }
    break;
    case BIT_OR:
    {
      s +=  operand1.to_string() + " | " + operand2.to_string();
    }
    break;
    case AND:
    {
      s +=  operand1.to_string() + " && " + operand2.to_string();
    }
    break;
    case OR:
    {
      s +=  operand1.to_string() + " || " + operand2.to_string();
    }
    break;
    case CMP:
    {
      switch (condition)
      {
        case EQUAL:
          s +=  operand1.to_string() + " == " + operand2.to_string();
          break;
        case NOT_EQUAL:
          s +=  operand1.to_string() + " != " + operand2.to_string();
          break;
        case GREATER:
          s +=  operand1.to_string() + " > " + operand2.to_string();
          break;
        case LESS:
          s +=  operand1.to_string() + " < " + operand2.to_string();
          break;
        case GREATER_EQUAL:
          s +=  operand1.to_string() + " >= " + operand2.to_string();
          break;
        case LESS_EQUAL:
          s +=  operand1.to_string() + " <= " + operand2.to_string();
          break;
        default:
          s +=  "error comparison";
          break;
      }
    }
    break;
  }
  return s;
}