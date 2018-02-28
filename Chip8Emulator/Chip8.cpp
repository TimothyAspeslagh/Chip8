#include "Chip8.h"
#include <string>
#include <iostream>
#include <fstream>
#include <climits>
#include <bitset>
#include <vector>
#include <memory>

Chip8::Chip8()
{
}


Chip8::~Chip8()
{
}

void Chip8::Initialise()
{
	//default states
	m_ProgramCounter = 0x200;
	m_Opcode = 0;
	m_IndexRegister = 0;
	m_StackPointer = 0;

	Clear_Screen_No_Instruction();
	//clear stack
	for (size_t i = 0; i < 16; i++)
	{
		m_Stack[i] = 0;
	}

	//clear registers V0-Vf
	for (size_t i = 0; i < 16; i++)
	{
		m_V[i] = 0;
	}
	//clear memory
	for (size_t i = 0; i < 4096; i++)
	{
		m_Memory[i] = 0;
	}
	//load fontset
	for (size_t i = 0; i < 80; ++i)
	{
		m_Memory[i] = m_FontSet[i];
	}
	//Reset timers
	m_DelayTimer = 0;
	m_SoundTimer = 0;

	for (size_t i = 0; i < m_BufferSize; ++i)
	{
		m_Memory[i + 512] = m_Buffer[i]; //What is buffer? TODO
	}
	//TODO
}

bool Chip8::LoadGame(std::string game)
{
	std::ifstream myFile;
	// Open file
	myFile.open("./Resources/merlin", std::ios::in | std::ios::ate);

	// Check file size
	const int size = myFile.tellg();
	std::cout << "Filesize: " <<size << std::endl;

	if(size == 0)
	{
		std::cout << "Failed to read ROM!" << std::endl;
		return false;
	}
	myFile.seekg(0);
	// Allocate memory to contain the whole file
	auto buf_ptr = std::make_unique<char[]>(size);

	// Copy the file into the buffer
	myFile.read((buf_ptr.get()), size);

	// Copy buffer to Chip8 memory
	for(int i = 0; i < size; ++i)
	{
		m_Memory[i + 512] = buf_ptr[i];
	}

	// Close file, free buffer
	myFile.close();
	buf_ptr.release();
}

void Chip8::EmulateCycle()
{

	//Fetch OpCode
	//the opcode is two bytes long 
	//so we need to read at two consecutive locations starting from the program counter
	auto prev = m_Opcode;
	m_Opcode = m_Memory[m_ProgramCounter] << 8 | m_Memory[m_ProgramCounter + 1];
	//Decode OpCode
	CallInstructionFromOpcode(prev);
	//Update Timers
	if (m_DelayTimer > 0)
	{
		--m_DelayTimer;
	}
	
	if (m_SoundTimer > 0)
	{
		if (m_SoundTimer == 1)
		{
			SoundBeep();
		}

		--m_SoundTimer;
	}
}

