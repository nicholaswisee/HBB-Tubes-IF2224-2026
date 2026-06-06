# Arion Compiler - Tugas Besar IF2224 Teori Bahasa Formal dan Otomata

Kelompok **GTFOTBFO (HBB)**

## Deskripsi Program

Program ini adalah implementasi compiler sekaligus interpreter untuk bahasa pemrograman **Arion**, sebuah bahasa mirip Pascal. Compiler bekerja melalui pipeline lima tahap:

1. **Lexical Analysis** - membaca source code karakter per karakter dan menghasilkan stream token.
2. **Syntax Analysis** - memvalidasi urutan token menggunakan recursive descent parser dan membangun Parse Tree.
3. **Semantic Analysis** - mengonversi Parse Tree menjadi AST, melakukan pengecekan tipe, scope, dan mengisi symbol table.
4. **Intermediate Code Generation** - menghasilkan instruksi P-Code berbasis stack machine dari Decorated AST.
5. **Interpreter** - mengeksekusi instruksi P-Code pada virtual stack machine.

## Struktur Proyek

```
.
├── src/
│   ├── main.cpp                        # Entry point pipeline lengkap
│   ├── driver_m1.cpp                   # Driver testing M1 (lexer)
│   ├── driver_m2.cpp                   # Driver testing M2 (parser)
│   ├── driver_m3.cpp                   # Driver testing M3 (semantic)
│   ├── driver_m4.cpp                   # Driver testing M4 (code gen + interpreter)
│   ├── lexical/
│   │   ├── Scanner.cpp / Scanner.hpp   # Lexical analyzer
│   │   ├── Token.cpp / Token.hpp       # Definisi token
│   │   └── SourceBuffer.cpp            # Buffer pembacaan source
│   ├── syntax/
│   │   └── Parser.cpp / Parser.hpp     # Recursive descent parser
│   ├── semantic/
│   │   ├── ParseTreeToAST.cpp          # Konversi Parse Tree ke AST
│   │   ├── SemanticAnalyzer.cpp        # Pengecekan tipe dan scope
│   │   ├── SymbolTableManager.cpp      # Manajemen tab, btab, atab
│   │   ├── TypeSystem.cpp              # Definisi dan kompatibilitas tipe
│   │   └── ASTNode.hpp                 # Definisi node AST
│   ├── intermediate/
│   │   ├── CodeGenerator.cpp           # Menghasilkan instruksi P-Code dari AST
│   │   └── Instruction.cpp             # Definisi instruksi
│   └── runtime/
│       ├── StackMachine.cpp            # Stack machine berbasis display/frame
│       └── Interpreter.cpp             # Eksekutor instruksi P-Code
├── test/
│   ├── milestone1/input/               # Test case M1
│   ├── milestone2/input/               # Test case M2
│   ├── milestone3/input/               # Test case M3
│   └── milestone4/input/               # Test case M4
├── doc/                                # Dokumentasi per milestone
├── bin/                                # Hasil build executable
├── obj/                                # Hasil kompilasi object files
└── Makefile
```

## Requirements

- `g++` dengan dukungan C++20
- `make`

## Cara Build dan Menjalankan

### Build

```bash
make
```

### Menjalankan compiler penuh (M1 sampai M4)

```bash
./bin/compiler <filename> [milestone]
```

- `<filename>`: nama file yang ada di folder `test/milestone<N>/input/`
- `[milestone]`: opsional, default `3`. Isi `4` untuk menjalankan sampai eksekusi.

Contoh:

```bash
./bin/compiler input1.txt 4
```

### Build dan menjalankan driver per milestone

```bash
make drivers
```

**M1 - Lexical Analysis:**

```bash
./bin/driver_m1 <filename>
# atau
make test-m1 FILE=<filename>
```

File diambil dari `test/milestone1/input/`.

**M2 - Syntax Analysis:**

```bash
./bin/driver_m2 <filename>
# atau
make test-m2 FILE=<filename>
```

File diambil dari `test/milestone2/input/`.

