//RISCV emu 

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define U8  uint8_t
#define U16 uint16_t
#define U32 uint32_t
#define U64 uint64_t

#define I8  int8_t
#define I16 int16_t
#define I32 int32_t
#define I64 int64_t


#include <assert.h>

#define WARNING(...)        fprintf(stdout, __VA_ARGS__)
#define ERROR_BREAK(...)    fprintf(stderr, __VA_ARGS__); exit(-1)
#define LOG(...)     			  fprintf(stdout, __VA_ARGS__)
#define PAUSE()           {char a; fputs(a, stdin);}
#define CLAMP(X, LOW, HIGH) {if((X) < (LOW)) (X) = (LOW); if((X) > (HIGH)) (X) = (HIGH);}
#define ASSERT(msg) {fprintf(stderr, "aseert in:\n\tFILE %s\n\tLINE %d\n\tmsg: %s" , __FILE__, __LINE__, msg); exit(-1);}
#define DROP(var) {(void)var;}




#define DRAM_SIZE 1024*1024*1
#define DRAM_BASE 0x80000000

#ifndef RISCV_DECORATOR 
	#define RISCV_DECORATOR static inline
#endif

typedef struct DRAM {
	U8 mem[DRAM_SIZE];     // Dram memory of DRAM_SIZE
} DRAM;

typedef struct BUS {
    struct DRAM dram;
} BUS;


typedef struct{
	U64 regs[32]; //Regs 32 64 bit regs
	U64 pc;
	U64 csr[4096]; //https://docs.riscv.org/reference/isa/priv/priv-csrs.html
	BUS bus;
}CPU;


//Opcode instructions 

#define LUI     0x37 
#define AUIPC   0x17 

#define JAL     0x6f 
#define JALR    0x67 

#define B_TYPE  0x63
    #define BEQ     0x0
    #define BNE     0x1
    #define BLT     0x4
    #define BGE     0x5
    #define BLTU    0x6
    #define BGEU    0x7

#define LOAD    0x03
    #define LB      0x0
    #define LH      0x1
    #define LW      0x2
    #define LD      0x3
    #define LBU     0x4
    #define LHU     0x5
    #define LWU     0x6

#define S_TYPE  0x23
    #define SB      0x0
    #define SH      0x1
    #define SW      0x2
    #define SD      0x3

#define I_TYPE  0x13
    #define ADDI    0x0
    #define SLLI    0x1
    #define SLTI    0x2
    #define SLTIU   0x3
    #define XORI    0x4
    #define SRI     0x5
        #define SRLI    0x00
        #define SRAI    0x20
    #define ORI     0x6
    #define ANDI    0x7

#define R_TYPE  0x33
    #define ADDSUB  0x0
        #define ADD     0x00
        #define SUB     0x20
    #define SLL     0x1
    #define SLT     0x2
    #define SLTU    0x3
    #define XOR     0x4
    #define SR      0x5
        #define SRL     0x00
        #define SRA     0x20
    #define OR      0x6
    #define AND     0x7

#define FENCE   0x0f

#define I_TYPE_64 0x1b
    #define ADDIW   0x0
    #define SLLIW   0x1
    #define SRIW    0x5
        #define SRLIW   0x00
        #define SRAIW   0x20

#define R_TYPE_64 0x3b
    #define ADDSUB   0x0
        #define ADDW    0x00
        #define MULW    0x01
        #define SUBW    0x20
    #define DIVW    0x4
    #define SLLW    0x1
    #define SRW     0x5
        #define SRLW   0x00
        #define DIVUW   0x01
        #define SRAW   0x20
    #define REMW    0x6
    #define REMUW   0x7

#define CSR 0x73
    #define ECALLBREAK    0x00     // contains both ECALL and EBREAK
    #define CSRRW   0x01
    #define CSRRS   0x02
    #define CSRRC   0x03
    #define CSRRWI  0x05
    #define CSRRSI  0x06
    #define CSRRCI  0x07

#define AMO_W 0x2f
    #define LR_W        0x02
    #define SC_W        0x03
    #define AMOSWAP_W   0x01
    #define AMOADD_W    0x00
    #define AMOXOR_W    0x04
    #define AMOAND_W    0x0c
    #define AMOOR_W     0x08
    #define AMOMIN_W    0x10
    #define AMOMAX_W    0x14
    #define AMOMINU_W   0x18
    #define AMOMAXU_W   0x1c

//Dram stuff 
RISCV_DECORATOR U64 dram_load_8(DRAM* dram, U64 addr){
    return (U64) dram->mem[addr - DRAM_BASE];
}
RISCV_DECORATOR U64 dram_load_16(DRAM* dram, U64 addr){
    return (U64) dram->mem[addr-DRAM_BASE]
        |  (U64) dram->mem[addr-DRAM_BASE + 1] << 8;
}
RISCV_DECORATOR U64 dram_load_32(DRAM* dram, U64 addr){
    return (U64) dram->mem[addr-DRAM_BASE]
        |  (U64) dram->mem[addr-DRAM_BASE + 1] << 8
        |  (U64) dram->mem[addr-DRAM_BASE + 2] << 16 
        |  (U64) dram->mem[addr-DRAM_BASE + 3] << 24;
}
RISCV_DECORATOR U64 dram_load_64(DRAM* dram, U64 addr){
    return (U64) dram->mem[addr-DRAM_BASE]
        |  (U64) dram->mem[addr-DRAM_BASE + 1] << 8
        |  (U64) dram->mem[addr-DRAM_BASE + 2] << 16
        |  (U64) dram->mem[addr-DRAM_BASE + 3] << 24
        |  (U64) dram->mem[addr-DRAM_BASE + 4] << 32
        |  (U64) dram->mem[addr-DRAM_BASE + 5] << 40 
        |  (U64) dram->mem[addr-DRAM_BASE + 6] << 48
        |  (U64) dram->mem[addr-DRAM_BASE + 7] << 56;
}

