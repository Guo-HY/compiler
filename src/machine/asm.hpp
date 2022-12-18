#ifndef _ASM_H
#define _ASM_H 1

#include <string>
#include <vector>
#include <initializer_list>
#include <list>
#include <utility>
#include <unordered_map>

class AsmOperand;
class AsmReg;
class AsmImm;
class AsmLabel;
class AsmInst;
class AsmGlobalData;
class AsmStrData;
class AsmWordData;
class AsmModule;
class AsmFunction;
class AsmBasicBlock;

enum AsmOperandIdtfr {
  NONE_AOI,
  REG_AOI,
  IMM_AOI,
  LABEL_AOI,
};

class AsmOperand {
  public:
  AsmOperandIdtfr asmOperandIdtfr;

  AsmOperand() : asmOperandIdtfr(NONE_AOI) {}
  AsmOperand(AsmOperandIdtfr idtfr) : asmOperandIdtfr(idtfr) {}
  virtual std::string toString() = 0;
};

class AsmReg : public AsmOperand {
  public:
  bool isPhysReg; /* 为 true表示是物理寄存器，使用物理寄存器号，否则使用虚拟寄存器号 */
  int virtNumber;
  int physNumber;

  AsmReg(bool phys, int n) : AsmOperand(AsmOperandIdtfr::REG_AOI) {
    this->isPhysReg = phys;
    if (phys) {
      physNumber = n;
      virtNumber = 0;
    } else {
      virtNumber = n;
      physNumber = 0;
    }
  }
  std::string toString() override;
};

class AsmImm : public AsmOperand {
  public:
  int immediate;
  
  AsmImm(int imm) : AsmOperand(AsmOperandIdtfr::IMM_AOI), immediate(imm) {}

  std::string toString() override;
};

class AsmLabel : public AsmOperand {
  public:
  std::string label;
  int bblkLabelId;

  AsmLabel(std::string l) : AsmOperand(AsmOperandIdtfr::LABEL_AOI), label(l), bblkLabelId(-1) {}

  std::string toString() override;
};

enum AsmInstIdtfr {
  NONE_AII,
  ADD_AII,    /* add rd, rs, rt       GPR[rd] <- GPR[rs]+GPR[rt] */
  ADDU_AII,   /* addu rd, rs, rt      GPR[rd] <- GPR[rs]+GPR[rt]*/
  ADDIU_AII,  /* addiu rt, rs, imm    GPR[rt] <- GPR[rs]+ imm */
  SUB_AII,    /* sub rd, rs, rt       GPR[rd] <- GPR[rs] - GPR[rt] */
  SUBU_AII,   /* subu rd, rs, rt      GPR[rd] <- GPR[rs] - GPR[rt] */
  MULT_AII,   /* mult rs, rt          (HI, LO)<- GPR[rs] × GPR[rt]*/
  DIV_AII,    /* div rs, rt           (HI, LO)<- GPR[rs] / GPR[rt]商存放在LO寄存器，余数存放在HI寄存器 */
  XOR_AII,    /* xor rd, rs, rt       GPR[rd] <- GPR[rs] XOR GPR[rt] */
  AND_AII,    /* and rd, rs, rt       GPR[rd] <- GPR[rs] AND GPR[rt] */
  ANDI_AII,   /* and rd, rs, imm       GPR[rd] <- GPR[rs] AND imm */
  OR_AII,     /* or rd, rs, rt        GPR[rd] <- GPR[rs] OR GPR[rt] */
  ORI_AII,    /* or rd, rs, imm        GPR[rd] <- GPR[rs] OR imm */
  MFHI_AII,   /* mfhi rd              GPR[rd] <- HI */
  MFLO_AII,   /* mflo rd              GPR[rd] <- LO */
  LW_AII,     /* lw rt, imm(base)  GPR[rt] <- memory[GPR[base]+imm] */
  SW_AII,     /* sw rt, imm(base)  memory[GPR[base]+imm] <- GPR[rt] */
  NOP_AII,    /* nop */
  J_AII,      /* j label */
  BNE_AII,    /* bne rs, rt, offset  */
  JR_AII,     /* jr rs */
  SLTIU_AII,  /* sltiu rt, rs, imm    GPR[rt] <- (GPR[rs] < immediate) */
  SLTU_AII,   /* sltu rd, rs, rt      GPR[rd] <- (GPR[rs] < GPR[rt]) */
  SLT_AII,    /* slt rd, rs, rt      GPR[rd] <- (GPR[rs] < GPR[rt]) */
  LA_AII,     /* la rd, label */
  JAL_AII,    /* jal label */
  SYSCALL_AII,  /* syscall */
  PHI_AII,    /* result = phi [val, label] ... */
};

/* reg read / write property */
enum RWP {
  READ_RWP,
  WRITE_RWP,
};

