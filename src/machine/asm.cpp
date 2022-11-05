#include "asm.hpp"
#include "asm_build.hpp"

char phyRegNum2name[][10] = {
  "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1",
  "t2", "t3", "t4", "t5", "t6", "t7", "s0", "s1", "s2", "s3", "s4",
  "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
};

char instIdtfr2name[][10] = {
  "none ", "add ", "addu ", "addiu ", "sub ", "subu ", "mult ", "div ", "xor ", 
  "and ", "or ", "mfhi ", "mflo ", "lw ", "sw ", "nop ", "j ", "bne ", "jr ", 
  "sltiu ", "sltu ", "slt ", "la ", "jal ", "syscall ",
};

std::string AsmReg::toString()
{
  if (this->isPhysReg == false) {
    panic("error");
  }
  std::string s;
  s += "$";
  s += phyRegNum2name[this->physNumber];
  return s;
}


std::string AsmImm::toString()
{
  std::string s;
  s += std::to_string(this->immediate);
  return s;
}

std::string AsmLabel::toString()
{
  return this->label;
}

std::string AsmInst::toString()
{
  std::string s; 
  s += "\t";
  s += instIdtfr2name[this->asmInstIdtfr];
  s += " ";
  switch (this->asmInstIdtfr)
  {
  case AsmInstIdtfr::NOP_AII:
  case AsmInstIdtfr::SYSCALL_AII:
    break;
  case AsmInstIdtfr::MFHI_AII:
  case AsmInstIdtfr::MFLO_AII:
  case AsmInstIdtfr::J_AII:
  case AsmInstIdtfr::JR_AII:
  case AsmInstIdtfr::JAL_AII:
    if (this->ops.size() != 1) {
      panic("error");
    }
    s += this->ops[0].first->toString();
    break;
  case AsmInstIdtfr::MULT_AII:
  case AsmInstIdtfr::DIV_AII:
  case AsmInstIdtfr::LA_AII:
    if (this->ops.size() != 2) {
      panic("error");
    }
    s += this->ops[0].first->toString();
    s += ", ";
    s += this->ops[1].first->toString();
    break;
  case AsmInstIdtfr::LW_AII:
  case AsmInstIdtfr::SW_AII:
    s += this->ops[0].first->toString();
    s += ", ";
    s += this->ops[1].first->toString();
    if (this->ops.size() < 3) {
      break;
    }
    s += "(";
    s += this->ops[2].first->toString();
    s += ")";
    break;
  default:
    if (this->ops.size() != 3) {
      panic("error");
    }
    s += this->ops[0].first->toString();
    s += ", ";
    s += this->ops[1].first->toString();
    s += ", ";
    s += this->ops[2].first->toString();
    break;
  }
  return s;
}

std::string AsmStrData::toString()
{ 
  std::string s;
  s += this->label->toString();
  s += ": .asciiz ";  
  s += this->str;
  return s;
}

std::string AsmWordData::toString()
{
  std::string s;
  s += this->label->toString();
  s += ": ";
  if (!this->hasInitValue) {
    s += ".space ";
    s += std::to_string(this->wordNum * 4);
    return s;
  }
  s += ".word ";
  for (u_long i = 0; i < this->wordsInitValue.size(); i++) {
    s += std::to_string(this->wordsInitValue[i]);
    if (i < this->wordsInitValue.size() - 1) {
      s += ",";
    }
  }
  return s;
}

std::string AsmModule::toString()
{
  std::string s;
  s += ".data\n\n";
  for (u_long i = 0; i < this->globalDatas.size(); i++) {
    s += this->globalDatas[i]->toString();
    s += "\n";
  }
  s += ".text\n\n";
  s += "jal main\n j gsc";
  for (u_long i = 0; i < this->functions.size(); i++) {
    s += this->functions[i]->toString();
    s += "\n";
  }
  s += "jsc:";
  return s;
}

std::string AsmFunction::toString()
{ 
  std::string s;
  s += this->funcName->toString();
  s += ":\n";
  for (u_long i = 0; i < this->basicBlocks.size(); i++) {
    s += this->basicBlocks[i]->toString();
    s += "\n";
  }
  return s;
}

std::string AsmBasicBlock::toString()
{
  std::string s;
  s += this->blockLabel->toString();
  s += ":\n";
  for (std::list<AsmInst*>::iterator it = this->insts.begin(); 
        it != this->insts.end(); it++) {
    s += (*it)->toString();
    s += "\n";
  }
  return s;
}