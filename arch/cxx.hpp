#include "arch.hpp"
#include <string>
#include <sstream>
using std::string;
using std::to_string;
using std::stringstream;
class CxxArch : public Arch {
	string initialize() {
		return R"(#include <iostream>
int register[256];
int memory[2048];
int stack[1024];
int* stackptr = &stack[0];
void push(int val) {
	*stackptr++ = val;
}
void epush() {
	stackptr++;
}
int pop() {
	return *--stackptr;
}
void epop() {
	stackptr--;
}
// Port 0: ASCII
// Port 1: Number
void output(int port, int data) {
	if (port == 0) {
		std::cout << (char)data;
	}
	if (port == 1) {
		std::cout << data;
	}
}
int input(int port) {
	int ret = 0;
	if (port == 0) {
		char c;
		std::cin >> c;
		ret = (int)c;
	}
	if (port == 1) std::cin >> ret;
	return ret;
}
int main() {
)";
	}
	string finalize() {
		return R"(return 0;
}
)";
	}
	// Indent Everything And Add Jump Label
	string prefix() {
		stringstream ret;
		ret << "i" << instruction << ":\n\t";
		return ret.str();
	}
	string postfix() {
		return "\n";
	}
	string main(int inst) {	// Jump to main function
		stringstream ret;
		ret << "goto i" << inst << ";";
		return ret.str();
	}
	// Data Copy Instructions
	string copy_register_register(int reg1, int reg2) {
		stringstream ret;
		ret << "register[" << reg2 << "] = register[" << reg1 << "];";
		return ret.str();
	}
	string copy_register_memory(int reg, int mem) {
		stringstream ret;
		ret << "memory[" << reg << "] = register[" << reg << "];";
		return ret.str();
	}
	string copy_register_pointer(int reg, int ptr) {
		stringstream ret;
		ret << "memory[register[" << ptr << "]] = register[" << reg << "];";
		return ret.str();
	}
	string copy_register_port(int reg, int port) {
		stringstream ret;
		ret << "output(" << port << ", register[" << reg << "]);";
		return ret.str();
	}
	string copy_memory_register(int mem, int reg) {
		stringstream ret;
		ret << "register[" << reg << "] = memory[" << mem << "];";
		return ret.str();
	}
	string copy_pointer_register(int ptr, int reg) {
		stringstream ret;
		ret << "register[" << reg << "] = memory[register[" << ptr << "]];";
		return ret.str();
	}
	string copy_port_register(int port, int reg) {
		stringstream ret;
		ret << "register[" << reg << "] = input(" << port << ");";
		return ret.str();
	}
	// Stack Instructions
	string push_register(int reg) {
		stringstream ret;
		ret << "push(register[" << reg << "]);";
		return ret.str();
	}
	string pop_register(int reg) {
		stringstream ret;
		ret << "register[" << reg << "] = pop();";
		return ret.str();
	}
	string empty_push() {
		return "epush();";
	}
	string empty_pop() {
		return "epop();";
	}
	// Conditional Instructions
	string condition_if_zero(int reg) {
		stringstream ret;
		ret << "if (register[" << reg << "] == 0)";
		return ret.str();
	}
	string condition_not_zero(int reg) {
		stringstream ret;
		ret << "if (register[" << reg << "] != 0)";
		return ret.str();
	}
	// Overflow Not Supported
	// Math Instructions
	// Add
	string add_register_register(int reg1, int reg2, int destreg) {
		stringstream ret;
		ret << "register[" << destreg << "] = register[" << reg1 << "] + register[" << reg2 << "];";
		return ret.str();
	}
	string add_value_register(int value, int reg, int destreg) {
		stringstream ret;
		ret << "register[" << destreg << "] = " << value << " + register[" << reg << "];";
		return ret.str();
	}
	string add_register_value(int reg, int value, int destreg) {
		stringstream ret;
		ret << "register[" << destreg << "] = register[" << reg << "] + " << value << ";";
		return ret.str();
	}
	// Subtract
	string subtract_register_register(int reg1, int reg2, int destreg) {
		stringstream ret;
		ret << "register[" << destreg << "] = register[" << reg1 << "] - register[" << reg2 << "];";
		return ret.str();
	}
	string subtract_value_register(int value, int reg, int destreg) {
		stringstream ret;
		ret << "register[" << destreg << "] = " << value << " - register[" << reg << "];";
		return ret.str();
	}
	string subtract_register_value(int reg, int value, int destreg) {
		stringstream ret;
		ret << "register[" << destreg << "] = register[" << reg << "] - " << value << ";";
		return ret.str();
	}
	// XOR
	string xor_register_register(int reg1, int reg2, int destreg) {
		stringstream ret;
		ret << "register[" << destreg << "] = register[" << reg1 << "] ^ register[" << reg2 << "];";
		return ret.str();
	}
	string xor_value_register(int value, int reg, int destreg) {
		stringstream ret;
		ret << "register[" << destreg << "] = " << value << " ^ register[" << reg << "];";
		return ret.str();
	}
	string xor_register_value(int reg, int value, int destreg) {
		stringstream ret;
		ret << "register[" << destreg << "] = register[" << reg << "] ^ " << value << ";";
		return ret.str();
	}
	// Or
	string or_register_register(int reg1, int reg2, int destreg) {
		stringstream ret;
		ret << "register[" << destreg << "] = register[" << reg1 << "] | register[" << reg2 << "];";
		return ret.str();
	}
	string or_value_register(int value, int reg, int destreg) {
		stringstream ret;
		ret << "register[" << destreg << "] = " << value << " | register[" << reg << "];";
		return ret.str();
	}
	string or_register_value(int reg, int value, int destreg) {
		stringstream ret;
		ret << "register[" << destreg << "] = register[" << reg << "] | " << value << ";";
		return ret.str();
	}
	// And
	string and_register_register(int reg1, int reg2, int destreg) {
		stringstream ret;
		ret << "register[" << destreg << "] = register[" << reg1 << "] & register[" << reg2 << "];";
		return ret.str();
	}
	string and_value_register(int value, int reg, int destreg) {
		stringstream ret;
		ret << "register[" << destreg << "] = " << value << " & register[" << reg << "];";
		return ret.str();
	}
	string and_register_value(int reg, int value, int destreg) {
		stringstream ret;
		ret << "register[" << destreg << "] = register[" << reg << "] & " << value << ";";
		return ret.str();
	}
	// One Reg ALU
	string not_register(int reg) {
		stringstream ret;
		ret << "register[" << reg << "] = ~register[" << reg << "];";
		return ret.str();
	}
	string rightshift_register(int reg) {
		stringstream ret;
		ret << "register[" << reg << "] = register[" << reg << "] << 1;";
		return ret.str();
	}
	string leftshift_register(int reg) {
		stringstream ret;
		ret << "register[" << reg << "] = register[" << reg << "] >> 1;";
		return ret.str();
	}
};