void Chip8::CallInstructionFromOpcode(unsigned short prev)
{
	switch (m_Opcode & 0xF000)
	{
	case 0x0000:
		switch (m_Opcode & 0x000F)
		{
		case 0x000:
			Clear_Screen(); //OpCode is 0x00E0
			break;
		case 0x000E: // OpCode is 0x00EE
			Return_From_Subroutine();
			break;
		default:
			std::cout << "Invalid Opcode: " << m_Opcode << std::endl;
			std::cout << "Previous " << prev << std::endl;
			break;
		}
		break;
	case 0x1000:
		// code 0x1NNN Jumps to address NNN
		Jump_To_Address();
		break;
	case 0x2000:
		// code 0x2NNN calls subroutine at NNN
		Call_Subroutine();
		break;
	case 0x3000:
		// code 0x3XNN skips the next instruction if VX == NN
		Skip_Instruction_3();
		break;
	case 0x4000:
		// code 0x4XNN skips the next instruction if VX != NN
		Skip_Instruction_4();
		break;
	case 0x5000:
		// code 0x5XY0 skips the next instruction if VX == VY
		Skip_Instruction_5();
		break;
	case 0x6000:
		// sets VX to NN
		Set_VX_To_NN();
		break;
	case 0x7000:
		// adds NN to VX
		Add_NN_To_VX();
		break;
	case 0x8000:
		//Break up into deeper switch
		switch (m_Opcode & 0x000F)
		{
		case 0x0000:
			Set_VX_To_VY();
			break;
		case 0x0001:
			Set_VX_To_VX_Or_VY();
			break;
		case 0x0002:
			Set_VX_To_VX_And_VY();
			break;
		case 0x0003:
			Set_VX_To_VX_Xor_VY();
			break;
		case 0x0004:
			Add_VY_To_VX_VFCARRY();
			break;
		case 0x0005:
			Substract_VY_From_VX_VFBORROW();
			break;
		case 0x0006:
			Shift_VX_Right_One_VFLEASTSIGNIFICANT();
			break;
		case 0x0007:
			Set_VX_To_VY_Minus_VX_VFBORROW();
			break;
		case 0x000E:
			Shift_VX_Left_One_VFMOSTSIGNIFICANT();
			break;
		default:
			std::cout << "Invalid Opcode: " << m_Opcode << std::endl;
			std::cout << "Previous " << prev << std::endl;
			break;
		}
		break;
	case 0x9000:
		//code 0x9XY0 skips the next instruction if VX != VY
		Skip_Instruction_If_VX_UNEQUAL_VY();
		break;
	case 0xA000:
		// code 0xANNN sets m_I to the address NNN
		Set_I_To_0xANNN();
		break;
	case 0xB000:
		// code 0xBNNN jumps to address NNN plus V0
		Jump_To_Address_Plus_V0();
		break;
	case 0xC000:
		//code 0xCXNN sets VX to the result of a bitwise & operation on random number and NN
		Set_VX_To_Random_Bitwise_NN();
		break;
	case 0xD000:
		//code 0xDXYN draws sprites starting at pos VX, VY;
		//N is the number of 8-bit rows that need to be drawn
		//all drawing is XOR
		Sprite_Operation();
		break;

	case 0xE000:
		switch (m_Opcode & 0x00FF)
		{
		case 0x009E:
			Skip_Instruction_If_Key_Pressed();
			break;
		case 0x00A1:
			Skip_Instruction_If_Key_Not_Pressed();
			break;
		default:
			std::cout << "Invalid Opcode: " << m_Opcode << std::endl;
			std::cout << "Previous " << prev << std::endl;
			break;

		}
		break;

	case 0xF000:
		//break up into deeper switch
		switch (m_Opcode & 0x00FF)
		{
		case 0x0007:
			Set_VX_To_Delay_Timer();
			break;
		case 0x000A:
			Await_Key_Press();
			break;
		case 0x0015:
			Set_Delay_Timer_To_VX();
			break;
		case 0x0018:
			Set_Sound_Timer_To_VX();
			break;
		case 0x001E:
			Add_VX_To_I();
			break;
		case 0x0029:
			Set_I_To_Sprite_VX();
			break;
		case 0x0033:
			Store_Binary_Of_VX();
			break;
		case 0x0055:
			Store_V0_To_VX_In_Memory_At_I();
			break;
		case 0x0065:
			Fill_V0_To_VX_From_Memory_At_I();
			break;
		default:
			std::cout << "Invalid Opcode: " << m_Opcode << std::endl;
			std::cout << "Previous " << prev << std::endl;
			break;
		}
		break;

	default:
		std::cout << "Invalid Opcode: " << m_Opcode << std::endl;
		std::cout << "Previous " << prev << std::endl;
		break;
	}
}


void Chip8::SetKeyState(size_t keyIndex, bool state)
{
	if (state)
	{
		m_Key[keyIndex] = 1;
	}
	else
	{
		m_Key[keyIndex] = 0;
	}
}

bool Chip8::GetDrawflagState()
{
	return m_DrawFlag;
}

void Chip8::SetDrawflagState(bool flag)
{
	m_DrawFlag = flag;
}

void Chip8::Clear_Screen()
{
	for (size_t i = 0; i < (64*32); i++)
	{
		m_Gfx[i] = 0;

	}
	m_DrawFlag = true;
	m_ProgramCounter += 2;
}

void Chip8::Clear_Screen_No_Instruction()
{
	for (size_t i = 0; i < (64 * 32); i++)
	{
		m_Gfx[i] = 0;

	}
	m_DrawFlag = true;

}

void Chip8::Return_From_Subroutine()
{
	--m_StackPointer;
	m_ProgramCounter = m_Stack[m_StackPointer];
	m_ProgramCounter += 2;
}

void Chip8::Jump_To_Address()
{
	// code 0x1NNN
	m_ProgramCounter = m_Opcode & 0x0FFF;
}

void Chip8::Call_Subroutine()
{
	// code 0x2NNN
	m_Stack[m_StackPointer] = m_ProgramCounter;
	++m_StackPointer;
	m_ProgramCounter = m_Opcode & 0x0FFF;
}