**M3 - Semantic Analysis:**

```bash
./bin/driver_m3 <filename>
# atau
make test-m3 FILE=<filename>
```

File diambil dari `test/milestone3/input/`.

**M4 - Intermediate Code Generation & Interpreter:**

```bash
./bin/driver_m4 <filename>
# atau
make test-m4 FILE=<filename>
```

File diambil dari `test/milestone4/input/`.

### Membersihkan hasil build

```bash
make clean
```

## Spesifikasi yang Dikerjakan

### Milestone 1: Lexical Analysis

Implementasi Scanner yang membaca source code Arion dan menghasilkan stream token. Mendukung semua token bahasa Arion: keyword, identifier, literal integer/real/char/string, operator, delimiter, dan komentar (`{ }` dan `(* *)`).

### Milestone 2: Syntax Analysis

Implementasi recursive descent parser yang memvalidasi token sesuai grammar bahasa Arion dan membangun Parse Tree. Mendukung deklarasi const/type/var/procedure/function, statement (assignment, if/else, while, repeat/until, for, case, compound), dan ekspresi lengkap.

### Milestone 3: Semantic Analysis

Implementasi analisis semantik mencakup konversi Parse Tree ke AST, pengecekan tipe ekspresi dan assignment, validasi deklarasi dan scope, serta pengisian symbol table (`tab`, `btab`, `atab`).

### Milestone 4: Intermediate Code Generation dan Interpreter

Implementasi code generator yang menghasilkan instruksi P-Code (`INT`, `LIT`, `LOD`, `STO`, `LODA`, `STOA`, `CAL`, `JMP`, `JPC`, `OPR`, `RET`) dari Decorated AST, serta interpreter berbasis stack machine yang mengeksekusi instruksi tersebut. Mendukung nested scope, pemanggilan procedure/function, dan deteksi runtime error.

## Pembagian Tugas

### Milestone 1

| NIM      | Nama                                    | Persentase |
| -------- | --------------------------------------- | ---------- |
| 13524021 | Nathanael Imandatua Manurung            | 25%        |
| 13524037 | Nicholas Wise Saragih Sumbayak          | 25%        |
| 13524065 | Kurt Mikhael Purba                      | 25%        |
| 13524107 | Rava Khoman Tuah Saragih                | 25%        |

### Milestone 2

| NIM      | Nama                                    | Persentase |
| -------- | --------------------------------------- | ---------- |
| 13524021 | Nathanael Imandatua Manurung            | 25%        |
| 13524037 | Nicholas Wise Saragih Sumbayak          | 25%        |
| 13524065 | Kurt Mikhael Purba                      | 25%        |
| 13524107 | Rava Khoman Tuah Saragih                | 25%        |

### Milestone 3

| NIM      | Nama                                    | Persentase |
| -------- | --------------------------------------- | ---------- |
| 13524021 | Nathanael Imandatua Manurung            | 25%        |
| 13524037 | Nicholas Wise Saragih Sumbayak          | 25%        |
| 13524065 | Kurt Mikhael Purba                      | 25%        |
| 13524107 | Rava Khoman Tuah Saragih                | 25%        |

### Milestone 4

| NIM      | Nama                                    | Persentase |
| -------- | --------------------------------------- | ---------- |
| 13524021 | Nathanael Imandatua Manurung            | 25%        |
| 13524037 | Nicholas Wise Saragih Sumbayak          | 25%        |
| 13524065 | Kurt Mikhael Purba                      | 25%        |
| 13524107 | Rava Khoman Tuah Saragih                | 25%        |

## Anggota Kelompok

Nama Kelompok: **GTFOTBFO (HBB)**

| NIM      | Nama                            |
| -------- | ------------------------------- |
| 13524021 | Nathanael Imandatua Manurung    |
| 13524037 | Nicholas Wise Saragih Sumbayak  |
| 13524065 | Kurt Mikhael Purba              |
| 13524107 | Rava Khoman Tuah Saragih        |
