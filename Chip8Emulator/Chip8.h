#pragma once

#include <string>

class Chip8
{
public:
	Chip8();
	~Chip8();
	void Initialise();
	bool LoadGame(std::string game);
	void CallInstructionFromOpcode(unsigned short prev);
	void EmulateCycle();
	void SetKeyState(size_t keyIndex, bool state);
	bool GetDrawflagState();
	void SetDrawflagState(bool flag);
	unsigned char GetScreen(size_t i);


private:
	//Opcode is the current instruction
	unsigned short m_Opcode;
	// Drawflag, checks to see if screen has to update
	bool m_DrawFlag = false;

	//Memory used for data storage
	// 0x000 <-> 0x1FF  | chip 8 interpreter, contains font set
	// 0x050 <-> 0x0A0  | Used for the built in font set (4x5)
	// 0x200 <-> 0xFFF  | Read Only Memory and Random Access Memory
	unsigned char m_Memory[4096];
	
	//FontSet, credit: multigesture.net
	unsigned char m_FontSet[80]=
	{
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
	
	//The CPU registers
	unsigned char m_V[16];

	//index register
	unsigned short m_IndexRegister;
	//program counter
	unsigned short m_ProgramCounter;
	
	//array of screen size that holds pixel state
	unsigned char m_Gfx[64 * 32];

	//Counts to zero when var > 0;
	unsigned char m_DelayTimer;
	unsigned char m_SoundTimer;

	//stack keeps location when performing a jump (see assembly interrupts)
	unsigned short m_Stack[16];
	unsigned short m_StackPointer;

	//HEX basd keypad 
	// 0x0 <-> 0xF
	unsigned char m_Key[16];


	size_t m_BufferSize = 10;
	unsigned char m_Buffer[10];
private:
	void SoundBeep();
	void Skip(); //skips next instruction

	//OPCODE INSTRUCTIONS
	void Clear_Screen();
	void Clear_Screen_No_Instruction();
	void Return_From_Subroutine();
	void Jump_To_Address();
	void Call_Subroutine();
	void Skip_Instruction_3();
	void Skip_Instruction_4();
	void Skip_Instruction_5();
	void Set_VX_To_NN();
	void Add_NN_To_VX();
	void Set_VX_To_VY();
	void Set_VX_To_VX_Or_VY();
	void Set_VX_To_VX_And_VY();
	void Set_VX_To_VX_Xor_VY();
	void Add_VY_To_VX_VFCARRY();
	void Substract_VY_From_VX_VFBORROW();
	void Shift_VX_Right_One_VFLEASTSIGNIFICANT();
	void Set_VX_To_VY_Minus_VX_VFBORROW();
	void Shift_VX_Left_One_VFMOSTSIGNIFICANT();
	void Skip_Instruction_If_VX_UNEQUAL_VY();
	void Set_I_To_0xANNN();
	void Jump_To_Address_Plus_V0();
	void Set_VX_To_Random_Bitwise_NN();
	void Sprite_Operation();
	void Skip_Instruction_If_Key_Pressed();
	void Skip_Instruction_If_Key_Not_Pressed();
	void Set_VX_To_Delay_Timer();
	void Await_Key_Press();
	void Set_Delay_Timer_To_VX();
	void Set_Sound_Timer_To_VX();
	void Add_VX_To_I();
	void Set_I_To_Sprite_VX();
	void Store_Binary_Of_VX();
	void Store_V0_To_VX_In_Memory_At_I();
	void Fill_V0_To_VX_From_Memory_At_I();
};