void Chip8::Skip_Instruction_3()
{
	// code 0x3XNN
	//if m_V[X] == NN
	//Skip();
	if (m_V[(m_Opcode & 0x0F00) >> 8] == (m_Opcode & 0x00FF))
		Skip();
	else
		m_ProgramCounter += 2;
}

void Chip8::Skip_Instruction_4()
{
	// code 0x4XNN
	//if m_V[X] != NN
	//Skip();
	if (m_V[(m_Opcode & 0x0F00) >> 8] != (m_Opcode & 0x00FF))
		Skip();
	else
		m_ProgramCounter += 2;
}

void Chip8::Skip_Instruction_5()
{
	// code 0x5XY0
	//if m_V[X] == m_V[Y]
	//Skip();

	if (m_V[(m_Opcode & 0x0F00) >> 8] == m_V[(m_Opcode & 0x0F0) >> 4])
		Skip();
	else
		m_ProgramCounter += 2;
}

void Chip8::Set_VX_To_NN()
{
	// code 0x6XNN
	//m_V[X] = NN;
	m_V[(m_Opcode & 0x0F00) >> 8] = m_Opcode & 0x00FF;
	m_ProgramCounter += 2;
}

void Chip8::Add_NN_To_VX()
{
	// code 0x7XNN
	//m_V[X] += NN;
	m_V[(m_Opcode & 0x0F00) >> 8] += m_Opcode & 0x00FF;
	m_ProgramCounter += 2;
}

void Chip8::Set_VX_To_VY()
{
	// code 0x8XY0
	//m_V[X] = m_V[Y]
	m_V[(m_Opcode & 0x0F00) >> 8] = m_V[(m_Opcode & 0x00F0) >>4];
	m_ProgramCounter += 2;
}

void Chip8::Set_VX_To_VX_Or_VY()
{
	// code 0x8XY1
	// bitwise OR operation
	//m_V[X] = m_V[X] | m_V[Y]
	m_V[(m_Opcode & 0x0F00) >> 8] = m_V[(m_Opcode & 0x0F00) >> 8] | m_V[(m_Opcode & 0x00F0) >> 4];
	m_ProgramCounter += 2;
}

void Chip8::Set_VX_To_VX_And_VY()
{
	// code 0x8XY2
	// bitwise AND operation
	//m_V[X] = m_V[X] & m_V[Y]
	m_V[(m_Opcode & 0x0F00) >> 8] = m_V[(m_Opcode & 0x0F00) >> 8] & m_V[(m_Opcode & 0x00F0) >>4];
	m_ProgramCounter += 2;
}

void Chip8::Set_VX_To_VX_Xor_VY()
{
	// code 0x8XY3
	// bitwise XOR operation
	//m_V[X] = m_V[X] ^ m_V[Y]
	m_V[(m_Opcode & 0x0F00) >> 8] = m_V[(m_Opcode & 0x0F00) >> 8] ^ m_V[(m_Opcode & 0x00F0) >> 4];
	m_ProgramCounter += 2;
}

void Chip8::Add_VY_To_VX_VFCARRY()
{

	// code 0x8XY4
	// m_V[X] += m_V[Y]
	// if carry -> m_V[0xF] = 1
	// else m_V[0xF] = 
	// carry is enabled when values exceed SHORT_MAX
	bool carry = false;
	if (m_V[(m_Opcode & 0x0F00) >> 8] > SHRT_MAX - m_V[(m_Opcode & 0x00F0) >> 4])
		carry = true;

	m_V[(m_Opcode & 0x0F00) >> 8] += m_V[(m_Opcode & 0x00F0) >> 4];

	if (carry)
		m_V[0xF] = 1;
	else
		m_V[0xF] = 0;

	m_ProgramCounter += 2;
}

void Chip8::Substract_VY_From_VX_VFBORROW()
{
	// code 0x8XY5
	// m_V[X] -= m_V[Y]
	// if borrow
	//m_V[0xF] = 0;
	// else m_V[0xF] = 1;
	// borrow is enabled when m_V[X] < m_V[y]
	bool borrow = false;

	if (m_V[(m_Opcode & 0x0F00) >> 8] < m_V[(m_Opcode & 0x00F0) >> 4])
		borrow = true;

	m_V[(m_Opcode & 0x0F00) >> 8] -= m_V[(m_Opcode & 0x00F0) >> 4];
	
	if (borrow)
		m_V[0xF] = 0;
	else
		m_V[0xF] = 1;

	m_ProgramCounter += 2;
}

