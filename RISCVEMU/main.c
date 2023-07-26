#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
uint32_t RAM_SIZE=1024*1024;
uint32_t START_ADDRES= 0X0000;
#define debug  1   // if you wanted to print state of cpu regs set 1 else 0
uint32_t instruct[]={0b1000100000000110011},number_of_instructions=1;  //INPUT INSTRUCTIONS
//0b1000100000000110011  add
/*void load_instruction(FILE *fajl)
{
    size_t i=0;
    fajl=fopen("lokacija","r");
    while (!feof(fajl))
    {
        fread(instruct[i],sizeof(uint32_t),1,fajl);
        i++;
    }
    number_of_instructions=i;
}
*/
typedef struct CPU
{
    uint32_t x[32];
    uint32_t pc;
    uint32_t bus;
}CPU;

typedef struct MEM
{
    uint32_t *RAM;   
}MEM;
//encodings structs

typedef struct R_TYPE
{
    uint16_t funct3,funct7;
    uint16_t  rs1,rs2;
    uint16_t rd;
 
}R_TYPE;

typedef struct I_TYPE
{
    uint16_t imm,rs1,funct3,rd;
}I_TYPE;

typedef struct S_TYPE
{
    uint16_t immH,immL ,rs1,funct3;
    uint16_t rs2;
}S_TYPE,B_TYPE;

typedef struct U_TYPE
{
    uint16_t imm, rd;
}U_TYPE,J_TYPE;

//shift (n-1)&0b(number of bits)

//encoding functions
void R_DECODING(uint32_t instruction,R_TYPE *decoded)  //ok 
{
    decoded->funct7=(instruction>>24)&0b11111111;
    decoded->funct3=(instruction>>11)&0b111; //ok
    decoded->rd=(instruction>>6)&0b11111;  //ok
    decoded->rs2=(instruction>>19)&0b11111;
    decoded->rs1=(instruction>>14)&0b11111;
    //printf("funct7: %d rs2: %d  rs1: %d  funct3: %d  rd : %d\n",decoded->funct7, decoded->rs2, decoded->rs1, decoded->funct3, decoded->rd); 
}
void I_DECODING(uint32_t instruction, I_TYPE *decoded)
{
    decoded->rd=(instruction>>6)&0b11111;  //ok
    decoded->funct3=(instruction>>11)&0b111; //ok
    decoded->rs1=(instruction>>14)&0b11111;
    decoded->imm=(instruction>>19);
    //printf("imm: %d  rs1: %d  funct3: %d  rd : %d\n",decoded->imm, decoded->rs1, decoded->funct3, decoded->rd); 
}

void SorB_DECODING(uint32_t instruction,S_TYPE *decoded)  //ok 
{
    decoded->immH=(instruction>>24)&0b11111111;
    decoded->funct3=(instruction>>11)&0b111; //ok
    decoded->immL=(instruction>>6)&0b11111;  //ok
    decoded->rs2=(instruction>>19)&0b11111;
    decoded->rs1=(instruction>>14)&0b11111;
    //printf("immH: %d rs2: %d  rs1: %d  funct3: %d  immL : %d\n",decoded->immH, decoded->rs2, decoded->rs1, decoded->funct3, decoded->immL); 
}

void UorJ_DECODING(uint32_t instruction,U_TYPE *decoded)
{
    decoded->rd=(instruction>>6)&0b11111;  
    decoded->imm=(instruction>>11);
    //printf("imm: %d  rd : %d\n",decoded->imm, decoded->rd); 
}
//fetch instruction

