# SSS-Tubes-IF2224-2026

## Identitas Kelompok
| Geraldo Artemius | 13524005 |
| Mikhael Andrian Yonatan | 13524051 |
| Junior Narta Situmorang | 13524055 |
| Reynard Nathanael | 13524103 |
| Nicholas Luis Chandra | 13524105 |

## Deskripsi Program
Program ini adalah lexical analyzer dengan bahasa C++ untuk membaca source code dari file input, melakukan tokenisasi, lalu menuliskan hasil token ke file output.

Alur program secara umum:
1. Membaca file input dari folder `test/milestone-1/`.
2. Memproses isi file menggunakan modul `Lexer` menjadi daftar token.
3. Menyimpan hasil token ke file output dengan format `<nama_file>-Result.<ekstensi>` di folder yang sama.

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

Setelah berhasil, executable bernama `lexer` akan terbentuk di root project.

### 3. Jalankan program

```bash
./lexer
```

Program akan meminta nama file, misalnya:

```text
Masukkan nama file (contoh: input.txt): test1.txt
```

Pastikan file input berada di folder:

```text
test/milestone-1/
```

Contoh output yang dihasilkan:

```text
test/milestone-1/test1-Result.txt
```

### 4. Membersihkan hasil build

```bash
make clean
```

## Pembagian Tugas

| Nama | NIM | Bagian | Persentase |
| --- | --- | --- | --- |
| Geraldo Artemius | 13524005 | Kode Program, Diagram DFA, Laporan | 20% |
| Mikhael Andrian Yonatan | 13524051 | Kode Program, Laporan | 20% |
| Junior Narta Situmorang | 13524055 | Kode Program, Laporan  | 20% |
| Reynard Nathanael | 13524103 | Kode Program, Diagram DFA, Laporan | 20% |
| Nicholas Luis Chandra | 13524105 | Kode Program, Laporan | 20% |