void Chip8::Shift_VX_Right_One_VFLEASTSIGNIFICANT()
{
	//code 0x8XY6
	m_V[0xF] = m_V[(m_Opcode & 0x0F00) >>8] & 0x1;
	m_V[(m_Opcode & 0x0F00 >> 8)] >>= 1; //m_V[(m_Opcode & 0x0F00 >> 8)] >> 4;
	m_ProgramCounter += 2;
}

void Chip8::Set_VX_To_VY_Minus_VX_VFBORROW()
{
	// code 0x8XY7
	// m_V[X] -= m_V[Y]
	// if borrow
	//m_V[0xF] = 0;
	// else m_V[0xF] = 1;
	// borrow is enabled when m_V[X] < m_V[y]
	bool borrow = false;

	if (m_V[(m_Opcode & 0x00F0) >> 4] < m_V[(m_Opcode & 0x0F00) >> 8])
		borrow = true;

	m_V[(m_Opcode & 0x0F00) >> 8] = m_V[(m_Opcode & 0x00F0) >> 4] - m_V[(m_Opcode & 0x0F00) >> 8];
	
	if (borrow)
		m_V[0xF] = 0;
	else
		m_V[0xF] = 1;

	m_ProgramCounter += 2;
}

void Chip8::Shift_VX_Left_One_VFMOSTSIGNIFICANT()
{
	//code 0x8XYE
	m_V[0xF] = m_V[(m_Opcode & 0x0F00) >>8] >> 7;
	m_V[(m_Opcode & 0x0F00 >> 8)] <<= 1;//m_V[(m_Opcode & 0x0F00 >> 8)] << 4;
	m_ProgramCounter += 2;
}

void Chip8::Skip_Instruction_If_VX_UNEQUAL_VY()
{
	// code 0x9XY0
	// if m_V[X] != m_V[Y]
	// Skip()
	if (m_V[(m_Opcode & 0x0F00) >> 8] != m_V[(m_Opcode & 0x00F0) >> 4])
		Skip();
	else
		m_ProgramCounter += 2;
}

void Chip8::Set_I_To_0xANNN()
{
	// code 0xANNN
	m_IndexRegister = (m_Opcode & 0x0FFF);

	m_ProgramCounter += 2;
}

void Chip8::Jump_To_Address_Plus_V0()
{
	// code 0xBNNN
	m_ProgramCounter = (m_Opcode & 0x0FFF) + m_V[0];
}

void Chip8::Set_VX_To_Random_Bitwise_NN()
{
	// code 0xCXNN
	// m_V[X] = random & NN;
	unsigned short random = rand() % (0xFF + 1); 
	m_V[(m_Opcode & 0x0F00) >> 8] = (random & (m_Opcode & 0x00FF));

	m_ProgramCounter += 2;
}

void Chip8::Sprite_Operation()
{
	// code 0xDXYN
	// draw a sprite at coordinates {m_V[X], m_V[Y]} that has a width of 8 pixels and a height
	// of N pixels. Each 8 pixel row is read as bit-coded starting from memory location m_IndexRegister
	// m_IndexRegister remains unchanged during this instruction. 
	// if any pixels are flipped from set to unset
	// m_V[F] = 1
	// else
	// m_V[F] = 0

	bool isAPixelFlippedToUnset = false;
	int xCoordinate = m_V[(m_Opcode & 0x0F00) >> 8];
	int yCoordinate = m_V[(m_Opcode & 0x00F0) >> 4];
	unsigned const int rows = int(m_Opcode & 0x000F); //N rows
	unsigned const int collumns = 8; // 8 collumns
	
	for (size_t i = 0; i < rows; i++)
	{
		std::bitset<collumns> rowData = m_Memory[m_IndexRegister + i];
		for (size_t e = 0; e < collumns; e++)
		{
			int screenPos = xCoordinate + ( 8 - (e + 1)) + ((yCoordinate + i) * 64);
			unsigned char bitInfo = rowData[e];
			bool oldPixelIsOne = (m_Gfx[screenPos] == 1);
	
			// 1. check if the bit gets flipped, set corresponding bool
			// 2. Perform XOR operation on m_Gfx
			m_Gfx[screenPos] ^= bitInfo;
	
			if (m_Gfx[screenPos] == 0 && oldPixelIsOne)
			{
				isAPixelFlippedToUnset = true;
			}
	
		}
	}
	
	if (isAPixelFlippedToUnset) // this is done so that a game can check for collision
	{
		m_V[0xF] = 1;
	}
	else
	{
		m_V[0xF] = 0;
	}
	m_DrawFlag = true;
	
	m_ProgramCounter += 2;


}

