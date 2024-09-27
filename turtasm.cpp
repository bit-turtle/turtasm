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

struct macro {
	std::string name;
	unsigned int location;
};

void error(std::string error, macro location, std::string errorline, std::string description) {
	std::cout << "[" << error << "] " << description << std::endl;
	std::cout << location.name << ":" << location.location << " -> " << errorline << std::endl;
}

enum types {
	REGISTER,
	PORT,
	MEMORY,
	POINTER,
	VALUE,
	DATA,
	MEMORYDATA
};

struct value {
	types type;
	std::string data;
	unsigned int value;
	bool error = false;
};

value getvalue(std::string string) {
	value val;
	char prefix = string[0];
	string = string.substr(1,string.length()-1);
	switch (prefix) {
		case 'd': {	// Decimal
			val.type = VALUE;
			val.value = std::stoul(string, nullptr, 10);	// Base 10
			break;
		}
		case 'x': {	// Hexadecimal
			val.type = VALUE;
			val.value = std::stoul(string, nullptr, 16);	// Base 16
			break;
		}
		case 'o': {	// Octal
			val.type = VALUE;
			val.value = std::stoul(string, nullptr, 8);	// Base 8
			break;
		}
		case 'r': {	// Register
			val.type = REGISTER;
			val.value = std::stoul(string, nullptr, 10);	// Base 10
			break;
		}
		case 'p': {	// Port
			val.type = PORT;
			val.value = std::stoul(string, nullptr, 10);	// Base 10
			break;
		}
		case 'c': {	// Character
			if (string.length() > 1) {
				std::cout << "[CharacterError] More Than One Character Found After 'c' Prefix" << std::endl;
				std::cout << "-> " << string << std::endl;
				val.error = true;
				return val;
			}
			val.type = VALUE;
			val.value = (unsigned int) string[0];
			break;
		}
		case '[': {	// Memory
			if (string[string.length()-1] != ']') {
				std::cout << "[BracketError] Closing ']' Not Found" << std::endl;
				val.error = true;
				return val;
			}
			std::string internalvalue = string.substr(0,string.length()-1);
			value in = getvalue(internalvalue);
			if (in.error) {
				val.error;
				return val;
			}
			if (in.type == POINTER || in.type == MEMORY || in.type == MEMORYDATA) {
				std::cout << "[MemoryError] References to Memory in References to Memory Are Not Allowed" << std::endl;
				val.error = true;
				return val;
			}
			val = in;
			if (in.type == VALUE) val.type = MEMORY;
			if (in.type == REGISTER) val.type = POINTER;
			if (in.type == DATA) val.type = MEMORYDATA;
			break;
		}
		case '$': {	// Data
			if (string.length() == 0) {
				std::cout << "[SyntaxError] No Data Reference After Prefix '$'" << std::endl;
				val.error = true;
				return val;
			}
			val.type = DATA;
			val.data = string;
			return val;
		}
		default: {
			std::cout << "[PrefixError] Unknown Prefix: '" << prefix << "' (Hint: Try Adding 'd' as A Prefix)" << std::endl;
			val.error = true;
			return val;
		}
	}
	return val;
}