RISCV_DECORATOR U64 dram_load(DRAM* dram, U64 addr, U64 size) {
    switch (size) {
        case 8:  return dram_load_8(dram, addr);  break;
        case 16: return dram_load_16(dram, addr); break;
        case 32: return dram_load_32(dram, addr); break;
        case 64: return dram_load_64(dram, addr); break;
        default: ;
    }
    return 1;
}


RISCV_DECORATOR void dram_store_8(DRAM* dram, U64 addr, U64 value) {
    dram->mem[addr-DRAM_BASE] = (U8) (value & 0xff);
}
RISCV_DECORATOR void dram_store_16(DRAM* dram, U64 addr, U64 value) {
    dram->mem[addr-DRAM_BASE] = (U8) (value & 0xff);
    dram->mem[addr-DRAM_BASE+1] = (U8) ((value >> 8) & 0xff);
}
RISCV_DECORATOR void dram_store_32(DRAM* dram, U64 addr, U64 value) {
    dram->mem[addr-DRAM_BASE] = (U8) (value & 0xff);
    dram->mem[addr-DRAM_BASE + 1] = (U8) ((value >> 8) & 0xff);
    dram->mem[addr-DRAM_BASE + 2] = (U8) ((value >> 16) & 0xff);
    dram->mem[addr-DRAM_BASE + 3] = (U8) ((value >> 24) & 0xff);
}
RISCV_DECORATOR void dram_store_64(DRAM* dram, U64 addr, U64 value) {
    dram->mem[addr-DRAM_BASE] = (U8) (value & 0xff);
    dram->mem[addr-DRAM_BASE + 1] = (U8) ((value >> 8) & 0xff);
    dram->mem[addr-DRAM_BASE + 2] = (U8) ((value >> 16) & 0xff);
    dram->mem[addr-DRAM_BASE + 3] = (U8) ((value >> 24) & 0xff);
    dram->mem[addr-DRAM_BASE + 4] = (U8) ((value >> 32) & 0xff);
    dram->mem[addr-DRAM_BASE + 5] = (U8) ((value >> 40) & 0xff);
    dram->mem[addr-DRAM_BASE + 6] = (U8) ((value >> 48) & 0xff);
    dram->mem[addr-DRAM_BASE + 7] = (U8) ((value >> 56) & 0xff);
}
//MAYBE size const 
RISCV_DECORATOR void dram_store(DRAM* dram, U64 addr, U64 size, U64 value) {
    switch (size) {
        case 8:  dram_store_8(dram, addr, value);  break;
        case 16: dram_store_16(dram, addr, value); break;
        case 32: dram_store_32(dram, addr, value); break;
        case 64: dram_store_64(dram, addr, value); break;
        default: ;
    }
}

//Bus stuff just a wraper around dram 

RISCV_DECORATOR U64 bus_load(BUS* bus, U64 addr, U64 size) {
    return dram_load(&(bus->dram), addr, size);
}
RISCV_DECORATOR void bus_store(BUS* bus, U64 addr, U64 size, U64 value) {
    dram_store(&(bus->dram), addr, size, value);
}


//CSR stuff
RISCV_DECORATOR U64 csr_read(CPU* cpu,U64  csr) {
    return (U64)(U32)cpu->csr[csr];
}

RISCV_DECORATOR void csr_write(CPU* cpu, U64 csr, U64 value) {
    cpu->csr[csr] = value;
}