void Chip8::Skip_Instruction_If_Key_Pressed()
{
	// code 0xEX9E
	// if m_Key[m_V[X]] == pressed 
	//skip()
	if (m_Key[m_V[(m_Opcode & 0x0F00) >> 8]] == 1)
		Skip();
	else
		m_ProgramCounter += 2;
}

void Chip8::Skip_Instruction_If_Key_Not_Pressed()
{
	// code 0xEXA1
	// if m_Key[m_V[X]] != pressed 
	//skip()
	if (m_Key[m_V[(m_Opcode & 0x0F00) >> 8]] != 1)
		Skip();
	else
		m_ProgramCounter += 2;
}

void Chip8::Set_VX_To_Delay_Timer()
{
	// code 0xFX07
	// m_V[x] = m_DelayTimer;
	m_V[(m_Opcode & 0x0F00) >> 8] = m_DelayTimer;

	m_ProgramCounter += 2;
}

void Chip8::Await_Key_Press()
{
	// code 0xFX0A
	//Wait for a key press, store the value of the key in Vx.
	//All execution stops until a key is pressed, then the value of that key is stored in Vx.
	bool press = false;

	for (size_t i = 0; i < 16; ++i)
	{
		if (m_Key[i] == 1)
		{
			m_V[(m_Opcode & 0x0F00) >> 8] = (unsigned char)i;
			press = true;
		}
	}
	//if no press is detected, the programcounter is not updated
	//because it is not updated, the next opcode will be the same
	//so this same instruction will be executed
	if (press == true)
	{
		m_ProgramCounter += 2;
	}
}

void Chip8::Set_Delay_Timer_To_VX()
{
	// code 0xFX15
	// m_DelayTimer = m_V[X]
	m_DelayTimer = m_V[(m_Opcode & 0x0F00) >>8];

	m_ProgramCounter += 2;
}

void Chip8::Set_Sound_Timer_To_VX()
{
	// code 0xFX18
	// m_SoundTimer = m_V[X]
	m_SoundTimer = m_V[(m_Opcode & 0x0F00) >> 8];

	m_ProgramCounter += 2;
}

void Chip8::Add_VX_To_I()
{
	// code 0xFX1E
	// m_IndexRegister += m_V[X];
	if (m_IndexRegister + m_V[(m_Opcode & 0x0F00) >> 8] > 0xFFF)
	{
		m_V[0xF] = 1;
	}
	else
	{
		m_V[0xF] = 0;
	}

	m_IndexRegister += m_V[(m_Opcode & 0x0F00) >> 8];

	m_ProgramCounter += 2;
}

void Chip8::Set_I_To_Sprite_VX()
{
	//code 0xFX29
	m_IndexRegister = m_V[(m_Opcode & 0x0F00) >> 8] * 0x5; 
	m_ProgramCounter += 2;
}

void Chip8::Store_Binary_Of_VX()
{
	//code 0xFX33
	// take decimal value of m_V[X] and place hundreds in memory[i]
	// tens in memory[i+1]
	// ones in memory[i+2]

	m_Memory[m_IndexRegister] = (m_V[(m_Opcode & 0x0F00) >> 8] / 100);
	m_Memory[m_IndexRegister + 1] = (m_V[(m_Opcode & 0x0F00) >> 8] /10) % 10;
	m_Memory[m_IndexRegister + 2] = (m_V[(m_Opcode & 0x0F00) >> 8] % 100) % 10;

	m_ProgramCounter += 2;
}

void Chip8::Store_V0_To_VX_In_Memory_At_I()
{
	// code 0xFX55
	for (size_t i = 0; i <=  (unsigned)((m_Opcode & 0x0F00)>>8); i++)
	{
		m_Memory[m_IndexRegister + i] = m_V[i];
	}
	m_IndexRegister += ((m_Opcode & 0x0F00) >> 8) + 1;
	m_ProgramCounter += 2;
}

void Chip8::Fill_V0_To_VX_From_Memory_At_I()
{
	// code 0xFX65
	for (size_t i = 0; i <=  (m_Opcode & 0x0F00) >>8; i++)
	{
		m_V[i] = m_Memory[m_IndexRegister + i];
	}
	m_IndexRegister += ((m_Opcode & 0x0F00) >> 8) + 1;
	m_ProgramCounter += 2;
}

void Chip8::Skip()
{
	m_ProgramCounter += 4;
}

void Chip8::SoundBeep()
{
	std::cout << "Bleep" << std::endl;
}

unsigned char Chip8::GetScreen(size_t i)
{
	return m_Gfx[i];
}
