#include <iostream>

#define DISPLAY_W 64
#define DISPLAY_H 32
#define KEYS_N 16

//some instruction have different behaviours in the CHIP-8 and CHIP-48 languages. this flag sets which behaviour to use.
#define CHIP_48 0 


//MEMORY
uint8_t memory[0x1000] = {0};
//programs start @ location 0x200 included (some from 0x600)
//last 256 bytes (0xF00-0xFFF) are for the display
//96 bytes before display (0xEA0-0xEFF) are reserved for the call stack

//INPUT
uint8_t keys[KEYS_N];

//REGISTERS
uint8_t V[0xF], delay_timer, sound_timer, stack_pointer;
//V: V0 to VF; VF is a flag for some instructions
uint16_t I, PC, stack[16];
//I: contains memory addresses, only last 12 bit are used
//stack: LIFO
bool draw;

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
//to put in memory from 0x050 to 0x09F


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
		switch(nn){
			case 0x00E0:
				for(int i = 0; i <= DISPLAY_W; i++)
					for(int j = 0; j <= DISPLAY_H; j++)
						display[i][j] = 0;
				PC += 2;
			break;
			case 0x00EE:
				stack_pointer -= 1;
				PC = stack[stack_pointer];
			break;
		}
		break;
		
		case 0x1000:
			PC = nnn;
		break;

		case 0x2000:
			stack_pointer += 1;
			stack[stack_pointer] = PC;
			PC = nnn;
		break;

		case 0x3000:
			if(V[x] == nn)
				PC += 2;
		break;

		case 0x4000:
			if(V[x] != nn)
				PC += 2;
		break;

		case 0x5000:
			if(V[x] == V[y])
				PC += 2;
		break;

		case 0x9000:
			if(V[x] != V[y])
				PC += 2;
		break;

		case 0x6000:
			V[x] = nn;
		break;

		case 0x7000:
			V[x] += nn;
		break;
//FIXME: program counter increment after instruction execution
		case 0x8000: //8XYN logical and arithmetic instructions
		switch(n){
			case 0x0:
				V[x] = V[y];
			break;
			case 0x1:
				V[x] |= V[y];
			break;
			case 0x2:
				V[x] &= V[y];
			break;
			case 0x3:
				V[x] ^= V[y];
			break;
			case 0x4: //Vx = Vx + Vy (affects carry flag (VF))
				V[0xF] = (V[x] + V[y] > 255) ? 1 : 0;
				V[x] += V[y];
			break;
			case 0x5: //Vx = Vx - Vy (affects carry flag (VF))
				V[0xF] = (V[x] > V[y]) ? 1 : 0;
				V[x] -= V[y];
			break;
			case 0x6: //Vx = Vy; Vx >> 1; Vf = shifted bit;
				if(!CHIP_48)//AMBIGUOUS INSTRUCTION!!!
					V[x] = V[y];
				V[0xF] = V[x] & 0x0001;
				V[x] = V[x] >> 1;
			break;
			case 0x7: //Vx = Vy - Vx (affects carry flag (VF))
				V[0xF] = (V[y] > V[x]) ? 1 : 0;
				V[x] = V[y] - V[x];
			break;
			case 0xE://Vx = Vy; Vx << 1; Vf = shifted bit
				if(!CHIP_48)//AMBIGUOUS INSTRUCTION!!!
					V[x] = V[y];
				V[0xF] = V[x] & 0x0001;
				V[x] = V[x] << 1;
			break;
		}
		break;

		case 0xA000:
			I = nnn;
			PC += 2;
		break;

		case 0xB000:
			if(!CHIP_48)//AMBIGUOUS INSTRUCTION!!!
				PC = nnn + V[0];
			else
				PC = nnn + V[x];
		break;

		case 0xC000:
			V[x] = (rand() % 256) & nn;
		break;


		case 0xD000:
			uint8_t row = V[x] % DISPLAY_W;
			uint8_t col = V[y] % DISPLAY_H;
			uint8_t byte = 0;
			V[0xF] = 0;

			for(uint8_t row_i = 0; row_i < n; row_i++){
				byte = memory[I + row_i];

				for(uint8_t bit_i = 0; bit_i < 8; bit_i++){
					if((display[row][col]) && ((byte >> bit_i) & 0x1)){
						V[0xF] = 1;
						display[row][col] ^= ((byte >> bit_i) & 0x1;
					} 
				}
				PC += 2;
				draw = true;
				break;
			}
		
		case 0xE000:
			switch(nn){
				case 0x9E:
					if(keys[V[x]])
						PC += 4;
					else
						PC += 2;
					break;
				case(0xA1):
					if(!keys[V[x]])
						PC += 4;
					else
						PC += 2;
					break;
			}

		case 0xF000:
			switch(nn){
				case 0x07:
					V[x] = delay_timer;
					PC += 2;
					break;
				case 0x0A:
					bool quit = false;
					while(quit == false){
						for(int i = 0; i < KEYS_N; i++){
							if(keys[i])
								V[x] = i;
							quit = true;
						}
					}
					PC += 2;
					break;
				case 0x15:
					delay_timer = V[x];
					PC += 2;
					break;
				case 0x18:
					sound_timer = V[x];
					PC += 2;
					break;
				case 0x1E:
					I += V[x];
					if(I + V[x] > 0xfff)
						V[0xF] = 1;
					else
						V[0xF] = 0;
					PC += 2;
					break;
				case 0x29:
					I = V[x] * 5; //5 è la lunghezza di un carattere
					PC += 2;
					break;
				case 0x33:
					memory[I] = V[x] / 100;
					memory[I + 1] = (V[x] % 100) / 10;
					memory[I + 2] = V[x] % 10;
					PC += 2;
					break;
				case 0x55:
					for(int i = 0; i <= x; i++)
						memory[I + i] = V[i];
					if(!CHIP_48)//AMBIGUOUS INSTRUCTION!!!
						I += x + 1;
					PC += 2;
					break;
				case 0x65:
					for(int i = 0; i <= x; i++)
						V[i] = memory[I + i];
					if(!CHIP_48)//AMBIGUOUS INSTRUCTION!!!
						I += x + 1;
					PC += 2;
					break;
			}
}




		//			if(!CHIP_48)//AMBIGUOUS INSTRUCTION!!!