void execute(uint32_t instruction,MEM *memory,CPU *cpu)
{
    uint16_t opcode;
    opcode=(instruction)&0b1111111;
    if (opcode==0b0110011)  //R_TYPE    
    {
            R_TYPE decoded;
            R_DECODING(instruction,&decoded);
            if(decoded.funct3==0x0)
            {
                if(decoded.funct7==0x00)
                {
                    cpu->x[decoded.rd]=decoded.rs1+decoded.rs2;
                    printf("ADD\n");
                } //add
                else if(decoded.funct7==0x20)
                {
                        cpu->x[decoded.rd]=decoded.rs1-decoded.rs2;
                        printf("SUB\n");
                }//sub
            }
                else if(decoded.funct3==0x4)
                     {
                        cpu->x[decoded.rd]=decoded.rs1^decoded.rs2;
                        printf("XOR\n");
                     }
                else if(decoded.funct3==0x6)
                     {
                         cpu->x[decoded.rd]=decoded.rs1 | decoded.rs2;
                         printf("OR\n");
                     }//OR

                else if(decoded.funct3==0x7)
                 {
                        cpu->x[decoded.rd]=decoded.rs1 & decoded.rs2;
                        printf("AND\n");
                 }//AND

                else if(decoded.funct3==0x1)
                 {
                        cpu->x[decoded.rd]=decoded.rs1 << decoded.rs2;
                        printf("SHIFT LEFT\n");
                 }//SHIFT LEFT


                 else if(decoded.funct3==0x5) //STa JE
                 {
                    if(decoded.funct7==0x00)
                    {
                        cpu->x[decoded.rd] = decoded.rs1 >> decoded.rs2;
                        printf("sri\n");
                    }
                    if(decoded.funct7==0x20)
                    {
                        uint32_t pom=0b10000000000000000000000000000000;
                        if((instruction&pom)==1)
                        {
                                    for (size_t i = 0; i < decoded.rs2; i++)
                                     {
                                        /* code */
                                        pom=pom+(pom>>1);
                                    }
                            cpu->x[decoded.rd]=decoded.rs1|pom; 
                        }
                        else
                            cpu->x[decoded.rd] = decoded.rs1 >> decoded.rs2;
                        printf("sra\n");
                    }
                  }//SHIFT RIGHT LOGICAL AND ARITMETICAL

            else if(decoded.funct3==0x2)
                {
                    cpu->x[decoded.rd]=((decoded.rs1 < decoded.rs2) ? 1:0);
                    printf("SLT\n");
                }//SET LESS THEN 

            else if(decoded.funct3==0x3)
             {
                    cpu->x[decoded.rd]=(decoded.rs1 < decoded.rs2) ? 1:0;
                    printf("SLT\n");
             }//SET LESS THEN (ZERO EXTENDED)  
    }

    else if(opcode==0b0010011)  //I_TYPE
    {
        I_TYPE decoded;
        I_DECODING(instruction,&decoded);

        if(decoded.funct3==0x0)
        {
            cpu->x[decoded.rd]=decoded.rs1+decoded.imm;
            printf("ADDI\n");
        }//ADDI

        else if(decoded.funct3==0x4)
        {
            cpu->x[decoded.rd]=decoded.rs1^decoded.imm;
            printf("XORI\n");
        }//XORI

        else if(decoded.funct3==0x6)
        {
            cpu->x[decoded.rd]=decoded.rs1|decoded.imm;
            printf("ORI\n");
        }//ORI

        else if(decoded.funct3==0x7)
        {
            cpu->x[decoded.rd]=decoded.rs1&decoded.imm;
            printf("ANDI\n");
        }//ANDI

        else if(decoded.funct3==0x1&&(decoded.imm>>4==0X00))
        {
            cpu->x[decoded.rd]=decoded.rs1<<(decoded.imm&0b1111);
            printf("SLLI\n");
        }//SLLI

        else if(decoded.funct3==0x5&&(decoded.imm>>4==0X00))
        {
            cpu->x[decoded.rd]=decoded.rs1>>(decoded.imm&0b1111);
            printf("srli\n");
        }//srli

        else if(decoded.funct3==0x5&&(decoded.imm>>4==0X20))
        {
            {
                        uint32_t pom=0b10000000000000000000000000000000;
                        if((instruction&pom)==1)
                        {
                                    for (size_t i = 0; i < decoded.rs1; i++)
                                     {
                                        /* code */
                                        pom=pom+(pom>>1);
                                    }
                            cpu->x[decoded.rd]=(decoded.imm&0b1111)|pom; 
                        }
                        else
                                    cpu->x[decoded.rd]=decoded.rs1>>(decoded.imm&0b1111);
         
            printf("srai\n");
        }//srai
        }
        else if(decoded.funct3==0x2)
        {
            cpu->x[decoded.rd]=(decoded.rs1 < decoded.imm) ? 1:0;

            printf("SLTI\n");
        }//SLTI


        else if(decoded.funct3==0x3)
        {
            cpu->x[decoded.rd]=(decoded.rs1 < decoded.imm) ? 1:0;

            printf("SLTIU\n");
        }//SLTIU  
    }
    else if(opcode==0b0000011)  // I_TYPE MEMORY INSTRUCTIONS
    {
        I_TYPE decoded;
        I_DECODING(instruction,&decoded);
        
        if(decoded.funct3==0x0)
        {
            cpu->x[decoded.rd]=memory->RAM[decoded.rs1+decoded.imm]&0b11111111;
            printf("LB\n");
        }//LB

        else if(decoded.funct3==0x1)
        {
            cpu->x[decoded.rd]=memory->RAM[decoded.rs1+decoded.imm]&0b1111111111111111;
            printf("LH\n");
        }//LH

        else if(decoded.funct3==0x2)
        {
            cpu->x[decoded.rd]=memory->RAM[decoded.rs1+decoded.imm];
            printf("LW\n");
        }//LW

        else if(decoded.funct3==0x4)
        {
            cpu->x[decoded.rd]=memory->RAM[decoded.rs1+decoded.imm]&0b11111111;
            printf("LBU\n");
        }//LBU

        else if(decoded.funct3==0x5)
        {
            cpu->x[decoded.rd]=memory->RAM[decoded.rs1+decoded.imm]&0b1111111111111111;
            printf("LHU\n");
        }//LHU
    }
    else if(opcode==0b0100011)  //S_TYPE
    {
        S_TYPE decoded;
        SorB_DECODING(instruction,&decoded);
        if(decoded.funct3==0x0)
        {
            memory->RAM[decoded.rs1+decoded.immL+(decoded.immH<<4)]=decoded.rs2&0b11111111;
            printf("SB\n");
        }//sb
        else if(decoded.funct3==0x1)
        {
            memory->RAM[decoded.rs1+decoded.immL+(decoded.immH<<4)]=decoded.rs2&0b1111111111111111;
            printf("SH\n");
        }//sH
       
       else if(decoded.funct3==0x2)
        {
            memory->RAM[decoded.rs1+decoded.immL+(decoded.immH<<4)]=decoded.rs2;
            printf("SW\n");
        }//sW        
    }
    if(opcode==0b1100011)  //B_TYPE
    {
        B_TYPE decoded;
        SorB_DECODING(instruction,&decoded);
        if (decoded.funct3==0x0&&(decoded.rs1==decoded.rs2))
        {
            cpu->pc+=decoded.immL+(decoded.immH<<4);
            printf("BEQ\n");
        }
        else if (decoded.funct3==0x1&&(decoded.rs1!=decoded.rs2))
        {
            cpu->pc+=decoded.immL+(decoded.immH<<4);
            printf("BNE\n");
        }
        else if (decoded.funct3==0x4&&(decoded.rs1<decoded.rs2))
        {
            cpu->pc+=decoded.immL+(decoded.immH<<4);
            printf("BLT\n");
        }

        else if (decoded.funct3==0x5&&(decoded.rs1>=decoded.rs2))
        {
            cpu->pc+=(decoded.immL+(decoded.immH<<4));
            printf("BGE\n");
        }

        else if (decoded.funct3==0x6&&(decoded.rs1<decoded.rs2))
        {
            cpu->pc+=(decoded.immL+(decoded.immH<<4));
            printf("BLTU\n");
        }

        else if (decoded.funct3==0x7&&(decoded.rs1>=decoded.rs2))
        {
            cpu->pc+=(decoded.immL+(decoded.immH<<4));
            printf("BEGU\n");
        }//BEGU
    }
    else if(opcode==0b1101111)
    {
        J_TYPE decoded; 
        UorJ_DECODING(instruction,&decoded);
        cpu->x[decoded.rd]=cpu->pc+4;
        cpu->pc+=decoded.imm;
        printf("JAL\n");
    }//JAL

    else if (opcode==0b1100111)
    {
        I_TYPE decoded;
        I_DECODING(instruction,&decoded);
        if(decoded.funct3==0x0)
        {
            cpu->x[decoded.rd]=cpu->pc+4;
            cpu->pc=decoded.rs1+decoded.imm;
            printf("JAIR");
        }// JAIR
    }
    
    else if (opcode==0b0010111) //I_TYPE
    {
        I_TYPE decoded;
        I_DECODING(instruction,&decoded);
        cpu->x[decoded.rd]=cpu->pc+(decoded.imm<<12);
        printf("AUIPC");
    }
    //BREAK
    else if(opcode==0b1110011) //ende
    {
        system("exit");
    }
    //incrementing pc
    cpu->pc++;
}
//start function
void init_mem(CPU *PROCESOR,MEM *memorija) // INIT REGS AND RAM
{

    for (size_t i = 0; i < 32; i++)
    {
        PROCESOR->x[i]=0;
        //printf("%d",PROCESOR->x[i]);        
    }
    PROCESOR->pc=START_ADDRES;
    memorija->RAM=(uint32_t*)calloc(RAM_SIZE,sizeof(uint32_t));
}

void prints(CPU procesor)
{
    #if debug==1
    printf("\n");
    for (size_t i = 1; i < 33; i++)
        {
            printf("x%zu=%d    ",i-1,procesor.x[i-1]);
            if(i%8==0)
                printf("\n\n");
        }
    #endif
}
int main()
{
    CPU procesor;
    MEM memorija;
    uint32_t instrukcija;
    J_TYPE pom;
    instrukcija=0b0110011;
    init_mem(&procesor,&memorija);
    //R_DECODING(instrukcija,&pom);
    size_t i=0;   
    while (1)
    {
        prints(procesor);
        printf("\n");
        execute(instruct[i++],&memorija,&procesor);
        getchar();
        prints(procesor);
        if(i==number_of_instructions)
            break;   
    }   
    //system("pause");
    return 0;
}