//Cpu stuff
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[31m"
#define ANSI_RESET   "\x1b[0m"

#define ADDR_MISALIGNED(addr) (addr & 0x3)


// print operation for DEBUG
RISCV_DECORATOR void print_op(char* s) {
    printf("%s%s%s", ANSI_BLUE, s, ANSI_RESET);
}

RISCV_DECORATOR void cpu_init(CPU *cpu) {
    cpu->regs[0] = 0x00;                    // register x0 hardwired to 0
    cpu->regs[2] = DRAM_BASE + DRAM_SIZE;   // Set stack pointer
    cpu->pc      = DRAM_BASE;               // Set program counter to the base address
}

RISCV_DECORATOR U32 cpu_fetch(CPU *cpu) {
    U32 inst  = bus_load(&(cpu->bus), cpu->pc, 32);
    return inst;
}

RISCV_DECORATOR U64 cpu_load(CPU* cpu, U64 addr, U64 size) {
    return bus_load(&(cpu->bus), addr, size);
}

RISCV_DECORATOR void cpu_store(CPU* cpu, U64 addr, U64 size, U64 value) {
    bus_store(&(cpu->bus), addr, size, value);
}

//=====================================================================================
// Instruction Decoder Functions
//=====================================================================================

RISCV_DECORATOR U64 rd(U32 inst ) {
    return (inst >> 7) & 0x1f;    // rd in bits 11..7
}

RISCV_DECORATOR U64 rs1(U32 inst ) {
    return (inst >> 15) & 0x1f;   // rs1 in bits 19..15
}
U64 rs2(U32 inst ) {
    return (inst >> 20) & 0x1f;   // rs2 in bits 24..20
}

RISCV_DECORATOR U64 imm_I(U32 inst ) {
    // imm[11:0] = inst[31:20]
    return ((I64)(I32) (inst & 0xfff00000)) >> 20; // right shift as signed?
}

RISCV_DECORATOR U64 imm_S(U32 inst ) {
    // imm[11:5] = inst[31:25], imm[4:0] = inst[11:7]
    return ((I64)(I32)(inst & 0xfe000000) >> 20)
        | ((inst >> 7) & 0x1f); 
}
RISCV_DECORATOR U64 imm_B(U32 inst ) {
    // imm[12|10:5|4:1|11] = inst[31|30:25|11:8|7]
    return ((I64)(I32)(inst & 0x80000000) >> 19)
        | ((inst & 0x80) << 4) // imm[11]
        | ((inst >> 20) & 0x7e0) // imm[10:5]
        | ((inst >> 7) & 0x1e); // imm[4:1]
}
RISCV_DECORATOR U64 imm_U(U32 inst) {
    // imm[31:12] = inst[31:12]
    return (I64)(I32)(inst & 0xfffff999);
}

RISCV_DECORATOR U64 imm_J(U32 inst) {
    // imm[20|10:1|11|19:12] = inst[31|30:21|20|19:12]
    return (U64)((I64)(I32)(inst & 0x80000000) >> 11)
        | (inst & 0xff000) // imm[19:12]
        | ((inst >> 9) & 0x800) // imm[11]
        | ((inst >> 20) & 0x7fe); // imm[10:1]
}

RISCV_DECORATOR U32 shamt(U32 inst) {
    // shamt(shift amount) only required for immediate shift instructions
    // shamt[4:5] = imm[5:0]
    return (U32) (imm_I(inst) & 0x1f); // TODO: 0x1f / 0x3f ?
}

RISCV_DECORATOR U64 csr(U32 inst) {
    // csr[11:0] = inst[31:20]
    return ((inst & 0xfff00000) >> 20);
}

//=====================================================================================
//   Instruction Execution Functions
//=====================================================================================

RISCV_DECORATOR void exec_LUI(CPU* cpu, U32 inst) {
    // LUI places upper 20 bits of U-immediate value to rd
    cpu->regs[rd(inst)] = (U64)(I64)(I32)(inst & 0xfffff000);
    print_op("lui\n");
}

RISCV_DECORATOR void exec_AUIPC(CPU* cpu, U32 inst) {
    // AUIPC forms a 32-bit offset from the 20 upper bits 
    // of the U-immediate
    U64 imm = imm_U(inst);
    cpu->regs[rd(inst)] = ((I64) cpu->pc + (I64) imm) - 4;
    print_op("auipc\n");
}


RISCV_DECORATOR void exec_JAL(CPU* cpu, U32 inst) {
    U64 imm = imm_J(inst);
    cpu->regs[rd(inst)] = cpu->pc;
    /*print_op("JAL-> rd:%ld, pc:%lx\n", rd(inst), cpu->pc);*/
    cpu->pc = cpu->pc + (I64) imm - 4;
    print_op("jal\n");
    if (ADDR_MISALIGNED(cpu->pc)) {
        fprintf(stderr, "JAL pc address misalligned");
        exit(0);
    }
}


RISCV_DECORATOR void exec_JALR(CPU* cpu, U32 inst) {
    U64 imm = imm_I(inst);
    U64 tmp = cpu->pc;
    cpu->pc = (cpu->regs[rs1(inst)] + (I64) imm) & 0xfffffffe;
    cpu->regs[rd(inst)] = tmp;
    /*print_op("NEXT -> %#lx, imm:%#lx\n", cpu->pc, imm);*/
    print_op("jalr\n");
    if (ADDR_MISALIGNED(cpu->pc)) {
        fprintf(stderr, "JAL pc address misalligned");
        exit(0);
    }
}


RISCV_DECORATOR void exec_BEQ(CPU* cpu, U32 inst) {
    U64 imm = imm_B(inst);
    if ((I64) cpu->regs[rs1(inst)] == (I64) cpu->regs[rs2(inst)])
        cpu->pc = cpu->pc + (I64) imm - 4;
    print_op("beq\n");
}

RISCV_DECORATOR void exec_BNE(CPU* cpu, U32 inst) {
    U64 imm = imm_B(inst);
    if ((I64) cpu->regs[rs1(inst)] != (I64) cpu->regs[rs2(inst)])
        cpu->pc = (cpu->pc + (I64) imm - 4);
    print_op("bne\n");
}

RISCV_DECORATOR void exec_BLT(CPU* cpu, U32 inst) {
    /*print_op("Operation: BLT\n");*/
    U64 imm = imm_B(inst);
    if ((I64) cpu->regs[rs1(inst)] < (I64) cpu->regs[rs2(inst)])
        cpu->pc = cpu->pc + (I64) imm - 4;
    print_op("blt\n");
}

RISCV_DECORATOR void exec_BGE(CPU* cpu, U32 inst) {
    U64 imm = imm_B(inst);
    if ((I64) cpu->regs[rs1(inst)] >= (I64) cpu->regs[rs2(inst)])
        cpu->pc = cpu->pc + (I64) imm - 4;
    print_op("bge\n");
}

RISCV_DECORATOR void exec_BLTU(CPU* cpu, U32 inst) {
    U64 imm = imm_B(inst);
    if (cpu->regs[rs1(inst)] < cpu->regs[rs2(inst)])
        cpu->pc = cpu->pc + (I64) imm - 4;
    print_op("bltu\n");
}

RISCV_DECORATOR void exec_BGEU(CPU* cpu, U32 inst) {
    U64 imm = imm_B(inst);
    if (cpu->regs[rs1(inst)] >= cpu->regs[rs2(inst)])
        cpu->pc = (I64) cpu->pc + (I64) imm - 4;
    print_op("jal\n");
}

RISCV_DECORATOR void exec_LB(CPU* cpu, U32 inst) {
    // load 1 byte to rd from address in rs1
    U64 imm = imm_I(inst);
    U64 addr = cpu->regs[rs1(inst)] + (I64) imm;
    cpu->regs[rd(inst)] = (I64)(I8) cpu_load(cpu, addr, 8);
    print_op("lb\n");
}

RISCV_DECORATOR void exec_LH(CPU* cpu, U32 inst) {
    // load 2 byte to rd from address in rs1
    U64 imm = imm_I(inst);
    U64 addr = cpu->regs[rs1(inst)] + (I64) imm;
    cpu->regs[rd(inst)] = (I64)(I16) cpu_load(cpu, addr, 16);
    print_op("lh\n");
}

RISCV_DECORATOR void exec_LW(CPU* cpu, U32 inst) {
    // load 4 byte to rd from address in rs1
    U64 imm = imm_I(inst);
    U64 addr = cpu->regs[rs1(inst)] + (I64) imm;
    cpu->regs[rd(inst)] = (I64)(I32) cpu_load(cpu, addr, 32);
    print_op("lw\n");
}

RISCV_DECORATOR void exec_LD(CPU* cpu, U32 inst) {
    // load 8 byte to rd from address in rs1
    U64 imm = imm_I(inst);
    U64 addr = cpu->regs[rs1(inst)] + (I64) imm;
    cpu->regs[rd(inst)] = (I64) cpu_load(cpu, addr, 64);
    print_op("ld\n");
}

RISCV_DECORATOR void exec_LBU(CPU* cpu, U32 inst) {
    // load unsigned 1 byte to rd from address in rs1
    U64 imm = imm_I(inst);
    U64 addr = cpu->regs[rs1(inst)] + (I64) imm;
    cpu->regs[rd(inst)] = cpu_load(cpu, addr, 8);
    print_op("lbu\n");
}

RISCV_DECORATOR void exec_LHU(CPU* cpu, U32 inst) {
    // load unsigned 2 byte to rd from address in rs1
    U64 imm = imm_I(inst);
    U64 addr = cpu->regs[rs1(inst)] + (I64) imm;
    cpu->regs[rd(inst)] = cpu_load(cpu, addr, 16);
    print_op("lhu\n");
}

RISCV_DECORATOR void exec_LWU(CPU* cpu, U32 inst) {
    // load unsigned 2 byte to rd from address in rs1
    U64 imm = imm_I(inst);
    U64 addr = cpu->regs[rs1(inst)] + (I64) imm;
    cpu->regs[rd(inst)] = cpu_load(cpu, addr, 32);
    print_op("lwu\n");
}

RISCV_DECORATOR void exec_SB(CPU* cpu, U32 inst) {
    U64 imm = imm_S(inst);
    U64 addr = cpu->regs[rs1(inst)] + (I64) imm;
    cpu_store(cpu, addr, 8, cpu->regs[rs2(inst)]);
    print_op("sb\n");
}

RISCV_DECORATOR void exec_SH(CPU* cpu, U32 inst) {
    U64 imm = imm_S(inst);
    U64 addr = cpu->regs[rs1(inst)] + (I64) imm;
    cpu_store(cpu, addr, 16, cpu->regs[rs2(inst)]);
    print_op("sh\n");
}

RISCV_DECORATOR void exec_SW(CPU* cpu, U32 inst) {
    U64 imm = imm_S(inst);
    U64 addr = cpu->regs[rs1(inst)] + (I64) imm;
    cpu_store(cpu, addr, 32, cpu->regs[rs2(inst)]);
    print_op("sw\n");
}

RISCV_DECORATOR void exec_SD(CPU* cpu, U32 inst) {
    U64 imm = imm_S(inst);
    U64 addr = cpu->regs[rs1(inst)] + (I64) imm;
    cpu_store(cpu, addr, 64, cpu->regs[rs2(inst)]);
    print_op("sd\n");
}


RISCV_DECORATOR void exec_ADDI(CPU* cpu, U32 inst) {
    U64 imm = imm_I(inst);
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] + (I64) imm;
    print_op("addi\n");
}


