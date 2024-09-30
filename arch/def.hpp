// Architecture Definition File
// Add New Architectures Here
#include "arch.hpp"
// Include Architecture Classes
#include "example.hpp"

// Architecture Getting Function
Arch* getarch(std::string name) {
	// If Name Return New Arch
	if (name == "example") return new ExampleArch();
	// Otherwise Return Empty A Null Pointer
	return nullptr;
}
