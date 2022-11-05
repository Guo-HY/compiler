
#ifndef _IR_UTILS
#define _IR_UTILS 1

#include "ir.hpp"

inline bool isModule(Value* v) {
  return v->valueIdtfr == ValueIdtfr::MODULE_VI;
}
inline bool isFunction(Value* v) {
  return v->valueIdtfr == ValueIdtfr::FUNCTION_VI;
}
inline bool isBasicBlock(Value* v) {
  return v->valueIdtfr == ValueIdtfr::BASICBLOCK_VI;
}
inline bool isNumberConstant(Value* v) {
  return v->valueIdtfr == ValueIdtfr::NUMBER_CONSTANT_VI;
}
inline bool isStringConstant(Value* v) {
  return v->valueIdtfr == ValueIdtfr::STRING_CONSTANT_VI;
}
inline bool isLabelValue(Value* v) {
  return v->valueIdtfr == ValueIdtfr::LABEL_VI;
}
inline bool isVirtRegValue(Value* v) {
  return v->valueIdtfr == ValueIdtfr::VIRTREG_VI;
}
inline bool isFuncFParamValue(Value* v) {
  return v->valueIdtfr == ValueIdtfr::FUNCFPARAM_VI;
}
inline bool isGlobalValue(Value* v) {
  return v->valueIdtfr == ValueIdtfr::GLOBAL_VAR_VI;
}
inline bool isGlobalInitValue(Value* v) {
  return v->valueIdtfr == ValueIdtfr::GLOBALINIT_VI;
}
inline bool isInstruction(Value* v) {
  return v->valueIdtfr == ValueIdtfr::INSTRUCTION_VI;
}
inline bool isBinaryInst(Instruction* i) {
  return i->instType == InstIdtfr::BINARY_II;
}
inline bool isAllocaInst(Instruction* i) {
  return i->instType == InstIdtfr::ALLOCA_II;
}
inline bool isLoadInst(Instruction* i) {
  return i->instType == InstIdtfr::LOAD_II;
}
inline bool isStoreInst(Instruction* i) {
  return i->instType == InstIdtfr::STORE_II;
}
inline bool isGEPInst(Instruction* i) {
  return i->instType == InstIdtfr::GEP_II;
}
inline bool isZextInst(Instruction* i) {
  return i->instType == InstIdtfr::ZEXT_II;
}
inline bool isIcmpInst(Instruction* i) {
  return i->instType == InstIdtfr::ICMP_II;
}
inline bool isCallInst(Instruction* i) {
  return i->instType == InstIdtfr::CALL_II;
}
inline bool isRetInst(Instruction* i) {
  return i->instType == InstIdtfr::RET_II;
}
inline bool isBrInst(Instruction* i) {
  return i->instType == InstIdtfr::BR_II;
}


#endif