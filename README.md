# AT Command Server Library

A modern, cross-platform C++23 library for parsing and executing AT commands according to the ITU-T V.250 standard.

S-parameters have built-in support while other commands require custom implementation as separate classes added during server configuration.

## Features

### Command Support
- **S-parameters**: S3 (Command line termination character) and S4 (Response formatting character) parameters are supported
- **Basic commands**: Single-letter commands: I (Request identification information), V (DCE response format), etc.
- **Ampersand commands**: Same as basic commands, but prefixed with &
- **Extended syntax commands**: + prefixed extended syntax commands (+GCI, +GMI, etc.)

### Parameter Parsing
#### Basic and ampersand commands
Basic and ampersand commands may accept a single numeric parameter. The value is parsed and can be used for processing (see, for example, the V-command).
A command can be configured as parameter-less. In this case, any attempt to provide a parameter value will be treated as an error.

Numeric parameter ranges can be configured for validation. Out-of-range parameters will be treated as an error.

#### Extended syntax commands
Extended syntax commands may accept several comma-separated parameters of different types.
For every parameter, a default value can be set. Default values are processed according to V.250 standard.

##### Numeric parameters
Decimal, hexadecimal and binary parameters are supported. Every parameter can be range-validated.

##### String parameters
Strings should be quoted, as required by the standard. For every string, a maximum length should be set. Exceeding this length is an error.

##### Hexadecimal strings
Non-standard type, but from the perspective of the standard, it is a simple quoted string. Introduced to facilitate BLOB input-output.
Should contain an even number of hex digits (0-9, A-F). Hyphen (-) and space characters may be used for formatting.

### Performance and memory use
The time complexity of parsing a single command line is O(n + k), where n is the number of characters in the command line and k is the total number of ranges for validation of numeric parameters.

The use of constant data structures is prioritized because they can be placed in FLASH memory in embedded systems. FLASH is usually cheaper and has a larger size than RAM.
Dynamic memory allocation is not used.

### Compile-Time Validation
Concepts and static asserts are used to catch many errors during compilation.

## Architecture
### State machine architecture
State machine is one of the fastest parsers available.

### Compile-Time Trie Construction
A Trie (prefix tree) is built at compile time to be used for parsing Extended Syntax Command names at runtime.
This enables O(1) complexity per letter during the name recognition.

### Binary search for basic commands
Even though linear search would still qualify as O(1) because the alphabet is constrained, binary search speeds things up a bit.

## Limitations

### Current Implementation Restrictions
- **Command line editing character not supported**: S5 parameter is not supported, because there is no raw command line buffer in the current architecture

### Design Constraints
- **Maximum commands**: Limited to 16,383 extended commands due to Trie encoding
- **Parameter buffer size**: Fixed at compile time, no dynamic resizing
- **String parameter limits**: Maximum string lengths defined at compile time

### Missing Features
- **Full V.250 compliance**: Some advanced features from the standard are not yet implemented
- **Multi-line commands**: Limited support for complex multi-line command sequences
- **Special characters in strings**: \XY encoded characters are not supported in strings for now

## Getting Started

See the `examples/at_terminal/` directory for a complete working example demonstrating how to:
- Define custom commands
- Configure the server settings
- Handle command execution
- Process parameters

## Building

The project uses CMake and requires C++23 support:

```bash
mkdir build
cd build
cmake ..
make
```

### Requirements
- C++23 compatible compiler (GCC 13+, Clang 16+, MSVC 19.30+)
- CMake 3.28+

## License
This software is provided under the MIT License. See LICENSE.txt for the complete license text.

## Contributing

Contributions are welcome! Bug reports and feature requests are highly appreciated too.
