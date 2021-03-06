#include <stdio.h>
#include "cpu.h"

#define DEBUG

const char* reg_names[] = { "A", "B", "C", "D", "E", "F", "G", "H" };

/* Instruction table */
typedef void (*instr_ptr) (void*, void*, void*);
instr_ptr instr_table[] = {
  fetch,
  store,
  set,
  add,
  sub,
  mul,
  divide,
  setc
};

/* Dump contents of registers */
void dump_registers(registers* regs) {
  int i; /* to iterate through general-purpose registers */

  if (regs == NULL) {
    fprintf(stderr, "dump_registers: null pointer!!");
    return;
  }

  /* Dump general-purpose registers */
  fprintf(stdout, "General purpose:\n");
  for (i = 0; i < N_REGIS; i++) {
    if (i < sizeof(reg_names)) {
      printf("Register %s: ", reg_names[i]);
    }    
    DUMPINT(stdout, regs->general[i]);
  }

  /* Dump various special-purpose registers */
  printf("Program counter:\n");
  DUMPINT(stdout, regs->prog_counter);
  printf("Memory address:\n");
  DUMPINT(stdout, regs->mem_addr);
  printf("Memory data:\n");
  DUMPINT(stdout, regs->mem_data);
  printf("Flags:\n");
  DUMPINT(stdout, regs->flags);
}

/* Dump contents of memory */
void dump_memory(memory* mem) {
  int i; /* to iterate over 32-bit words */
  uint32_t* word; /* temp word to print */

  /* Print out word-by-word */
  printf("Main memory:\n");

  word = (uint32_t*) mem->data;
  for (i = 0; i < MEMSIZE/32; i++) {
    printf("%08x\n", *word++); 
  }
}

void execute(registers* regs, memory* mem) {
  int instr_index; /* Instruction index (for table) */
  int nreg1; /* Index of first register in instruction */
  int nreg2; /* Index of second register in instruction */
  int nreg3; /* Index of second register in instruction */

  /* 'Decode' instruction */
  instr_index = GETINSTR(regs->prog_counter);

#ifdef DEBUG
  fprintf(stderr, "prog_counter = %08x\n", regs->prog_counter);
  fprintf(stderr, "Executing instruction # %u\n", instr_index);
#endif

  /* Get register args */
  nreg1 = GETOP1(regs->prog_counter);
  nreg2 = GETOP2(regs->prog_counter);
  nreg3 = GETOP3(regs->prog_counter);

#ifdef DEBUG
  fprintf(stderr, "nreg1 = %u, reg1 = %08x\n", nreg1, regs->general[nreg1]);
  fprintf(stderr, "nreg2 = %u, reg2 = %08x\n", nreg2, regs->general[nreg2]);
  fprintf(stderr, "nreg3 = %u, reg3 = %08x\n", nreg3, regs->general[nreg3]);
#endif

  /* 'Execute' the instruction */
  if (instr_index == GETINSTR(FE_OP) || instr_index == GETINSTR(ST_OP)) {
    /* This is a fetch/store instruction; need to pass in the memory */
    instr_table[instr_index](mem, regs->general + nreg1, regs->general + nreg2);
  } else if (instr_index == GETINSTR(SETC_OP)) {
    /* This is a set constant instruction; have to pass the constant value */
    instr_table[instr_index]((void*)GETCONST(regs->prog_counter),
      regs->general + nreg1, NULL);
  } else {
    instr_table[instr_index](regs->general + nreg1, regs->general + nreg2,
      regs->general + nreg3);
  }
}

void fetch(void* mem, void* addr, void* dest) {
  /* Set the value using the 'real' location in memory; the address stored in
   * 'addr' is the simulated value, i.e. relative to the beginning of the
   * simulated memory */
  *(uint32_t*)dest =
    *((uint32_t*)(((memory*)mem)->data + *(uint32_t*)addr)); 
}

void store(void* mem, void* src, void* addr) {
  /* The offset calculation must be done in bytes here */
  uint32_t* dest_addr = (uint32_t*)(((memory*)mem)->data + *(uint32_t*)addr);

#ifdef DEBUG
  fprintf(stderr, "store: mem = %016x, addr = %08x, dest_addr = %016x\n",
    mem, addr, dest_addr);
#endif

  *dest_addr = *(uint32_t*)src;
}

void set(void* src, void* dest, void* unused) {
  *(uint32_t*)dest = *(uint32_t*)src;
}

/* FIXME: add code for setting condition bits in flags register after each
 * arithmetic operation */

void add(void* op1, void* op2, void* dest) {
  *(int32_t*)dest = *(int32_t*)op1 + *(int32_t*)op2;
}

void sub(void* op1, void* op2, void* dest) {
  *(int32_t*)dest = *(int32_t*)op1 - *(int32_t*)op2;
}

void mul(void* op1, void* op2, void* dest) {
  *(int32_t*)dest = *(int32_t*)op1 * (*(int32_t*)op2);
}

void divide(void* op1, void* op2, void* dest) {
  *(int32_t*)dest = *(int32_t*)op1 / *(int32_t*)op2;
}

void setc(void* num, void* dest, void* unused) {
#ifdef DEBUG
  fprintf(stderr, "store: constant = %08x\n", (uint32_t)num);
#endif

  *(uint32_t*)dest = (uint32_t)num;
}

/* TODO */
void sll(void* num, void* src, void* dest) {
}

/* TODO */
void srl(void* num, void* src, void* dest) {
}

/* TODO */
void andb(void* op1, void* op2, void* dest) {
}

/* TODO */
void orb(void* op1, void* op2, void* dest) {
}

/* TODO */
void xorb(void* op1, void* op2, void* dest) {
}
