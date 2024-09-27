#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstdint>

// ArgH
#include "arg/argh.h"

// RapidXML
#include "xml/rapidxml.hpp"
#include "xml/rapidxml_print.hpp"

// Architectures
#include "arch/main.hpp"

// Values
int getval(std::string str) {
	return (int)std::stoi(str);
}

// Instructions
enum inst {
	COPY,
	PUSH,
	POP,
	ADD,
	SUBTRACT,
	XOR,
	OR,
	AND,
	NOT,
	RIGHTSHIFT,
	LEFTSHIFT,
};

inst getinst(std::string str) {
	if (str == "copy") return COPY;
	if (str == "push") return PUSH;
	if (str == "pop") return POP;
	if (str == "add") return ADD;
	if (str == "subtract") return SUBTRACT;
	if (str == "xor") return XOR;
	if (str == "or") return OR;
	if (str == "and") return AND;
	if (str == "not") return NOT;
	if (str == "rightshift") return RIGHTSHIFT;
	if (str == "leftshift") return LEFTSHIFT;
	std::cout << "Unknown Instruction: " << str << std::endl;
	std::exit(EXIT_FAILURE);
}

// Types
enum type {
	REGISTER,
	POINTER,
	MEMORY,
	PORT,
	VALUE
};

type gettype(std::string str) {
	if (str == "register") return REGISTER;
	if (str == "pointer") return POINTER;
	if (str == "memory") return MEMORY;
	if (str == "port") return PORT;
	if (str == "value") return VALUE;
	std::cout << "Unknown Type: " << str << std::endl;
	std::exit(EXIT_FAILURE);
}

