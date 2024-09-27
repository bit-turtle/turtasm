# TurtASM Compiler
TurtASM is a cross platform compiler that tries to be compatible with every type of computer ever through a simple system to add architectures. It is similar to assembly (That's why it is called *'...ASM'*)

## How It Works
1. The TurtASM Compiler `turtasm` compiles the source code to a IR Representation in XML
2. The TurtASM Linker `turtlink` links the IR Representation into a format that can run on the target architecture

### Using The Commands
* TurtASM Compiler: `turtasm [inputfiles...] -o outputfile`
* TurtASM Linker: `turtlink inputfile --arch architecturename -o outputfile`

#### Adding An Architecture
Architectures in turtasm are classes that derive from the `Arch` class. To add a new architecture open `arch/def.hpp` and either include a file with your architecture class in it or define the class right in `def.hpp`. After including your class definition add an `if` statement in the function `Arch* getarch(std::string)` that returns `new YourArch()` when the name matches. There are examples for this in comments in `arch/def.hpp`. For an example architecture class look at `arch/examplearch.hpp`. Architecture classes work by overriding the `virtual` fucntions in the `Arch` class.  
After you add an architecture, recompile `turtlink` to be able to use your architecture.