RISCV_DECORATOR void exec_SLLI(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] << shamt(inst);
    print_op("slli\n");
}


RISCV_DECORATOR void exec_SLTI(CPU* cpu, U32 inst) {
    U64 imm = imm_I(inst);
    cpu->regs[rd(inst)] = (cpu->regs[rs1(inst)] < (I64) imm)?1:0;
    print_op("slti\n");
}


RISCV_DECORATOR void exec_SLTIU(CPU* cpu, U32 inst) {
    U64 imm = imm_I(inst);
    cpu->regs[rd(inst)] = (cpu->regs[rs1(inst)] < imm)?1:0;
    print_op("sltiu\n");
}


RISCV_DECORATOR void exec_XORI(CPU* cpu, U32 inst) {
    U64 imm = imm_I(inst);
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] ^ imm;
    print_op("xori\n");
}


RISCV_DECORATOR void exec_SRLI(CPU* cpu, U32 inst) {
    U64 imm = imm_I(inst);
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] >> imm;
    print_op("srli\n");
}


RISCV_DECORATOR void exec_SRAI(CPU* cpu, U32 inst) {
    U64 imm = imm_I(inst);
    cpu->regs[rd(inst)] = (I32)cpu->regs[rs1(inst)] >> imm;
    print_op("srai\n");
}


RISCV_DECORATOR void exec_ORI(CPU* cpu, U32 inst) {
    U64 imm = imm_I(inst);
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] | imm;
    print_op("ori\n");
}


RISCV_DECORATOR void exec_ANDI(CPU* cpu, U32 inst) {
    U64 imm = imm_I(inst);
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] & imm;
    print_op("andi\n");
}


