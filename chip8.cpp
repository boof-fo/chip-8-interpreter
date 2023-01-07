#include <iostream>

#define DISPLAY_W 64
#define DISPLAY_H 32

//MEMORY
uint8_t memory[0x1000] = {0};
//interprete usa i primi 512 byte, quindi i programmi inziano dalla location 0x200 inclusa (alcuni da 0x600)
////gli ultimi 256 byte (0xF00-0xFFF) sono usati per il display
////i 96 byte prima del display (0xEA0-0xEFF) sono riservati per il call stack

//REGISTERS
uint8_t V[16], delay_timer, sound_timer, stack_pointer;
//V: vanno da V0 a VF; VF è usato come flag per alcune istruzioni
uint16_t I, PC, stack[16];
//I: contiene indirizzi a memoria, quindi solo gli ultimi 12 bit sono usati
//stack: LIFO

//DISPLAY
bool display[DISPLAY_W][DISPLAY_H];

uint8_t default_font[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
//to put in memory from 050 to 09F


int cycle(){
	uint16_t opcode, instruction;
	//FETCH
	instruction = memory[PC] << 8 | memory[PC + 1];


	//DECODE
	uint16_t x, y, n, nn, nnn;

	opcode = instruction & 0xF000;
	x = (instruction >> 8) & 0x000F;//the lower 4 bits of the high byte of the instruction
	y = (instruction >> 4) & 0x000F;//the upper 4 bits of the low byte of the instruction
	n = instruction & 0x000F;//the lowest 4 bits of the instruction
	nn = instruction & 0x00FF;//the lowest 8 bits of the instruction
	nnn = instruction & 0x0FFF;//lowest 12 bits of the instruction


	//EXECUTE
	switch (opcode){
		case 0x0000:
		//manages 0x0NNN opcodes
			if(nn == 0x00E0){ //clear screen
				for(int i = 0; i <= DISPLAY_W; i++)
					for(int j = 0; j <= DISPLAY_H; j++){
						display[i][j] = 0;
						PC += 2;
					}
			} else
			if(nn == 0x00EE){ //return from subroutine
				stack_pointer -= 1;
				PC = stack[stack_pointer];
			}
		break;
		
		case 0x1000: //0x1NNN JUMP to NNN
			PC = nnn;
		break;

		case 0x2000: //call subroutine at location NNN
			stack_pointer += 1;
			stack[stack_pointer] = PC;
			PC = nnn;
		break;

		case 0x3000: //3XNN skip if Vx == nn
			if(V[x] == nn)
				PC += 2;
		break;

		case 0x4000: //4XNN skip if Vx != nn
			if(V[x] != nn)
				PC += 2;
		break;

		case 0x5000: //5XY0 skip if Vx == Vy
			if(V[x] == V[y])
				PC += 2;
		break;

		case 0x9000: //9XY0 skip if Vx != Vy
			if(V[x] != V[y])
				PC += 2;
		break;

		case 0x6000: //6XNN set VX to NN
			V[x] = nn;
		break;

		case 0x7000: //7XNN add NN to VX (doesn't affect carry flag)
			V[x] += nn;
		break;

		case 0x8000: //logical and arithmetic instructions
		switch(n)
			case 0x0://set Vx to Vy
				V[x] = V[y];
			break;
			case 0x1://Vx = OR of Vx and Vy
				V[x] |= V[y];
			break;
			case 0x2://Vx = AND of Vx and Vy
				V[x] &= V[y];
			break;
			case 0x3://Vx = XOR of Vx and Vy
				V[x] ^= V[y];
			break;
			case 0x4://add NN to VX (affects carry flag (VF))
				
			break;
		break;
	}
}









