// Architecture Definition File
// Add New Architectures Here
#include "arch.hpp"
// Include Architecture Classes
#include "example.hpp"	// ExampleArch
#include "cxx.hpp"	// CxxArch	// C++ Architecture

// Architecture Getting Function
Arch* getarch(std::string name) {
	// If Name Return New Arch
	if (name == "example") return new ExampleArch();
	if (name == "cxx") return new CxxArch();
	// Otherwise Return Empty A Null Pointer
	return nullptr;
}
