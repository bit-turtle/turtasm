#include "arch.hpp"
#include <string>
using std::string;

class ExampleArch : public Arch {
	// Runs Before Linking
	string initialize() {
		return "example";
	}
	// Runs After Linking
	string finalize() {
		return "arch";
	}
};