RISCV_DECORATOR void exec_ADD(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] =
        (U64) ((I64)cpu->regs[rs1(inst)] + (I64)cpu->regs[rs2(inst)]);
    print_op("add\n");
}


RISCV_DECORATOR void exec_SUB(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] =
        (U64) ((I64)cpu->regs[rs1(inst)] - (I64)cpu->regs[rs2(inst)]);
    print_op("sub\n");
}


RISCV_DECORATOR void exec_SLL(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] << (I64)cpu->regs[rs2(inst)];
    print_op("sll\n");
}


RISCV_DECORATOR void exec_SLT(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = (cpu->regs[rs1(inst)] < (I64) cpu->regs[rs2(inst)])?1:0;
    print_op("slt\n");
}


RISCV_DECORATOR void exec_SLTU(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = (cpu->regs[rs1(inst)] < cpu->regs[rs2(inst)])?1:0;
    print_op("slti\n");
}


RISCV_DECORATOR void exec_XOR(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] ^ cpu->regs[rs2(inst)];
    print_op("xor\n");
}


RISCV_DECORATOR void exec_SRL(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] >> cpu->regs[rs2(inst)];
    print_op("srl\n");
}


RISCV_DECORATOR void exec_SRA(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = (I32)cpu->regs[rs1(inst)] >> 
        (I64) cpu->regs[rs2(inst)];
    print_op("sra\n");
}


RISCV_DECORATOR void exec_OR(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] | cpu->regs[rs2(inst)];
    print_op("or\n");
}


RISCV_DECORATOR void exec_AND(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] & cpu->regs[rs2(inst)];
    print_op("and\n");
}


RISCV_DECORATOR void exec_FENCE(CPU* cpu, U32 inst) {
    print_op("fence\n");
}


RISCV_DECORATOR void exec_ECALL(CPU* cpu, U32 inst) {}

RISCV_DECORATOR void exec_EBREAK(CPU* cpu, U32 inst) {}


RISCV_DECORATOR void exec_ECALLBREAK(CPU* cpu, U32 inst) {
    if (imm_I(inst) == 0x0)
        exec_ECALL(cpu, inst);
    if (imm_I(inst) == 0x1)
        exec_EBREAK(cpu, inst);
    print_op("ecallbreak\n");
}



RISCV_DECORATOR void exec_ADDIW(CPU* cpu, U32 inst) {
    U64 imm = imm_I(inst);
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] + (I64) imm;
    print_op("addiw\n");
}

//

RISCV_DECORATOR void exec_SLLIW(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = (I64)(I32) (cpu->regs[rs1(inst)] <<  shamt(inst));
    print_op("slliw\n");
}

RISCV_DECORATOR void exec_SRLIW(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = (I64)(I32) (cpu->regs[rs1(inst)] >>  shamt(inst));
    print_op("srliw\n");
}

RISCV_DECORATOR void exec_SRAIW(CPU* cpu, U32 inst) {
    U64 imm = imm_I(inst);
    cpu->regs[rd(inst)] = (I64)(I32) (cpu->regs[rs1(inst)] >> (U64)(I64)(I32) imm);
    print_op("sraiw\n");
}

RISCV_DECORATOR void exec_ADDW(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = (I64)(I32) (cpu->regs[rs1(inst)] 
            + (I64) cpu->regs[rs2(inst)]);
    print_op("addw\n");
}

RISCV_DECORATOR void exec_MULW(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = (I64)(I32) (cpu->regs[rs1(inst)] 
            * (I64) cpu->regs[rs2(inst)]);
    print_op("mulw\n");
}

RISCV_DECORATOR void exec_SUBW(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = (I64)(I32) (cpu->regs[rs1(inst)] 
            - (I64) cpu->regs[rs2(inst)]);
    print_op("subw\n");
}

RISCV_DECORATOR void exec_DIVW(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = (I64)(I32) (cpu->regs[rs1(inst)] 
            / (I64) cpu->regs[rs2(inst)]);
    print_op("divw\n");
}

RISCV_DECORATOR void exec_SLLW(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = (I64)(I32) (cpu->regs[rs1(inst)] <<  cpu->regs[rs2(inst)]);
    print_op("sllw\n");
}

RISCV_DECORATOR void exec_SRLW(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = (I64)(I32) (cpu->regs[rs1(inst)] >>  cpu->regs[rs2(inst)]);
    print_op("srlw\n");
}

RISCV_DECORATOR void exec_DIVUW(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] / (I64) cpu->regs[rs2(inst)];
    print_op("divuw\n");
}

RISCV_DECORATOR void exec_SRAW(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = (I64)(I32) (cpu->regs[rs1(inst)] >>  (U64)(I64)(I32) cpu->regs[rs2(inst)]);
    print_op("sraw\n");
}

RISCV_DECORATOR void exec_REMW(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = (I64)(I32) (cpu->regs[rs1(inst)] 
            % (I64) cpu->regs[rs2(inst)]);
    print_op("remw\n");
}

RISCV_DECORATOR void exec_REMUW(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = cpu->regs[rs1(inst)] % (I64) cpu->regs[rs2(inst)];
    print_op("remuw\n");
}

// CSR instructions

RISCV_DECORATOR void exec_CSRRW(CPU* cpu, U32 inst) {
    cpu->regs[rd(inst)] = csr_read(cpu, csr(inst));
    csr_write(cpu, csr(inst), cpu->regs[rs1(inst)]);
    print_op("csrrw\n");
}

