// Architecture Defininition Base Class
// All Functions Return String To Be Appended to Output
#ifndef ARCH
#define ARCH
// Begin Arch
#include <string>
using std::string;
class Arch {
	public:
		// Initializer and Finalizer Functions
		virtual string initialize() {
			return "";
		}
		virtual string finalize() {
			return "";
		}
		// Instruction Functions
		// * Returns "" if Unsupported
		// Copy Instructions
		virtual string copy_register_register(int reg1, int reg2) {
			return "";
		};
		virtual string copy_value_register(int value, int reg) {
			return "";
		};
		virtual string copy_memory_register(int mem, int reg) {
			return "";
		};
		virtual string copy_register_memory(int reg, int mem) {
			return "";
		};
		virtual string copy_pointer_register(int reg1, int reg2) {
			return "";
		};
		virtual string copy_register_pointer(int reg1, int reg2) {
			return "";
		};
		virtual string copy_register_port(int reg, int port) {
			return "";
		}
		virtual string copy_port_register(int port, int reg) {
			return "";
		}
		// Stack Instructions
		virtual string push_register(int reg) {
			return "";
		}
		virtual string pop_register(int reg) {
			return "";
		}
		// ALU Math and Logic Instructions
		virtual string add_register_register(int reg1, int reg2, int destreg) {
			return "";
		}
		virtual string add_value_register(int reg1, int reg2, int destreg) {
			return "";
		}
		virtual string add_register_value(int reg1, int reg2, int destreg) {
			return "";
		}
		virtual string subtract_register_register(int reg1, int reg2, int destreg) {
			return "";
		}
		virtual string subtract_value_register(int reg1, int reg2, int destreg) {
			return "";
		}
		virtual string subtract_register_value(int reg1, int reg2, int destreg) {
			return "";
		}
		virtual string xor_register_register(int reg1, int reg2, int destreg) {
			return "";
		}
		virtual string xor_value_register(int reg1, int reg2, int destreg) {
			return "";
		}
		virtual string xor_register_value(int reg1, int reg2, int destreg) {
			return "";
		}
		virtual string or_register_register(int reg1, int reg2, int destreg) {
			return "";
		}
		virtual string or_value_register(int reg1, int reg2, int destreg) {
			return "";
		}
		virtual string or_register_value(int reg1, int reg2, int destreg) {
			return "";
		}
		virtual string and_register_register(int reg1, int reg2, int destreg) {
			return "";
		}
		virtual string and_value_register(int reg1, int reg2, int destreg) {
			return "";
		}
		virtual string and_register_value(int reg1, int reg2, int destreg) {
			return "";
		}
		// One Register ALU
		virtual string not_register(int reg) {
			return "";
		}
		virtual string rightshift_register(int reg) {
			return "";
		}
		virtual string leftshift_register(int reg) {
			return "";
		}
};
// End Arch
#endif