#define WRR(w, r1, r2) {(w), WRITE_RWP}, {(r1), READ_RWP}, {(r2), READ_RWP}
#define RRR(r1, r2, r3) {(r1), READ_RWP}, {(r2), READ_RWP}, {(r3), READ_RWP}
#define R(r) {(r), READ_RWP}
#define RR(r1, r2) {(r1), READ_RWP}, {(r2), READ_RWP}
#define WR(w, r) {(w), WRITE_RWP}, {(r), READ_RWP}
#define W(w) {(w), WRITE_RWP}
#define RRR(r1, r2, r3) {(r1), READ_RWP}, {(r2), READ_RWP}, {(r3), READ_RWP}

class AsmInst {
  public:
  AsmInstIdtfr asmInstIdtfr;
  /* second是寄存器的读写属性，imm与label默认为读 */
  std::vector<std::pair<AsmOperand*, RWP>> ops; 

  AsmInst() : asmInstIdtfr(AsmInstIdtfr::NOP_AII) {}
  AsmInst(AsmInstIdtfr idtfr) : asmInstIdtfr(idtfr) {}
  AsmInst(AsmInstIdtfr idtfr, std::initializer_list<std::pair<AsmOperand*, RWP>> ops) {
    this->asmInstIdtfr = idtfr;
    for (auto i : ops) {
      this->ops.push_back(i);
    }
  }

  bool isReadRWP(RWP rwp) {
    return rwp == RWP::READ_RWP;
  }
  bool isWriteRWP(RWP rwp) {
    return rwp == RWP::WRITE_RWP;
  }

  std::string toString();
};

class AsmPhiInst : public AsmInst {
  public:
  std::vector<std::pair<AsmOperand*, AsmLabel*>> varDefs;
  AsmReg* result;

  AsmPhiInst() : AsmInst(AsmInstIdtfr::PHI_AII), result(NULL) {}
};

enum AsmGlobalDataIdtfr {
  STR_AGDI,
  WORD_AGDI,
};

class AsmGlobalData {
  public:
  AsmGlobalDataIdtfr asmGlobalDataIdtfr;
  AsmLabel* label;
  AsmGlobalData(AsmGlobalDataIdtfr idtfr, AsmLabel* l) : asmGlobalDataIdtfr(idtfr), label(l) {}
  virtual std::string toString() = 0;
};

class AsmStrData : public AsmGlobalData {
  public:
  std::string str;
  AsmStrData(AsmLabel* l, std::string s) : AsmGlobalData(AsmGlobalDataIdtfr::STR_AGDI, l), str(s) {} 
  std::string toString() override;
};

class AsmWordData : public AsmGlobalData {
  public:
  bool hasInitValue;
  std::vector<int> wordsInitValue;
  int wordNum;
  AsmWordData(AsmLabel* l, int wn) : AsmGlobalData(AsmGlobalDataIdtfr::STR_AGDI, l), hasInitValue(false), wordNum(wn) {} 
  std::string toString() override;
};

class AsmModule {
  public:
  std::vector<AsmGlobalData*> globalDatas;
  std::vector<AsmFunction*> functions;

  void addGlobalData(AsmGlobalData* data) {
    this->globalDatas.push_back(data);
  }

  void addFunction(AsmFunction* f) {
    this->functions.push_back(f);
  }
  std::string toString();
};

class AsmFunction {
  public:
  AsmLabel* funcName;
  std::vector<AsmBasicBlock*> basicBlocks;
  int stackWordSize;  /* 函数栈空间的大小(字为单位) */
  int funcCallArgsWordSize; /* 函数参数栈大小（字为单位）*/
  int raWordOffset; /* ra相对于sp的偏移（字为单位） */
  std::unordered_map<int, AsmImm*> funcArgsId2stackOffset;  /* 字节为单位 */
  std::pair<AsmImm*, AsmImm*> frameSize;  /* 字节为单位 */
  std::unordered_map<int, AsmImm*> virtRegId2stackOffset; 
  int maxVirtRegId; /* 函数中虚拟寄存器的最大编号 */

  AsmFunction(AsmLabel* name) : funcName(name), stackWordSize(0), funcCallArgsWordSize(0), 
    raWordOffset(0), maxVirtRegId(0) {}
  void addBasicBlock(AsmBasicBlock* b) {
    this->basicBlocks.push_back(b);
  }
  std::string toString();
  // void plainRegAllocator();
};

class AsmBasicBlock {
  public:
  AsmLabel* blockLabel;
  std::list<AsmInst*> insts;
  // std::unordered_map<int, AsmBasicBlock*> succeed; /* 该基本块的后继基本块 */

  AsmBasicBlock(AsmLabel* label) : blockLabel(label) {}
  void addInst(AsmInst* inst) { 
    this->insts.push_back(inst);
  }

  int getBblkId() {
    return this->blockLabel->bblkLabelId;
  }

  std::string toString();
};

#endif