int main(int, char* argv[]) {
	argh::parser cmdl(argv);

	std::string outputfile;
	cmdl({"-o", "--output"}, "turtobj.xml") >> outputfile;

	// No File, Nothing To Do
	if (!cmdl(1)) {
		std::cout << "No Input File!" << std::endl;
		return 1;
	}
	// Merge All Specified Files into One
	std::vector<macro> fileloc;
	std::vector<std::string> file;
	for (unsigned int i = 1; cmdl(i); i++) {
		std::ifstream source(cmdl[i]);
		if (!source.is_open()) {
			std::cout << "File Not Found: " << cmdl[i] << std::endl;
			return 2;
		}
		std::string line;
		unsigned int linenum = 1;
		while (std::getline(source, line)) {
			macro lineloc;
			lineloc.name = cmdl[i];
			lineloc.location = linenum;
			fileloc.push_back(lineloc);
			linenum++;
			file.push_back(line);
		}
	}
	// Trim Comments #
	for (int i = 0; i < file.size(); i++) {
		std::stringstream line(file[i]);
		std::getline(line, file[i], '#');
	}
	// Trim Whitespace
	for (int i = 0; i < file.size(); i++) {
		std::stringstream line(file[i]);
		std::string word;
		std::stringstream output;
		bool whitespace = false;
		while( std::getline(line, word, ' ') ) {
			std::stringstream tabs(word);
			while ( std::getline(tabs, word, '\t') ) {
				if (whitespace) output << " ";
				if (word.length() == 0) {
					whitespace = false;
					continue;
				}
				output << word;
				whitespace = true;
			}
		}
		file[i] = output.str();
	}
	// Trim Empty Lines
	for (int i = 0; i < file.size(); i++) if (file[i].empty()) {
		fileloc.erase(fileloc.begin()+i);
		file.erase(file.begin()+i);
		i--;
	}
	// Compile
	// Set Up XML Document
	rapidxml::xml_document<> objectxml;
	rapidxml::xml_node<>* decl = objectxml.allocate_node(rapidxml::node_type::node_declaration);
	decl->append_attribute(objectxml.allocate_attribute("version", "1.0"));
	decl->append_attribute(objectxml.allocate_attribute("encoding", "utf-8"));
	objectxml.append_node(decl);
	rapidxml::xml_node<>* output = objectxml.allocate_node(rapidxml::node_type::node_element, "program");
	output->append_attribute(objectxml.allocate_attribute("version", "1.0"));
	output->append_attribute(objectxml.allocate_attribute("type", "turtasm-object"));
	objectxml.append_node(output);
	char* xml_empty = objectxml.allocate_string("");
	char* xml_data = objectxml.allocate_string("data");
	char* xml_instruction = objectxml.allocate_string("instruction");
	char* xml_source = objectxml.allocate_string("source");
	char* xml_destination = objectxml.allocate_string("destination");
	char* attr_type = objectxml.allocate_string("type");
	// Instruction Types
	char* type_call = objectxml.allocate_string("call");
	char* type_jump = objectxml.allocate_string("jump");
	char* type_return = objectxml.allocate_string("return");
	char* type_copy = objectxml.allocate_string("copy");
	char* type_add = objectxml.allocate_string("add");
	char* type_subtract = objectxml.allocate_string("subtract");
	char* type_xor = objectxml.allocate_string("xor");
	char* type_or = objectxml.allocate_string("or");
	char* type_and = objectxml.allocate_string("and");
	char* type_not = objectxml.allocate_string("not");
	char* type_rightshift = objectxml.allocate_string("rightshift");
	char* type_leftshift = objectxml.allocate_string("leftshift");
	// Stack Instructions
	char* type_push = objectxml.allocate_string("push");
	char* type_pop = objectxml.allocate_string("pop");
	// Conditional Instruction Types
	char* type_condition_if_zero = objectxml.allocate_string("condition_if_zero");
	char* type_condition_not_zero = objectxml.allocate_string("condition_not_zero");
	char* type_condition_if_overflow = objectxml.allocate_string("condition_if_overflow");
	char* type_condition_not_overflow = objectxml.allocate_string("condition_not_overflow");
	// Data Types
	char* type_register = objectxml.allocate_string("register");
	char* type_port = objectxml.allocate_string("port");
	char* type_memory = objectxml.allocate_string("memory");
	char* type_pointer = objectxml.allocate_string("pointer");
	char* type_value = objectxml.allocate_string("value");
	char* type_input = objectxml.allocate_string("input");
	char* type_output = objectxml.allocate_string("output");
	// Compile Variables
	unsigned int instruction = 0;
	std::vector<macro> functions;
	std::vector<macro> data;
	unsigned int datasize = 0;
	bool errorcompile = false;
	for (int i = 0; i < file.size(); i++) {
		std::stringstream line(file[i]);
		std::string operation;
		line >> operation;
		if (operation == "f" || operation == "s") { // 'f'unction Macro / 's'ubroutine (Same thing)
			if (line.rdbuf()->in_avail() == 0) {
				errorcompile = true;
				error("SyntaxError", fileloc[i], "", "No Name For Function Specified");
				break;
			}
			std::string name;
			line >> name;
			macro function;
			function.name = name;
			function.location = instruction;
			functions.push_back(function);
			if (!line.rdbuf()->in_avail() == 0) {
				errorcompile = true;
				error("JunkError", fileloc[i], line.str(), "Junk Characters After Function Declaration (Forgot To Comment?)");
				break;
			}
			continue;
		}
		if (operation == "b") {	// Data Macro reserve 'b'yte
			if (line.rdbuf()->in_avail() == 0) {
				errorcompile = true;
				error("SyntaxError", fileloc[i], "", "No Name For Reserved Bytes Specified");
				break;
			}
			std::string name;
			line >> name;
			macro mem;
			mem.location = datasize;
			mem.name = name;
			data.push_back(mem);
			std::string size;
			if (line.rdbuf()->in_avail() == 0) {
				datasize++;	// Default 1 Byte
			}
			else {
				std::string sizestr;
				line >> sizestr;
				value sizeval = getvalue(sizestr);
				if (sizeval.error) {
					errorcompile = true;
					error("ValueError", fileloc[i], line.str(), "Error Parsing Value");
					break;
				}
				if (sizeval.type != VALUE) {
					errorcompile = true;
					error("TypeError", fileloc[i], line.str(), "Value Type Is Not A Number");
					break;
				}
				datasize += sizeval.value;
			}
			if (!line.rdbuf()->in_avail() == 0) {
				errorcompile = true;
				error("JunkError", fileloc[i], line.str(), "Junk Characters After Reserved Byte Declaration (Forgot To Comment?)");
				break;
			}
			continue;
		}
		// Return Command
		if (operation == "<") {
			rapidxml::xml_node<>* instruction = objectxml.allocate_node(rapidxml::node_type::node_element, xml_instruction, xml_empty);
			instruction->append_attribute(objectxml.allocate_attribute(attr_type, type_return));
			output->append_node(instruction);
			if (line.rdbuf()->in_avail() == 0) {
				errorcompile = true;
				error("JunkError", fileloc[i], line.str(), "Junk Characters After Return Statement (Forgot To Comment?)");
				break;
			}
			continue;
		}
		if (operation == "<-") {	// Stack Push Operation
			if (line.rdbuf()->in_avail() == 0) {
				errorcompile = true;
				error("SyntaxError", fileloc[i], line.str(), "No Register Specified For Push Operation");
				break;
			}
			std::string regstr;
			line >> regstr;
			value reg = getvalue(regstr);
			if (reg.error) {
				errorcompile = true;
				error("ValueError", fileloc[i], line.str(), "Error Parsing Value");
				break;
			}
			if (reg.type != REGISTER) {
				errorcompile = true;
				error("TypeError", fileloc[i], line.str(), "Value Type Is Not A Register");
				break;
			}
			if (!line.rdbuf()->in_avail() == 0) {
				errorcompile = true;
				error("JunkError", fileloc[i], line.str(), "Junk Characters After Stack Push Source (Forgot To Comment?)");
				break;
			}
			std::stringstream regid;
			// Add Push Instruction to Output XML
			regid << reg.value;
			rapidxml::xml_node<>* instruction = objectxml.allocate_node(rapidxml::node_type::node_element, xml_instruction, xml_empty);
			output->append_node(instruction);
			rapidxml::xml_attribute<>* type = objectxml.allocate_attribute(attr_type, type_push);
			instruction->append_attribute(type);
			rapidxml::xml_node<>* source = objectxml.allocate_node(rapidxml::node_type::node_element, xml_source, objectxml.allocate_string(regid.str().c_str()));
			instruction->append_node(source);
			rapidxml::xml_attribute<>* sourcetype = objectxml.allocate_attribute(attr_type, type_register);
			source->append_attribute(sourcetype);
			continue;
		}
		if (operation == "->") {	// Stack Pop Operation
			if (line.rdbuf()->in_avail() == 0) {
				errorcompile = true;
				error("SyntaxError", fileloc[i], line.str(), "No Register Specified For Pop Operation");
				break;
			}
			std::string regstr;
			line >> regstr;
			value reg = getvalue(regstr);
			if (reg.error) {
				errorcompile = true;
				error("ValueError", fileloc[i], line.str(), "Error Parsing Value");
				break;
			}
			if (reg.type != REGISTER) {
				errorcompile = true;
				error("TypeError", fileloc[i], line.str(), "Value Type Is Not A Register");
				break;
			}
			if (!line.rdbuf()->in_avail() == 0) {
				errorcompile = true;
				error("JunkError", fileloc[i], line.str(), "Junk Characters After Stack Push Destination (Forgot To Comment?)");
				break;
			}
			std::stringstream regid;
			// Add Pop Instruction to Output XML
			regid << reg.value;
			rapidxml::xml_node<>* instruction = objectxml.allocate_node(rapidxml::node_type::node_element, xml_instruction, xml_empty);
			output->append_node(instruction);
			rapidxml::xml_attribute<>* type = objectxml.allocate_attribute(attr_type, type_pop);
			instruction->append_attribute(type);
			rapidxml::xml_node<>* destination = objectxml.allocate_node(rapidxml::node_type::node_element, xml_destination, objectxml.allocate_string(regid.str().c_str()));
			instruction->append_node(destination);
			rapidxml::xml_attribute<>* destinationtype = objectxml.allocate_attribute(attr_type, type_register);
			destination->append_attribute(destinationtype);
			continue;
		}
		// Jump
		if (operation == ">>") {
			if (line.rdbuf()->in_avail() == 0) {
				errorcompile = true;
				error("SyntaxError", fileloc[i], line.str(), "No Instruction Location After '>'");
				break;
			}
			std::string locstr;
			line >> locstr;
			value location = getvalue(locstr);
			if (location.error) {
				errorcompile = true;
				error("ValueError", fileloc[i], line.str(), "Error Parsing Value");
				break;
			}
			if (location.type == MEMORYDATA || location.type == MEMORY || location.type == POINTER) {
				errorcompile = true;
				error("TypeError", fileloc[i], line.str(), "Invalid Type For Jump Instruction '>'");
				break;
			}
			char* type_name;
			rapidxml::xml_node<>* instruction = objectxml.allocate_node(rapidxml::node_type::node_element, xml_instruction, xml_empty);
			rapidxml::xml_node<>* source = objectxml.allocate_node(rapidxml::node_type::node_element, xml_source, xml_empty);
			
			// Only Registers and Values / Functions Allowed

			if (location.type == MEMORYDATA || location.type == MEMORY) {
				errorcompile = true;
				error("TypeError", fileloc[i], line.str(), "Can Not Jump To Memory References (Hint: Use '$' prefix and a function name)");
				break;
			}
			if (location.type == POINTER) {
				errorcompile = true;
				error("TypeError", fileloc[i], line.str(), "Can't Jump To Pointer (Hint: Just use the register)");
				break;
			}
			if (location.type == PORT) {
				errorcompile = true;
				error("TypeError", fileloc[i], line.str(), "Can't Jump To I/O Port (Hint: Use A Register)");
				break;
			}

			std::stringstream locatess;
			locatess << location.value;
			std::string locationstr(locatess.str());

			if (location.type == DATA) {
				macro dataloc;
				bool found = false;
				for (unsigned int d = 0; d < data.size(); d++) {
					if (functions[d].name == location.data) {
						dataloc = data[d];
						found = true;
						break;
					}
				}
				if (!found) {
					errorcompile = true;
					error("ReferenceError", fileloc[i], line.str(), "Function Reference ID Not Found (Hint: '$' Prefix)");
					break;
				}
				std::stringstream locss;
				locss << dataloc.location;
				std::string loc(locss.str());
				char* type_name;
				if (location.type == VALUE) type_name = type_value;
				if (location.type == REGISTER) type_name = type_register;
				rapidxml::xml_node<>* source = objectxml.allocate_node(rapidxml::node_type::node_element, xml_source, objectxml.allocate_string(loc.c_str()));
				source->append_attribute(objectxml.allocate_attribute(attr_type, type_name));
				instruction->append_node(source);
			}
			else {
				char* type_name;
				switch (location.type) {
					case REGISTER: type_name = type_register; break;
					case VALUE: type_name = type_value; break;
				}
				rapidxml::xml_node<>* source = objectxml.allocate_node(rapidxml::node_type::node_element, xml_source, objectxml.allocate_string(locationstr.c_str()));
				source->append_attribute(objectxml.allocate_attribute(attr_type, type_name));
				instruction->append_node(source);
			}
			instruction->append_attribute(objectxml.allocate_attribute(attr_type, type_jump));
			output->append_node(instruction);
			continue;
		}
		// Call
		if (operation == ">") {
			if (line.rdbuf()->in_avail() == 0) {
				errorcompile = true;
				error("SyntaxError", fileloc[i], line.str(), "No Instruction Location After '>'");
				break;
			}
			std::string locstr;
			line >> locstr;
			value location = getvalue(locstr);
			if (location.error) {
				errorcompile = true;
				error("ValueError", fileloc[i], line.str(), "Error Parsing Value");
				break;
			}
			if (location.type == MEMORYDATA || location.type == MEMORY || location.type == POINTER) {
				errorcompile = true;
				error("TypeError", fileloc[i], line.str(), "Invalid Type For Jump Instruction '>'");
				break;
			}
			char* type_name;
			rapidxml::xml_node<>* instruction = objectxml.allocate_node(rapidxml::node_type::node_element, xml_instruction, xml_empty);
			rapidxml::xml_node<>* source = objectxml.allocate_node(rapidxml::node_type::node_element, xml_source, xml_empty);
			
			// Only Registers and Values / Functions Allowed

			if (location.type == MEMORYDATA || location.type == MEMORY) {
				errorcompile = true;
				error("TypeError", fileloc[i], line.str(), "Can Not Jump To Memory References (Hint: Use '$' prefix and a function name)");
				break;
			}
			if (location.type == POINTER) {
				errorcompile = true;
				error("TypeError", fileloc[i], line.str(), "Can't Jump To Pointer (Hint: Just use the register)");
				break;
			}
			if (location.type == PORT) {
				errorcompile = true;
				error("TypeError", fileloc[i], line.str(), "Can't Jump To I/O Port (Hint: Use A Register)");
				break;
			}

			std::stringstream locatess;
			locatess << location.value;
			std::string locationstr(locatess.str());

			if (location.type == DATA) {
				macro dataloc;
				bool found = false;
				for (unsigned int d = 0; d < data.size(); d++) {
					if (functions[d].name == location.data) {
						dataloc = data[d];
						found = true;
						break;
					}
				}
				if (!found) {
					errorcompile = true;
					error("ReferenceError", fileloc[i], line.str(), "Function Reference ID Not Found (Hint: '$' Prefix)");
					break;
				}
				std::stringstream locss;
				locss << dataloc.location;
				std::string loc(locss.str());
				char* type_name;
				if (location.type == VALUE) type_name = type_value;
				if (location.type == REGISTER) type_name = type_register;
				rapidxml::xml_node<>* source = objectxml.allocate_node(rapidxml::node_type::node_element, xml_source, objectxml.allocate_string(loc.c_str()));
				source->append_attribute(objectxml.allocate_attribute(attr_type, type_name));
				instruction->append_node(source);
			}
			else {
				char* type_name;
				switch (location.type) {
					case REGISTER: type_name = type_register; break;
					case VALUE: type_name = type_value; break;
				}
				rapidxml::xml_node<>* source = objectxml.allocate_node(rapidxml::node_type::node_element, xml_source, objectxml.allocate_string(locationstr.c_str()));
				source->append_attribute(objectxml.allocate_attribute(attr_type, type_name));
				instruction->append_node(source);
			}
			instruction->append_attribute(objectxml.allocate_attribute(attr_type, type_call));
			output->append_node(instruction);
			continue;
		}
		// Conditional Operations
		if (operation == "?") {
			if (line.rdbuf()->in_avail() == 0) {
				errorcompile = true;
				error("SyntaxError", fileloc[i], line.str(), "No Condition After '?'");
				break;
			}
			line >> operation;
			char* type_name;
			bool regneeded = true;
			if (operation == "0") type_name = type_condition_if_zero;
			else if (operation == "!0") type_name = type_condition_not_zero;
			else if (operation == "overflow") {
				type_name = type_condition_if_overflow;
				regneeded = false;
			}
			else if (operation == "!overflow") {
				type_name = type_condition_not_overflow;
				regneeded = false;
			}
			else {
				errorcompile = true;
				error("ConditionError", fileloc[i], line.str(), "Unknown Condition (Hint: Try '0')");
				break;
			}
			rapidxml::xml_node<>* instruction = objectxml.allocate_node(rapidxml::node_type::node_element, xml_instruction, xml_empty);
			instruction->append_attribute(objectxml.allocate_attribute(attr_type, type_name));
			if (regneeded) {
				if (line.rdbuf()->in_avail() == 0) {
					errorcompile = true;
					error("SyntaxError", fileloc[i], line.str(), "Expected Register, Got Nothing (Hint: Add 'r0')");
					break;
				}
				std::string regstr;
				line >> regstr;
				value reg = getvalue(regstr);
				if (reg.error) {
					errorcompile = true;
					error("ValueError", fileloc[i], line.str(), "Error Parsing Value");
					break;
				}
				if (reg.type != REGISTER) {
					errorcompile = true;
					error("TypeError", fileloc[i], line.str(), "Expected Register But Got Something Else (Hint: 'r' Prefix)");
					break;
				}
				std::stringstream regss;
				regss << reg.value;
				std::string regval(regss.str());
				rapidxml::xml_node<>* regnode = objectxml.allocate_node(rapidxml::node_type::node_element, xml_source, objectxml.allocate_string(regval.c_str()) );
				regnode->append_attribute(objectxml.allocate_attribute(attr_type, type_register));
				instruction->append_node(regnode);
			}
			// Add instruction to program
			output->append_node(instruction);
			continue;
		}
		// Register Operations
		{
			bool hasmem = false;
			bool hasport = false;
			rapidxml::xml_node<>* instruction = objectxml.allocate_node(rapidxml::node_type::node_element, xml_instruction, xml_empty);
			bool isdone = false;
			bool hasreg = false;
			value source1 = getvalue(operation);
			if (source1.error) {
				errorcompile = true;
				error("ValueError", fileloc[i], line.str(), "Error Parsing Value");
				break;
			}
			std::stringstream source1ss;
			source1ss << source1.value;
			std::string source1str(source1ss.str());

			// HasMem
			if (source1.type == MEMORY || source1.type == MEMORYDATA || source1.type == POINTER) hasmem = true;
			// HasPort
			if (source1.type == PORT) hasport = true;

			if (source1.type == DATA || source1.type == MEMORYDATA) {
				macro location;
				bool found = false;
				for (unsigned int d = 0; d < data.size(); d++) {
					if (data[d].name == source1.data) {
						location = data[d];
						found = true;
						break;
					}
				}
				if (!found) {
					errorcompile = true;
					error("ReferenceError", fileloc[i], line.str(), "Data Reference ID Not Found (Hint: '$' Prefix)");
					break;
				}
				std::stringstream locss;
				locss << location.location;
				std::string loc(locss.str());
				char* type_name;
				if (source1.type == DATA) type_name = type_value;
				if (source1.type == MEMORYDATA) type_name = type_memory;
				rapidxml::xml_node<>* source = objectxml.allocate_node(rapidxml::node_type::node_element, xml_source, objectxml.allocate_string(loc.c_str()));
				source->append_attribute(objectxml.allocate_attribute(attr_type, type_name));
				instruction->append_node(source);
			}
			else {
				if (source1.type == REGISTER) hasreg = true;
				char* type_name;
				switch (source1.type) {
					case REGISTER: type_name = type_register; break;
					case PORT: type_name = type_port; break;
					case VALUE: type_name = type_value; break;
					case POINTER: type_name = type_pointer; break;
					case MEMORY: type_name = type_memory; break;
				}
				rapidxml::xml_node<>* source = objectxml.allocate_node(rapidxml::node_type::node_element, xml_source, objectxml.allocate_string(source1str.c_str()));
				source->append_attribute(objectxml.allocate_attribute(attr_type, type_name));
				instruction->append_node(source);
			}
			if (line.rdbuf()->in_avail() == 0) {
				errorcompile = true;
				error("SyntaxError", fileloc[i], line.str(), "No Operation Specified After Value");
				break;
			}
			line >> operation;
			bool regsource = true;	// At Least One Source Must Be A Register
			bool memsource = false;	// Memory Is An Allowed Source
			bool portsource = false;	// Port Is An Allowed Source
			bool secondsource = false;	// Second Source Needed
			bool regdestination = false;	// Destination Must Be A Register
			bool dest = true;	// Destination Required
			if (operation == "->") {	// Copy
				regsource = false;	// The Source Doesn't have to contain a register
				memsource = true;	// Allow Memory As A Source
				portsource = true;	// Allow Ports As A Source
				instruction->append_attribute(objectxml.allocate_attribute(attr_type, type_copy));
			}
			else if (operation == "+") {
				secondsource = true;
				regdestination = true;
				instruction->append_attribute(objectxml.allocate_attribute(attr_type, type_add));
			}
			else if (operation == "-") {
				secondsource = true;
				regdestination = true;
				instruction->append_attribute(objectxml.allocate_attribute(attr_type, type_subtract));
			}
			else if (operation == "^") {
				secondsource = true;
				regdestination = true;
				instruction->append_attribute(objectxml.allocate_attribute(attr_type, type_xor));
			}
			else if (operation == "|") {
				secondsource = true;
				regdestination = true;
				instruction->append_attribute(objectxml.allocate_attribute(attr_type, type_or));
			}
			else if (operation == "&") {
				secondsource = true;
				regdestination = true;
				instruction->append_attribute(objectxml.allocate_attribute(attr_type, type_and));
			}
			// One Register Math
			else if (operation == "!") {	// Not
				dest = false;	// Destination is itself
				instruction->append_attribute(objectxml.allocate_attribute(attr_type, type_not));
			}
			else if (operation == ">>") {	// Rightshift
				dest = false;	// Destination is itself
				instruction->append_attribute(objectxml.allocate_attribute(attr_type, type_rightshift));
			}
			else if (operation == "<<") {	// Leftshift
				dest = false;	// Destination is itself
				instruction->append_attribute(objectxml.allocate_attribute(attr_type, type_leftshift));
			}
			
			// Error
			else {
				errorcompile = true;
				error("OperationError", fileloc[i], line.str(), "Invalid Operation");
				break;
			}
			// Get Source2
			if (secondsource) {
				if (line.rdbuf()->in_avail() == 0) {
					errorcompile = true;
					error("SyntaxError", fileloc[i], line.str(), "Second Value Not Provided To Operation");
					break;
				}
				std::string source2txt;
				line >> source2txt;
				value source2 = getvalue(source2txt);
				if (source2.error) {
					errorcompile = true;
					error("ValueError", fileloc[i], line.str(), "Error Parsing Value");
					break;
				}
				std::stringstream source2ss;
				source2ss << source2.value;
				std::string source2str(source2ss.str());
				
				// HasMem
				if (source2.type == MEMORY || source2.type == MEMORYDATA || source2.type == POINTER) hasmem = true;
				// HasPort
				if (source2.type == PORT) hasport = true;

				if (source2.type == DATA || source2.type == MEMORYDATA) {
					macro location;
					bool found = false;
					for (unsigned int d = 0; d < data.size(); d++) {
						if (data[d].name == source2.data) {
							location = data[d];
							found = true;
							break;
						}
					}
					if (!found) {
						errorcompile = true;
						error("ReferenceError", fileloc[i], line.str(), "Data Reference ID Not Found (Hint: '$' Prefix)");
						break;
					}
					std::stringstream locss;
					locss << location.location;
					std::string loc(locss.str());
					char* type_name;
					if (source2.type == DATA) type_name = type_value;
					if (source2.type == MEMORYDATA) type_name = type_memory;
					rapidxml::xml_node<>* source = objectxml.allocate_node(rapidxml::node_type::node_element, xml_source, objectxml.allocate_string(loc.c_str()));
					source->append_attribute(objectxml.allocate_attribute(attr_type, type_name));
					instruction->append_node(source);
				}
				else {
					if (source2.type == REGISTER) hasreg = true;
					char* type_name;
					switch (source2.type) {
						case REGISTER: type_name = type_register; break;
						case PORT: type_name = type_port; break;
						case VALUE: type_name = type_value; break;
						case POINTER: type_name = type_pointer; break;
						case MEMORY: type_name = type_memory; break;
					}
					rapidxml::xml_node<>* source = objectxml.allocate_node(rapidxml::node_type::node_element, xml_source, objectxml.allocate_string(source2str.c_str()));
					source->append_attribute(objectxml.allocate_attribute(attr_type, type_name));
					instruction->append_node(source);
				}
				if (line.rdbuf()->in_avail() == 0) {
					errorcompile = true;
					error("SyntaxError", fileloc[i], line.str(), "Expected '->' (Hint: Add '->' and a Destination)");
					break;
				}
				line >> operation;
				if (operation != "->") {
					errorcompile = true;
					error("SyntaxError", fileloc[i], line.str(), "Expected '->' But Found Some Other Thing");
					break;
				}
			}

			// Memsource Violation
			if (!memsource && hasmem) {
				errorcompile = true;
				error("SourceError", fileloc[i], line.str(), "Using Memory As A Source Is Not Allowed Here (Hint: Try Using Registers)");
				break;
			}

			// Portsource Violation
			if (!portsource && hasport) {
				errorcompile = true;
				error("SourceError", fileloc[i], line.str(), "Using I/O Ports As A Source Is Not Allowed Here (Hint: Try Using Registers)");
				break;
			}

			// Regsource Violation
			if (regsource && !hasreg) {
				errorcompile = true;
				error("SourceError", fileloc[i], line.str(), "At Least One Source Must Be A Register");
				break;
			}

			if (dest) {
				
				if (line.rdbuf()->in_avail() == 0) {
					errorcompile = true;
					error("SyntaxError", fileloc[i], line.str(), "No Destination Provided (Hint: Add a Destination like 'r0')");
					break;
				}

				line >> operation;
				value destination = getvalue(operation);
				if (destination.error) {
					errorcompile = true;
					error("ValueError", fileloc[i], line.str(), "Error Parsing Value");
					break;
				}
				std::stringstream destinationss;
				destinationss << destination.value;
				std::string destinationstr(destinationss.str());
				if (destination.type == DATA) {
					errorcompile = true;
					error("DestinationError", fileloc[i], line.str(), "A Data Reference Pointer is Not A Destination (Hint: Use a Register)");
					break;
				}
				if (destination.type == VALUE) {
					errorcompile = true;
					error("DestinationError", fileloc[i], line.str(), "A Value is Not A Destination (Hint: Use a Register)");
					break;
				}

				// Enforce Register Destination
				if (regdestination) {
					if (destination.type != REGISTER) {
						errorcompile= true;
						error("DestinationError", fileloc[i], line.str(), "Expected A Register As A Destination (Hint: 'r' Prefix)");
						break;
					}
				}

				if (destination.type == REGISTER) hasreg = true;

				// Enforce Register As Source or Destination
				if (!hasreg) {
					errorcompile = true;
					error("SyntaxError", fileloc[i], line.str(), "Eitcher The Source Or Destination Must Be A Register");
				}

				if (destination.type == MEMORYDATA) {
					macro location;
					bool found = false;
					for (unsigned int d = 0; d < data.size(); d++) {
						if (data[d].name == destination.data) {
							location = data[d];
							found = true;
							break;
						}
					}
					if (!found) {
						errorcompile = true;
						error("ReferenceError", fileloc[i], line.str(), "Data Reference ID Not Found (Hint: '$' Prefix)");
						break;
					}
					std::stringstream locss;
					locss << location.location;
					std::string loc(locss.str());
					char* type_name;
					if (destination.type == MEMORYDATA) type_name = type_memory;
					rapidxml::xml_node<>* source = objectxml.allocate_node(rapidxml::node_type::node_element, xml_destination, objectxml.allocate_string(loc.c_str()));
					source->append_attribute(objectxml.allocate_attribute(attr_type, type_memory));
					instruction->append_node(source);
				}
				else {
					char* type_name;
					switch (destination.type) {
						case REGISTER: type_name = type_register; break;
						case PORT: type_name = type_port; break;
						case POINTER: type_name = type_pointer; break;
						case MEMORY: type_name = type_memory; break;
					}
					rapidxml::xml_node<>* source = objectxml.allocate_node(rapidxml::node_type::node_element, xml_destination, objectxml.allocate_string(destinationstr.c_str()));
					source->append_attribute(objectxml.allocate_attribute(attr_type, type_name));
					instruction->append_node(source);
				}
			}
			// Append Instruction
			output->append_node(instruction);
			continue;
		}
		// If Compilation Does Not 'continue' for Some Reason
		errorcompile = true;
		error("UnknownError", fileloc[i], line.str(), "Compiler Just Didn't Know What To Do With This Line ¯\\_( :) )/¯");
		break;
	}
	if (errorcompile) {
		std::cout << "[CompileError] Errors Occured, Compilation Failed" << std::endl;
		return EXIT_FAILURE;
	}
	// Output Final File
	std::string objectstr;
	rapidxml::print(std::back_inserter(objectstr), objectxml);
	std::cout << objectstr;
	std::ofstream outputobj(outputfile);
	outputobj << objectstr;
	outputobj.close();
	return EXIT_SUCCESS;
}
