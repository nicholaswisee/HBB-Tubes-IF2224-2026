# HBB Tubes IF2224 2026

## Deskripsi Program

Program ini merupakan proyek compiler bahasa Arion yang dibagi menjadi lima tahap: Lexical Analysis, Syntax Analysis, Semantic Analysis, Intermediate Code Generation, dan Interpreter. Milestone 1 berfokus pada implementasi Lexical Analysis (lexer) untuk membaca source code Arion dan menghasilkan token stream. Milestone 2 berfokus pada implementasi Syntax Analysis (parser) menggunakan algoritma Recursive Descent untuk memeriksa urutan token dan membangun Parse Tree.

Program bekerja dalam dua fase:

1. **Lexical Analysis**: Scanner membaca source code dan menghasilkan daftar token.
2. **Syntax Analysis**: Parser menerima daftar token dan membangun Parse Tree menggunakan Recursive Descent, lalu mencetak tree ke terminal dan menyimpannya ke file teks di `test/output/`.

## Requirements

- g++ dengan dukungan C++20
- make

## Cara Instalasi dan Penggunaan Program

1. Clone repository.
2. Masuk ke folder proyek.
3. Build program:

```bash
make all
```

1. Jalankan program dengan file input dari folder test/input:

```bash
make run FILE=input1.txt
```

1. Alternatif menjalankan executable langsung:

```bash
./bin/compiler input1.txt
```

1. Bersihkan build artifacts:

```bash
make clean
```

Catatan: nilai FILE harus merupakan nama file yang ada di folder test/input.

## Driver untuk Testing per Milestone

Program menyediakan driver terpisah untuk menguji setiap milestone secara individual:

### Build Driver

```bash
make drivers
```

### Menjalankan Driver

**M1 - Lexical Analysis only:**

```bash
./bin/driver_m1 <filename>
# atau
make test-m1 FILE=edge_keywords_case.txt
```

Output: Daftar token dari source code.

**M2 - Syntax Analysis:**

```bash
./bin/driver_m2 <filename>
# atau
make test-m2 FILE=1.txt
```

Output: Parse tree dari token stream.

**M3 - Semantic Analysis:**

```bash
./bin/driver_m3 <filename>
# atau
make test-m3 FILE=pos_basic_assign.txt
```

Output: AST, Symbol Tables, dan hasil analisis semantik.

### Lokasi Test File

- M1: `test/milestone1/input/`
- M2: `test/milestone2/input/`
- M3: `test/milestone3/input/`

## Identitas Kelompok

Nama Kelompok: GTFOTBFO (HBB)

- Nathanael Imandatua Manurung - 13524021
- Nicholas Wise Saragih Sumbayak - 13524037
- Kurt Mikhael Purba - 13524065
- Rava Khoman Tuah Saragih - 13524107