int main(int, char* argv[]) {
	argh::parser cmdl(argv);

	std::string outputfile;
	cmdl({"-o", "--output"}, "turtexe.out") >> outputfile;

	std::string outputarch;
	if (!(cmdl({"-a", "--arch"}) >> outputarch)) {
		std::cout << "No Output Architecture Specified! (Use '--arch' or '-a')" << std::endl;
		return EXIT_FAILURE;
	}

	// No File, Nothing To Do
	if (!cmdl(1)) {
		std::cout << "No Input File!" << std::endl;
		return EXIT_FAILURE;
	}
	std::string inputfile = cmdl[1];

	// Get Arch
	Arch* arch = getarch(outputarch);
	if (arch == nullptr) {
		std::cout << "Invaild Architecture" << std::endl;
		std::exit(EXIT_FAILURE);
	}
	std::stringstream output;
	// Initialize Architecture
	output << arch->initialize();

	// Open Object File
	std::ifstream objectfile(inputfile);
	if (!objectfile.is_open()) {
		std::cout << "Failed To Open Object File: " << inputfile << std::endl;
		return EXIT_FAILURE;
	}

	std::stringstream objectstream;
	objectstream << objectfile.rdbuf();
	objectfile.close();
	std::string objectstr(objectstream.str());
	char* objectchars = objectstr.data();

	rapidxml::xml_document<> object;
	object.parse<0>(objectchars);

	rapidxml::xml_node<>* program = object.first_node("program");

	long instid = 0;
	for (rapidxml::xml_node<>* instruction = program->first_node("instruction"); instruction; instruction = instruction->next_sibling() ) {
		// Keep Track Of Instuction ID For Errors
		instid ++;
		//

		inst it = getinst(instruction->first_attribute("type")->value());
		int s1;
		type s1t;
		int s2;
		type s2t;
		int d;
		type dt;
		int sid = 1;
		for (rapidxml::xml_node<>* node = instruction->first_node(); node; node = node->next_sibling()) {
			type t = gettype(node->first_attribute("type")->value());
			int v = getval(std::string(node->value()));
			if (node->name() == "source") {
				if (sid == 1) {
					s1 = v;
					s1t = t;
				}
				else if (sid == 2) {
					s2 = v;
					s2t = t;
				}
				sid++;
			}
			else if (node->name() == "destination") {
				d = v;
				dt = t;
			}
			else {
				std::cout << "Found Tag Other Than <source> and <destination> !!" << std::endl;
				return EXIT_FAILURE;
			}
		}
		// Get Instruction
		std::string istr;	// Arch Output String
		switch (it) {
			// Copy Instruction
			case COPY:
				switch (s1t) {
					case REGISTER:
						switch (dt) {
							case REGISTER:
								istr = arch->copy_register_register(s1, d);
								break;
							case MEMORY:
								istr = arch->copy_register_port(s1, d);
								break;
							case POINTER:
								istr = arch->copy_register_pointer(s1, d);
								break;
							case PORT:
								istr = arch->copy_register_port(s1, d);
								break;
						}
						break;
					case VALUE:
						if (dt == REGISTER) istr = arch->copy_value_register(s1, d);
						break;
					case MEMORY:
						if (dt == REGISTER) istr = arch->copy_value_register(s1, d);
						break;
					case POINTER:
						if (dt == REGISTER) istr = arch->copy_pointer_register(s1, d);
						break;
					case PORT:
						if (dt == REGISTER) istr = arch->copy_port_register(s1, d);
				}
				break;
			// Stack Instructions
			case PUSH:
				istr = arch->push_register(s1);
				break;
			case POP:
				istr = arch->pop_register(d);
				break;
			// ALU Math Instructions
			case ADD:
				if (s1t == REGISTER && s2t == REGISTER) istr = arch->add_register_register(s1,s2,d);
				if (s1t == VALUE && s2t == REGISTER) istr = arch->add_value_register(s1,s2,d);
				if (s1t == REGISTER && s2t == VALUE) istr = arch->add_register_value(s1,s2,d);
				break;
			case SUBTRACT:
				if (s1t == REGISTER && s2t == REGISTER) istr = arch->subtract_register_register(s1,s2,d);
				if (s1t == VALUE && s2t == REGISTER) istr = arch->subtract_value_register(s1,s2,d);
				if (s1t == REGISTER && s2t == VALUE) istr = arch->subtract_register_value(s1,s2,d);
				break;
			case XOR:
				if (s1t == REGISTER && s2t == REGISTER) istr = arch->xor_register_register(s1,s2,d);
				if (s1t == VALUE && s2t == REGISTER) istr = arch->xor_value_register(s1,s2,d);
				if (s1t == REGISTER && s2t == VALUE) istr = arch->xor_register_value(s1,s2,d);
				break;
			case OR:
				if (s1t == REGISTER && s2t == REGISTER) istr = arch->or_register_register(s1,s2,d);
				if (s1t == VALUE && s2t == REGISTER) istr = arch->or_value_register(s1,s2,d);
				if (s1t == REGISTER && s2t == VALUE) istr = arch->or_register_value(s1,s2,d);
				break;
			case AND:
				if (s1t == REGISTER && s2t == REGISTER) istr = arch->and_register_register(s1,s2,d);
				if (s1t == VALUE && s2t == REGISTER) istr = arch->and_value_register(s1,s2,d);
				if (s1t == REGISTER && s2t == VALUE) istr = arch->and_register_value(s1,s2,d);
				break;
			case NOT:
				if (s1t == REGISTER) istr = arch->not_register(s1);
				break;
			case RIGHTSHIFT:
				if (s1t == REGISTER) istr = arch->rightshift_register(s1);
				break;
			case LEFTSHIFT:
				if (s1t == REGISTER) istr = arch->leftshift_register(s1);
				break;
		}
		if (istr == "") {
			std::cout << "Unsupported Instruction Or Instruction Combination: <instruction> #" << instid << std::endl;
			std::exit(EXIT_FAILURE);
		}
		output << istr;
	}

	// Finalize Architecture
	output << arch->finalize();
	
	std::ofstream outputbin(outputfile);
	outputbin << output.str();
	outputbin.close();
	return EXIT_SUCCESS;
}
