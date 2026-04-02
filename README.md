# HBB Tubes IF2224 2026

## Deskripsi Program

Program ini merupakan proyek compiler bahasa Arion yang dibagi menjadi lima tahap: Lexical Analysis, Syntax Analysis, Semantic Analysis, Intermediate Code Generation, dan Interpreter. Milestone 1 berfokus pada implementasi Lexical Analysis (lexer) untuk membaca source code Arion dan menghasilkan token stream.

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

## Identitas Kelompok

Nama Kelompok: GTFOTBFO (HBB)

- Nathanael Imandatua Manurung - 13524021
- Nicholas Wise Saragih Sumbayak - 13524037
- Kurt Mikhael Purba - 13524065
- Rava Khoman Tuah Saragih - 13524107
