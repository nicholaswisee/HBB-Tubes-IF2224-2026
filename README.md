# HBB Tubes IF2224 2026

Simple compiler project in C++ for Arion

## Project Specification (Current Scope)

- Language: C++20
- Build system: Makefile
- Current implemented: Lexical Analyzer
- Input source files: `test/input/<filename>`
- Output: token stream printed to stdout

## Project Structure

- `src/`
    - `main.cpp`: program entry point, runs scanner and prints tokens
    - `lexical/`: scanner, token model, source buffer, logger
- `test/input/`: sample input files for scanner tests
- `test/output/`: output folder (optional/manual use)
- `obj/`: build artifacts
- `bin/compiler`: executable output

## Prerequisites

- `g++` with C++20 support
- `make`

## Build and Run

### Build

```bash
make all
```

### Run with an input file (recommended)

```bash
make run FILE=input1.txt
```

`FILE` must be a filename that exists in `test/input/`.

### Run executable directly

```bash
./bin/compiler input1.txt
```

### Clean build artifacts

```bash
make clean
```

## Notes

- If no file is provided to `make run`, Makefile shows usage:
    - `Usage: make run FILE=<filename-in-test-input>`

## Authors

HBB - GTFOTBFO

- Nathanael Imandatua Manurung - 13524021
- Nicholas Wise Saragih Sumbayak - 13524037
- Kurt Mikhael Purba - 13524065}
- Rava Khoman Tuah Saragih - 13524107}
