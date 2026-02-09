# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2024-XX-XX

### Added
- Basic Zephyr example
- Character echo parameter
- Asynchronous commands
- Command abort functionality
- getContext() method to ServerHandle
- setContext() method for Server

### Changed
- Improved CMake configuration
- Better access control
- Renamed variables for clarity
- Command IDs in command line are now always encoded with 2 bytes

### Fixed
- Edge cases when there are no basic or extended commands
- Missing detail headers
- Freeze on ERROR return code
- Incorrect Trie child count
- Trie packed representation
- Subtree size field size limited to 3 bytes

### Performance
- Command IDs in cmdline are always encoded with 2 bytes

### Documentation
- Added Doxygen documentation generation
- Added Github Pages upload

### Testing
- Added Trie tests
- Added Github Actions to run tests

## [0.0.1] - 2024-01-XX

### Added
- Initial release of AtServer library