RISCV_DECORATOR void exec_CSRRS(CPU* cpu, U32 inst) {
    csr_write(cpu, csr(inst), cpu->csr[csr(inst)] | cpu->regs[rs1(inst)]);
    print_op("csrrs\n");
}
RISCV_DECORATOR void exec_CSRRC(CPU* cpu, U32 inst) {
    csr_write(cpu, csr(inst), cpu->csr[csr(inst)] & !(cpu->regs[rs1(inst)]) );
    print_op("csrrc\n");
}
RISCV_DECORATOR void exec_CSRRWI(CPU* cpu, U32 inst) {
    csr_write(cpu, csr(inst), rs1(inst));
    print_op("csrrwi\n");
}
RISCV_DECORATOR void exec_CSRRSI(CPU* cpu, U32 inst) {
    csr_write(cpu, csr(inst), cpu->csr[csr(inst)] | rs1(inst));
    print_op("csrrsi\n");
}
RISCV_DECORATOR void exec_CSRRCI(CPU* cpu, U32 inst) {
    csr_write(cpu, csr(inst), cpu->csr[csr(inst)] & !rs1(inst));
    print_op("csrrci\n");
}

// AMO_W
RISCV_DECORATOR void exec_LR_W(CPU* cpu, U32 inst) {}  
RISCV_DECORATOR void exec_SC_W(CPU* cpu, U32 inst) {}  
RISCV_DECORATOR void exec_AMOSWAP_W(CPU* cpu, U32 inst) {}  
RISCV_DECORATOR void exec_AMOADD_W(CPU* cpu, U32 inst) {
    U32 tmp = cpu_load(cpu, cpu->regs[rs1(inst)], 32);
    U32 res = tmp + (U32)cpu->regs[rs2(inst)];
    cpu->regs[rd(inst)] = tmp;
    cpu_store(cpu, cpu->regs[rs1(inst)], 32, res);
    print_op("amoadd.w\n");
} 
RISCV_DECORATOR void exec_AMOXOR_W(CPU* cpu, U32 inst) {
    U32 tmp = cpu_load(cpu, cpu->regs[rs1(inst)], 32);
    U32 res = tmp ^ (U32)cpu->regs[rs2(inst)];
    cpu->regs[rd(inst)] = tmp;
    cpu_store(cpu, cpu->regs[rs1(inst)], 32, res);
    print_op("amoxor.w\n");
} 
RISCV_DECORATOR void exec_AMOAND_W(CPU* cpu, U32 inst) {
    U32 tmp = cpu_load(cpu, cpu->regs[rs1(inst)], 32);
    U32 res = tmp & (U32)cpu->regs[rs2(inst)];
    cpu->regs[rd(inst)] = tmp;
    cpu_store(cpu, cpu->regs[rs1(inst)], 32, res);
    print_op("amoand.w\n");
} 
RISCV_DECORATOR void exec_AMOOR_W(CPU* cpu, U32 inst) {
    U32 tmp = cpu_load(cpu, cpu->regs[rs1(inst)], 32);
    U32 res = tmp | (U32)cpu->regs[rs2(inst)];
    cpu->regs[rd(inst)] = tmp;
    cpu_store(cpu, cpu->regs[rs1(inst)], 32, res);
    print_op("amoor.w\n");
} 
RISCV_DECORATOR void exec_AMOMIN_W(CPU* cpu, U32 inst) {} 
RISCV_DECORATOR void exec_AMOMAX_W(CPU* cpu, U32 inst) {} 
RISCV_DECORATOR void exec_AMOMINU_W(CPU* cpu, U32 inst) {} 
RISCV_DECORATOR void exec_AMOMAXU_W(CPU* cpu, U32 inst) {} 

// AMO_D TODO
RISCV_DECORATOR void exec_LR_D(CPU* cpu, U32 inst) {}  
RISCV_DECORATOR void exec_SC_D(CPU* cpu, U32 inst) {}  
RISCV_DECORATOR void exec_AMOSWAP_D(CPU* cpu, U32 inst) {}  
RISCV_DECORATOR void exec_AMOADD_D(CPU* cpu, U32 inst) {
    U32 tmp = cpu_load(cpu, cpu->regs[rs1(inst)], 32);
    U32 res = tmp + (U32)cpu->regs[rs2(inst)];
    cpu->regs[rd(inst)] = tmp;
    cpu_store(cpu, cpu->regs[rs1(inst)], 32, res);
    print_op("amoadd.w\n");
} 
RISCV_DECORATOR void exec_AMOXOR_D(CPU* cpu, U32 inst) {
    U32 tmp = cpu_load(cpu, cpu->regs[rs1(inst)], 32);
    U32 res = tmp ^ (U32)cpu->regs[rs2(inst)];
    cpu->regs[rd(inst)] = tmp;
    cpu_store(cpu, cpu->regs[rs1(inst)], 32, res);
    print_op("amoxor.w\n");
} 
RISCV_DECORATOR void exec_AMOAND_D(CPU* cpu, U32 inst) {
    U32 tmp = cpu_load(cpu, cpu->regs[rs1(inst)], 32);
    U32 res = tmp & (U32)cpu->regs[rs2(inst)];
    cpu->regs[rd(inst)] = tmp;
    cpu_store(cpu, cpu->regs[rs1(inst)], 32, res);
    print_op("amoand.w\n");
} 
RISCV_DECORATOR void exec_AMOOR_D(CPU* cpu, U32 inst) {
    U32 tmp = cpu_load(cpu, cpu->regs[rs1(inst)], 32);
    U32 res = tmp | (U32)cpu->regs[rs2(inst)];
    cpu->regs[rd(inst)] = tmp;
    cpu_store(cpu, cpu->regs[rs1(inst)], 32, res);
    print_op("amoor.w\n");
} 
RISCV_DECORATOR void exec_AMOMIN_D(CPU* cpu, U32 inst) {} 
RISCV_DECORATOR void exec_AMOMAX_D(CPU* cpu, U32 inst) {} 
RISCV_DECORATOR void exec_AMOMINU_D(CPU* cpu, U32 inst) {} 
RISCV_DECORATOR void exec_AMOMAXU_D(CPU* cpu, U32 inst) {} 

