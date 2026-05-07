# SSS-Tubes-IF2224-2026

## Identitas Kelompok
| Geraldo Artemius | 13524005 |
| Mikhael Andrian Yonatan | 13524051 |
| Junior Narta Situmorang | 13524055 |
| Reynard Nathanael | 13524103 |
| Nicholas Luis Chandra | 13524105 |

## Deskripsi Program
## Deskripsi Program
Program ini adalah program compiler Lexical Analyzer dan Syntax Analyzer/Parser yang dibangun menggunakan bahasa C++. Program akan membaca *source code* dari file input, melakukan proses leksikal untuk menghasilkan susunan token, lalu melakukan *parsing* untuk menyusun *Parse Tree* serta mendeteksi *syntax error* sesuai aturan tata bahasa yang ditentukan.

Alur program secara umum:
1. Membaca file input *source code* (misalnya dari folder `test/milestone-2/`).
2. Memproses isi file menggunakan modul `Lexer` menjadi daftar *token*.
3. Memproses daftar *token* tersebut menggunakan modul `Parser` untuk membangun *Parse Tree*. Jika ada sintaks yang tidak sesuai, parser akan menuliskan letak pesan *syntax error*.
4. Menyimpan hasil run ke dua file output terpisah di dalam direktori yang sama dengan ekstensi:
   - File hasil leksikal: `<nama_file>-Result-Token.txt`
   - File hasil parsing: `<nama_file>-Result-Parse.txt`

## Requirements
- Sistem operasi yang mendukung compiler C++ (Linux/macOS/Windows).
- `g++` dengan dukungan standar C++17.
- `make`.

## Cara Instalasi dan Penggunaan Program
### 1. Clone repository

```bash
git clone https://github.com/staplesmaster/SSS-Tubes-IF2224-2026.git
cd SSS-Tubes-IF2224-2026
```

### 2. Compile program

```bash
make
```

Setelah berhasil, executable bernama `parser` akan terbentuk di root project.

### 3. Jalankan program

```bash
./parser
```

Program akan meminta nama file, misalnya:

```text
Masukkan nama file (contoh: input.txt): test1.txt
```

Pastikan file input berada di folder:

```text
test/milestone-2/
```

Contoh output yang dihasilkan:

```text
test/milestone-2/test1-Result.txt
```

### 4. Membersihkan hasil build

```bash
make clean
```

## Pembagian Tugas

| Nama | NIM | Bagian | Persentase |
| --- | --- | --- | --- |
| Geraldo Artemius | 13524005 | Parse Type, Parse Statement, Laporan | 20% |
| Mikhael Andrian Yonatan | 13524051 | Error Handling, ParseTree, Laporan | 20% |
| Junior Narta Situmorang | 13524055 | Parse Statement, Expression, Laporan  | 20% |
| Reynard Nathanael | 13524103 | Parse Statement, Laporan | 20% |
| Nicholas Luis Chandra | 13524105 | Parse Declaration, Laporan | 20% |