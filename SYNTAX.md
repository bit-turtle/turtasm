# Turtasm Language Syntax
In TurtASM, everything has to do with the flow of data. `->` is the most important symbol.
## Comments
Comments in TurtASM are everything after a `#` (Example: `r1 -> r2 # This is a comment`)
## Moving Data
The `->` operator is used to move data from a source to a destination ( Example: `r1 -> r2` )
### Special Rules For Moving Data
* Either the source or the destination has to include a register
## Locations
* Registers ( `rRegister` )
  - Stores numbers for computations
* Memory Reference ( `[location]` )
  - A permanent reference to a location in RAM
* Pointer Reference ( `[rRegister]` )
  - A reference to the location in RAM corresponding to the value stored in a register
## Data Types, Prefixes, and Brackets
* `f` and `s` Prefix
  - The `f` and `s` prefixes indicate that the name following it is the name of the function/subroutine to create
* `b` Prefix
  - The `b` prefix indicates that the values following it will be the name of the RAM memory reservation and the size of the RAM memory reservation
* `r` Prefix
  - The `r` prefix indicates that the number following it is a reference to a register
* `[` and `]` Brackets
  - Square brackets indicate that the value contained inside is a location in RAM
* `$` Prefix
  - The `$` prefix indicates that the name after it references either the name of a reserved location in RAM or the name of a function ( Contextual )
* Value Prefixes ( Indicate the type of number following it )
  - `d` Prefix: Decimal Number
  - `o` Prefix: Octal Number
  - `x` Prefix: Hexadecimal Number
  - `c` Prefix: Character Value ( char Value of **1** character )
## Functions
Functions in TurtASM are not much more than a location in the program. To pass values into a function, set the values of registers for the function to read. ( Example: `f main` Creates a function called main )
## Memory Reservations
Memory reservations are a way to give names and sizes to locations in ram. ( Example: `b memres d32` Reserves 32 bytes under name `memres` )