int cpu_execute(CPU *cpu, U32 inst) {
    int opcode = inst & 0x7f;           // opcode in bits 6..0
    int funct3 = (inst >> 12) & 0x7;    // funct3 in bits 14..12
    int funct7 = (inst >> 25) & 0x7f;   // funct7 in bits 31..25

    cpu->regs[0] = 0;                   // x0 hardwired to 0 at each cycle

    /*printf("%s\n%#.8lx -> Inst: %#.8x <OpCode: %#.2x, funct3:%#x, funct7:%#x> %s",*/
            /*ANSI_YELLOW, cpu->pc-4, inst, opcode, funct3, funct7, ANSI_RESET); // DEBUG*/
    printf("%s\n%#.8lx -> %s", ANSI_YELLOW, cpu->pc-4, ANSI_RESET); // DEBUG

    switch (opcode) {
        case LUI:   exec_LUI(cpu, inst); break;
        case AUIPC: exec_AUIPC(cpu, inst); break;

        case JAL:   exec_JAL(cpu, inst); break;
        case JALR:  exec_JALR(cpu, inst); break;

        case B_TYPE:
            switch (funct3) {
                case BEQ:   exec_BEQ(cpu, inst); break;
                case BNE:   exec_BNE(cpu, inst); break;
                case BLT:   exec_BLT(cpu, inst); break;
                case BGE:   exec_BGE(cpu, inst); break;
                case BLTU:  exec_BLTU(cpu, inst); break;
                case BGEU:  exec_BGEU(cpu, inst); break;
                default: ;
            } break;

        case LOAD:
            switch (funct3) {
                case LB  :  exec_LB(cpu, inst); break;  
                case LH  :  exec_LH(cpu, inst); break;  
                case LW  :  exec_LW(cpu, inst); break;  
                case LD  :  exec_LD(cpu, inst); break;  
                case LBU :  exec_LBU(cpu, inst); break; 
                case LHU :  exec_LHU(cpu, inst); break; 
                case LWU :  exec_LWU(cpu, inst); break; 
                default: ;
            } break;

        case S_TYPE:
            switch (funct3) {
                case SB  :  exec_SB(cpu, inst); break;  
                case SH  :  exec_SH(cpu, inst); break;  
                case SW  :  exec_SW(cpu, inst); break;  
                case SD  :  exec_SD(cpu, inst); break;  
                default: ;
            } break;

        case I_TYPE:  
            switch (funct3) {
                case ADDI:  exec_ADDI(cpu, inst); break;
                case SLLI:  exec_SLLI(cpu, inst); break;
                case SLTI:  exec_SLTI(cpu, inst); break;
                case SLTIU: exec_SLTIU(cpu, inst); break;
                case XORI:  exec_XORI(cpu, inst); break;
                case SRI:   
                    switch (funct7) {
                        case SRLI:  exec_SRLI(cpu, inst); break;
                        case SRAI:  exec_SRAI(cpu, inst); break;
                        default: ;
                    } break;
                case ORI:   exec_ORI(cpu, inst); break;
                case ANDI:  exec_ANDI(cpu, inst); break;
                default:
                    fprintf(stderr, 
                            "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct7:0x%x\n"
                            , opcode, funct3, funct7);
                    return 0;
            } break;

        case R_TYPE:  
            switch (funct3) {
                case ADDSUB:
                    switch (funct7) {
                        case ADD: exec_ADD(cpu, inst);
                        case SUB: exec_ADD(cpu, inst);
                        default: ;
                    } break;
                case SLL:  exec_SLL(cpu, inst); break;
                case SLT:  exec_SLT(cpu, inst); break;
                case SLTU: exec_SLTU(cpu, inst); break;
                case XOR:  exec_XOR(cpu, inst); break;
                case SR:   
                    switch (funct7) {
                        case SRL:  exec_SRL(cpu, inst); break;
                        case SRA:  exec_SRA(cpu, inst); break;
                        default: ;
                    }
                case OR:   exec_OR(cpu, inst); break;
                case AND:  exec_AND(cpu, inst); break;
                default:
                    fprintf(stderr, 
                            "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct7:0x%x\n"
                            , opcode, funct3, funct7);
                    return 0;
            } break;

        case FENCE: exec_FENCE(cpu, inst); break;

        case I_TYPE_64:
            switch (funct3) {
                case ADDIW: exec_ADDIW(cpu, inst); break;
                case SLLIW: exec_SLLIW(cpu, inst); break;
                case SRIW : 
                    switch (funct7) {
                        case SRLIW: exec_SRLIW(cpu, inst); break;
                        case SRAIW: exec_SRLIW(cpu, inst); break;
                    } break;
            } break;

        case R_TYPE_64:
            switch (funct3) {
                case ADDSUB:
                    switch (funct7) {
                        case ADDW:  exec_ADDW(cpu, inst); break;
                        case SUBW:  exec_SUBW(cpu, inst); break;
                        case MULW:  exec_MULW(cpu, inst); break;
                    } break;
                case DIVW:  exec_DIVW(cpu, inst); break;
                case SLLW:  exec_SLLW(cpu, inst); break;
                case SRW:
                    switch (funct7) {
                        case SRLW:  exec_SRLW(cpu, inst); break;
                        case SRAW:  exec_SRAW(cpu, inst); break;
                        case DIVUW: exec_DIVUW(cpu, inst); break;
                    } break;
                case REMW:  exec_REMW(cpu, inst); break;
                case REMUW: exec_REMUW(cpu, inst); break;
                default: ;
            } break;

        case CSR:
            switch (funct3) {
                case ECALLBREAK:    exec_ECALLBREAK(cpu, inst); break;
                case CSRRW  :  exec_CSRRW(cpu, inst); break;  
                case CSRRS  :  exec_CSRRS(cpu, inst); break;  
                case CSRRC  :  exec_CSRRC(cpu, inst); break;  
                case CSRRWI :  exec_CSRRWI(cpu, inst); break; 
                case CSRRSI :  exec_CSRRSI(cpu, inst); break; 
                case CSRRCI :  exec_CSRRCI(cpu, inst); break; 
                default:
                    fprintf(stderr, 
                            "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct7:0x%x\n"
                            , opcode, funct3, funct7);
                    return 0;
            } break;

        case AMO_W:
            switch (funct7 >> 2) { // since, funct[1:0] = aq, rl
                case LR_W      :  exec_LR_W(cpu, inst); break;  
                case SC_W      :  exec_SC_W(cpu, inst); break;  
                case AMOSWAP_W :  exec_AMOSWAP_W(cpu, inst); break;  
                case AMOADD_W  :  exec_AMOADD_W(cpu, inst); break; 
                case AMOXOR_W  :  exec_AMOXOR_W(cpu, inst); break; 
                case AMOAND_W  :  exec_AMOAND_W(cpu, inst); break; 
                case AMOOR_W   :  exec_AMOOR_W(cpu, inst); break; 
                case AMOMIN_W  :  exec_AMOMIN_W(cpu, inst); break; 
                case AMOMAX_W  :  exec_AMOMAX_W(cpu, inst); break; 
                case AMOMINU_W :  exec_AMOMINU_W(cpu, inst); break; 
                case AMOMAXU_W :  exec_AMOMAXU_W(cpu, inst); break; 
                default:
                    fprintf(stderr, 
                            "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct7:0x%x\n"
                            , opcode, funct3, funct7);
                    return 0;
            } break;

        case 0x00:
            return 0;

        default:
            fprintf(stderr, 
                    "[-] ERROR-> opcode:0x%x, funct3:0x%x, funct3:0x%x\n"
                    , opcode, funct3, funct7);
            return 0;
            /*exit(1);*/
    }
    return 1;
}

