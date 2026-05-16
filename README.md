# SSS-Tubes-IF2224-2026

## Identitas Kelompok
| Geraldo Artemius | 13524005 |
| Mikhael Andrian Yonatan | 13524051 |
| Junior Narta Situmorang | 13524055 |
| Reynard Nathanael | 13524103 |
| Nicholas Luis Chandra | 13524105 |

## Deskripsi Program
Program ini adalah program compiler yang dibangun menggunakan bahasa C++. Program akan membaca *source code* dari file input, melakukan proses leksikal untuk menghasilkan susunan token, melakukan *parsing* untuk menyusun *Parse Tree*, mengkonversi ke *Abstract Syntax Tree (AST)*, dan melakukan analisis semantik untuk mendeteksi *semantic error* sesuai aturan bahasa yang ditentukan.

Alur program secara umum:
1. Membaca file input *source code* (misalnya dari folder `test/milestone-3/`).
2. Memproses isi file menggunakan modul `Lexer` menjadi daftar *token*.
3. Memproses daftar *token* tersebut menggunakan modul `Parser` untuk membangun *Parse Tree*. Jika ada sintaks yang tidak sesuai, parser akan menuliskan letak pesan *syntax error*.
4. Mengkonversi *Parse Tree* menjadi *Abstract Syntax Tree (AST)* untuk representasi yang lebih ringkas.
5. Melakukan analisis semantik menggunakan modul `SemanticAnalyzer` untuk memeriksa kecocokan tipe, deklarasi variabel, penggunaan identifier yang terdefinisi, dan aturan semantik lainnya.
6. Menyimpan hasil run ke empat file output terpisah di dalam direktori yang sama dengan ekstensi:
   - File hasil leksikal: `<nama_file>-Result-Token.txt`
   - File hasil parsing: `<nama_file>-Result-Parse.txt`
   - File hasil AST: `<nama_file>-Result-AST.txt`
   - File hasil semantic: `<nama_file>-Result-Semantic.txt`

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

Setelah berhasil, executable bernama `semantic` akan terbentuk di root project.

### 3. Jalankan program

```bash
./semantic
```

Program akan meminta nama file, misalnya:

```text
Masukkan nama file (contoh: input.txt): testCase1.txt
```

Pastikan file input berada di folder:

```text
test/milestone-3/
```

Contoh output yang dihasilkan:

```text
File Token: test/milestone-3/testCase1-Result-Token.txt
File Parse: test/milestone-3/testCase1-Result-Parse.txt
File AST: test/milestone-3/testCase1-Result-AST.txt
File Semantic: test/milestone-3/testCase1-Result-Semantic.txt
```

### 4. Membersihkan hasil build

```bash
make clean
```

## Predefined Identifier

Program mendukung predefined identifier berikut yang sudah terdefinisi dalam symbol table global:

### Tipe Data (Type)
| Identifier | Deskripsi |
|---|---|
| `integer` | Tipe bilangan bulat |
| `real` | Tipe bilangan desimal |
| `char` | Tipe karakter tunggal |
| `boolean` | Tipe logika (true/false) |
| `string` | Tipe string/teks |

### Konstanta (Constant)
| Identifier | Tipe | Deskripsi |
|---|---|---|
| `true` | boolean | Nilai kebenaran benar |
| `false` | boolean | Nilai kebenaran salah |

### Procedure (I/O)
| Identifier | Deskripsi |
|---|---|
| `writeln` | Menulis output ke layar dengan newline |
| `write` | Menulis output ke layar tanpa newline |
| `readln` | Membaca input dari user dengan newline |
| `read` | Membaca input dari user tanpa newline |

### Function (Utility)
| Identifier | Return Type | Deskripsi |
|---|---|---|
| `abs` | integer/real | Nilai absolut (mutlak) |
| `sqrt` | real | Akar kuadrat |
| `length` | integer | Panjang string |

Contoh penggunaan:
```pascal
program Example;
var
  x: integer;
  s: string;
begin
  writeln('Masukkan bilangan: ');
  readln(x);
  writeln('Nilai absolut: ', abs(x));
  s := 'hello';
  writeln('Panjang string: ', length(s));
end.
```

## Pembagian Tugas

| Nama | NIM | Bagian | Persentase |
| --- | --- | --- | --- |
| Geraldo Artemius | 13524005 | Parse Type, Parse Statement, Laporan | 20% |
| Mikhael Andrian Yonatan | 13524051 | Error Handling, ParseTree, Laporan | 20% |
| Junior Narta Situmorang | 13524055 | Parse Statement, Expression, Laporan  | 20% |
| Reynard Nathanael | 13524103 | Parse Statement, Laporan | 20% |
| Nicholas Luis Chandra | 13524105 | Parse Declaration, Laporan | 20% |