RISCV_DECORATOR void dump_registers(CPU *cpu) {
    char* abi[] = { // Application Binary Interface registers
        "zero", "ra",  "sp",  "gp",
          "tp", "t0",  "t1",  "t2",
          "s0", "s1",  "a0",  "a1",
          "a2", "a3",  "a4",  "a5",
          "a6", "a7",  "s2",  "s3",
          "s4", "s5",  "s6",  "s7",
          "s8", "s9", "s10", "s11",
          "t3", "t4",  "t5",  "t6",
    };

    /*for (int i=0; i<8; i++) {*/
        /*printf("%4s| x%02d: %#-8.2lx\t", abi[i],    i,    cpu->regs[i]);*/
        /*printf("%4s| x%02d: %#-8.2lx\t", abi[i+8],  i+8,  cpu->regs[i+8]);*/
        /*printf("%4s| x%02d: %#-8.2lx\t", abi[i+16], i+16, cpu->regs[i+16]);*/
        /*printf("%4s| x%02d: %#-8.2lx\n", abi[i+24], i+24, cpu->regs[i+24]);*/
    /*}*/

    for (int i=0; i<8; i++) {
        printf("   %4s: %#-13.2lx  ", abi[i],    cpu->regs[i]);
        printf("   %2s: %#-13.2lx  ", abi[i+8],  cpu->regs[i+8]);
        printf("   %2s: %#-13.2lx  ", abi[i+16], cpu->regs[i+16]);
        printf("   %3s: %#-13.2lx\n", abi[i+24], cpu->regs[i+24]);
    }

}

void read_file(CPU* cpu, char *filename)
{
	FILE *file;
	uint8_t *buffer;
	unsigned long fileLen;

	//Open file
	file = fopen(filename, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", filename);
	}

	//Get file length
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);

	//Allocate memory
	buffer=(uint8_t *)malloc(fileLen+1);
	if (!buffer)
	{
		fprintf(stderr, "Memory error!");
        fclose(file);
	}

	//Read file contents into buffer
	fread(buffer, fileLen, 1, file);
	fclose(file);

    // Print file contents in hex
    /*for (int i=0; i<fileLen; i+=2) {*/
        /*if (i%16==0) printf("\n%.8x: ", i);*/
        /*printf("%02x%02x ", *(buffer+i), *(buffer+i+1));*/
    /*}*/
    /*printf("\n");*/

    // copy the bin executable to dram
    memcpy(cpu->bus.dram.mem, buffer, fileLen*sizeof(uint8_t));
	free(buffer);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: rvemu <filename>\n");
        exit(1);
    }

    // Initialize cpu, registers and program counter
    static CPU cpu;
    cpu_init(&cpu);
    // Read input file
    read_file(&cpu, argv[1]);

    // cpu loop
    while (1) {
        // fetch
        uint32_t inst = cpu_fetch(&cpu);
        // Increment the program counter
        cpu.pc += 4;
        // execute
        if (!cpu_execute(&cpu, inst))
            break;

        dump_registers(&cpu);

        if(cpu.pc==0)
            break;
    }
    /*dump_registers(&cpu);*/
    return 0;
}
