# Sisop-2-2025-IT02

# Soal Pertama "action.c"

Repository ini berisi source code program `action.c`, sebuah program monitoring yang dibuat dengan bahasa C. Program ini memiliki beberapa command utama untuk mengelola proses milik user pada sistem Linux.

## Fitur Utama
- ./action
    - Download Clues.zip, extract isinya, lalu hapus file zip-nya.
- ./action -m Filter
    - Filter file .txt valid dari ClueA–D ke folder Filtered.
- ./action -m Combine
    - Gabungkan isi file di Filtered secara berurutan (1 a 2 b ...) ke Combined.txt.
- ./action -m Decode
    - Decode isi Combined.txt menggunakan ROT13 → hasilnya disimpan ke Decoded.txt.

## Penjelasan Kode
- <stdio.h> – Untuk input/output seperti printf() dan scanf().
- <stdlib.h> – Untuk fungsi umum seperti malloc(), free(), exit(), dan atoi().
- <string.h> – Untuk manipulasi string (strlen(), strcmp(), dll).
- <sys/stat.h> – Untuk mendapatkan informasi file atau direktori (stat()).
- <dirent.h> – Untuk membaca isi direktori dengan opendir(), readdir().
- <ctype.h> – Untuk pemeriksaan/konversi karakter (isdigit(), tolower(), dll).
- unistd.h> – Untuk fungsi sistem seperti fork(), exec(), dan chdir().
- <sys/wait.h> – Untuk menunggu proses anak selesai (wait(), waitpid()).

### 0. Run Command
```c
void run_command(char *argv[]) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
        perror("execvp gagal");
        exit(EXIT_FAILURE);
    } else {
        wait(NULL);
    }
}
```
- **Tujuan:** 
  Fungsi ini digunakan untuk menjalankan perintah terminal (command line) dari dalam program C. Dengan kata lain, fungsi ini membuat program kita bisa mengeksekusi perintah seperti ls, cp, 
  mv, dll, seolah-olah kita mengetikkannya langsung di terminal.

- **Cara Kerja:**
- Fungsi fork() digunakan untuk membuat proses anak.
- Jika berada di proses anak (pid == 0), program akan menjalankan perintah yang diberikan melalui execvp().
- Jika execvp() gagal, maka akan mencetak pesan error melalui perror() dan keluar dari proses.
- Jika berada di proses induk, maka wait(NULL) akan membuat program menunggu proses anak selesai sebelum melanjutkan.

### 1. Mengunduh, Mengekstrak, dan Menghapus file
```c
void download_clues() {
    struct stat st = {0};
    if (stat("Clues", &st) == 0) {
        printf("Folder 'Clues' sudah ada. Tidak download ulang.\n");
        return;
    }

    char *wget[] = {"wget", "--no-check-certificate",
        "https://drive.usercontent.google.com/u/0/uc?id=1xFn1OBJUuSdnApDseEczKhtNzyGekauK&export=download",
        "-O", "Clues.zip", NULL};
    char *unzip[] = {"unzip", "Clues.zip", NULL};
    char *rm[] = {"rm", "Clues.zip", NULL};

    run_command(wget);
    run_command(unzip);
    run_command(rm);

    printf("Clues.zip selesai di-download, diekstrak, dan dihapus.\n");
}
```
**Tujuan:** 
Fungsi ini digunakan untuk mengunduh file Clues.zip dari Google Drive, lalu mengekstraknya dan menghapus file zip-nya setelah selesai. Fungsi ini juga memastikan bahwa folder Clues tidak dibuat dua kali.

**Cara Kerja:**
- Mengecek apakah folder Clues sudah ada menggunakan stat().
- Jika folder sudah ada, maka proses download tidak dilakukan, dan program menampilkan pesan bahwa folder sudah tersedia.
- Jika folder belum ada, program akan menjalankan tiga perintah secara berurutan:
- wget: Mengunduh file Clues.zip dari URL Google Drive.
- unzip: Mengekstrak isi dari Clues.zip.
- rm: Menghapus file Clues.zip setelah diekstrak.
- Ketiga perintah ini dijalankan menggunakan fungsi helper run_command() yang memanfaatkan fork() dan execvp().

### 2. Menyaring file .txt tertentu dari 4 folder (Filtered)
```c
void filter_files() {
    const char *dirs[] = {"Clues/ClueA", "Clues/ClueB", "Clues/ClueC", "Clues/ClueD"};
    mkdir("Filtered", 0755);

    for (int i = 0; i < 4; i++) {
        DIR *dir = opendir(dirs[i]);
        if (!dir) continue;

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG && strlen(entry->d_name) == 5 && strstr(entry->d_name, ".txt")) {
                if (isalnum(entry->d_name[0]) && entry->d_name[1] == '.') {
                   char src[256], dst[256];
                   snprintf(src, sizeof(src), "%s/%s", dirs[i], entry->d_name);
                   snprintf(dst, sizeof(dst), "Filtered/%s", entry->d_name);
                   char *cp[] = {"cp", src, dst, NULL};
                   run_command(cp);
                }
            }
        }
        closedir(dir);
    }

    printf("Filtering selesai. File valid dipindah ke 'Filtered'.\n");
}
```
**Tujuan:** 
Fungsi ini bertugas untuk menyaring file .txt tertentu dari 4 folder petunjuk (ClueA, ClueB, ClueC, ClueD) dan memindahkannya ke folder baru bernama Filtered. Hanya file dengan nama yang sesuai kriteria tertentu yang akan dipindahkan.

**Cara Kerja:**
- Membuat folder Filtered sebagai tempat hasil penyaringan.
- Mengecek masing-masing dari 4 folder (ClueA sampai ClueD) di dalam folder Clues.
- Membaca semua file dalam folder tersebut dan menyaring file dengan kriteria berikut:
     - Nama file berjumlah 5 karakter.
     - Nama file berformat <karakter>.<ext> (contoh: A.txt, 9.txt).
     - Karakter pertama adalah alfanumerik (huruf atau angka).
     - Ekstensi file adalah .txt.
- File yang lolos kriteria akan disalin ke folder Filtered menggunakan command cp yang dijalankan melalui fungsi run_command().

### 3. menggabungkan isi dari file-file teks yang ada di FIltered(Combine.txt)
```c
int compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void combine_files() {
    DIR *dir = opendir("Filtered");
    if (!dir) {
        perror("Gagal buka folder Filtered");
        return;
    }

    FILE *output = fopen("Combined.txt", "w");
    if (!output) {
        perror("Gagal buat Combined.txt");
        closedir(dir);
        return;
    }

    char *angka[100], *huruf[100];
    int ca = 0, ch = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".txt")) {
            if (isdigit(entry->d_name[0])) angka[ca++] = strdup(entry->d_name);
            else if (isalpha(entry->d_name[0])) huruf[ch++] = strdup(entry->d_name);
        }
    }
    closedir(dir);

    qsort(angka, ca, sizeof(char *), compare);
    qsort(huruf, ch, sizeof(char *), compare);

    int i = 0, j = 0;
    while (i < ca || j < ch) {
        for (int pass = 0; pass < 2; pass++) {
            if ((pass == 0 && i < ca) || (pass == 1 && j < ch)) {
                char *fname = (pass == 0) ? angka[i++] : huruf[j++];
                char path[256];
                snprintf(path, sizeof(path), "Filtered/%s", fname);
                FILE *f = fopen(path, "r");
                if (f) {
                    char c;
                    while ((c = fgetc(f)) != EOF) fputc(c, output);
                    fclose(f);
                }
                char *rm[] = {"rm", path, NULL};
                run_command(rm);
                free(fname);
            }
        }
    }

    fclose(output);
    printf("Gabungan selesai. File Combined.txt berhasil dibuat.\n");
}
```
**Tujuan:** Fungsi ini digunakan untuk menggabungkan isi dari file-file teks (dengan nama diawali angka dan huruf) yang ada di folder Filtered, mengurutkannya, dan kemudian menyimpannya dalam satu file bernama Combined.txt. Setelah file gabungan selesai dibuat, file-file sumber akan dihapus.

**Fungsi compare:** Fungsi compare() adalah fungsi pembantu yang digunakan oleh qsort() untuk mengurutkan nama file secara alfabetis. Fungsi ini membandingkan dua string (nama file) dengan menggunakan strcmp().

**Cara Kerja:**
- Buka Folder Filtered: Fungsi dimulai dengan membuka folder Filtered yang berisi file-file teks hasil penyaringan. Jika folder tidak dapat dibuka, fungsi akan menampilkan pesan error dan keluar.
- Buat File Combined.txt: File Combined.txt akan dibuat untuk menampung isi dari semua file teks yang ditemukan dalam folder Filtered. Jika file gagal dibuat, maka folder akan ditutup dan fungsi dihentikan.
- Identifikasi File yang Akan Digabung: Fungsi ini kemudian membaca seluruh file di folder Filtered. File yang diperiksa harus berformat .txt. File-file ini kemudian dipisahkan ke dalam dua kategori:
    - Angka: File dengan nama yang dimulai dengan angka.
    - Huruf: File dengan nama yang dimulai dengan huruf.
- Urutkan File: Setelah file-file dipisahkan, kedua kategori (angka dan huruf) diurutkan secara alfabetis menggunakan fungsi qsort() dan compare() untuk membantu proses pengurutan.
- Gabungkan Isi File: Setelah pengurutan, file-file yang ada di kategori angka dan huruf digabungkan ke dalam Combined.txt. File pertama yang dimasukkan adalah file dengan nama yang dimulai dengan angka, lalu diikuti dengan file yang dimulai dengan huruf.
    - Setiap file dibuka dan isinya dibaca menggunakan fgetc(), kemudian ditulis ke Combined.txt                  menggunakan fputc().
    - Setelah selesai menyalin isi file, file tersebut akan dihapus dengan menjalankan perintah rm.
- Tutup File dan Folder: Setelah semua file digabungkan, file Combined.txt ditutup dan pesan berhasil ditampilkan.

### 4. mendekripsi isi file Combined.txt yang sebelumnya dienkripsi menggunakan metode ROT13 (Decode)
```c
void decode_rot13() {
    FILE *in = fopen("Combined.txt", "r");
    FILE *out = fopen("Decoded.txt", "w");
    if (!in || !out) {
        perror("Gagal buka Combined.txt atau Decoded.txt");
        return;
    }

    char c;
    while ((c = fgetc(in)) != EOF) {
        if (isalpha(c)) {
            char base = isupper(c) ? 'A' : 'a';
            c = ((c - base + 13) % 26) + base;
        }
        fputc(c, out);
    }

    fclose(in);
    fclose(out);
    printf("Decoded.txt berhasil dibuat dari Combined.txt dengan ROT13.\n");
}
```
**Tujuan:** 
Fungsi ini digunakan untuk mendekripsi file Combined.txt yang sebelumnya dienkripsi menggunakan metode ROT13, lalu hasil dekripsi disimpan ke file baru bernama Decoded.txt.

**Cara Kerja:**
- Fungsi membuka dua file:
    - Combined.txt → sebagai input (file terenkripsi)
    - Decoded.txt → sebagai output (hasil dekripsi)
- Membaca Karakter Satu per Satu
    - Fungsi membaca isi dari Combined.txt per karakter menggunakan metode pembacaan karakter tunggal.
- Melakukan ROT13 (Dekripsi)
    - Jika karakter adalah huruf (A–Z atau a–z), maka dilakukan pergeseran 13 huruf ke belakang dalam             alfabet.
    - ROT13 bersifat simetris, jadi jika dienkripsi dua kali hasilnya kembali ke semula.
- Menulis ke File Output
    - Karakter yang sudah didekripsi langsung ditulis ke Decoded.txt.
- Menutup File
    - Setelah proses selesai, kedua file ditutup dan program memberi pesan bahwa Decoded.txt telah berhasil 
      dibuat.

### 5. Menjalankan argumen dari terminal.
```c
int main(int argc, char *argv[]) {
    if (argc == 1) {
        download_clues();
    } else if (argc == 3 && strcmp(argv[1], "-m") == 0) {
        if (strcmp(argv[2], "Filter") == 0) filter_files();
        else if (strcmp(argv[2], "Combine") == 0) combine_files();
        else if (strcmp(argv[2], "Decode") == 0) decode_rot13();
        else printf("Mode tidak dikenal: %s\n", argv[2]);
    } else {
        printf("Penggunaan:\n");
        printf("  ./action               => Download Clues.zip\n");
        printf("  ./action -m Filter     => Filter file ke folder Filtered\n");
        printf("  ./action -m Combine    => Gabung isi file ke Combined.txt\n");
        printf("  ./action -m Decode     => Decode ROT13 ke Decoded.txt\n");
    }
    return 0;
}
```
**Tujuan:** 
Fungsi main() ini menjadi titik awal eksekusi program dan berfungsi untuk menangani argumen dari terminal. Tergantung dari argumen yang diberikan, program akan menjalankan fungsi tertentu (download_clues, filter_files, combine_files, atau decode_rot13).

**Cara Kerja:**
- Jika Tidak Ada Argumen
    - Saat program dijalankan tanpa argumen (argc == 1), maka fungsi download_clues() akan langsung               dijalankan.
    - Artinya: program akan mengunduh dan mengekstrak Clues.zip.
- Jika Ada Argumen -m dan Nama Mode
    - Jika ada 2 argumen tambahan, dan argumen pertama adalah -m, maka argumen kedua akan menentukan mode         eksekusi:
         - Filter → menjalankan filter_files(), menyaring file yang valid ke folder Filtered.
         - Combine → menjalankan combine_files(), menggabungkan isi file menjadi Combined.txt.
         - Decode → menjalankan decode_rot13(), mendekripsi file Combined.txt menjadi Decoded.txt.
    - Jika mode tidak dikenali, maka akan muncul pesan error.
- Jika Format Argumen Salah
    - Jika argumen tidak sesuai format yang dikenali, maka akan ditampilkan panduan penggunaan program agar       user tahu cara pakainya.

### 6. Memasukkan password
- Memasukkan password yang sudah didapatkan pada link yang tertera pada soal

# Soal Keempat "Debugmon"

Repository ini berisi source code program `debugmon.c`, sebuah program monitoring yang dibuat dengan bahasa C. Program ini memiliki beberapa command utama untuk mengelola proses milik user pada sistem Linux.

## Fitur Utama

Program ini mendukung beberapa command, antara lain:
- **list `<user>`**: Menampilkan daftar proses milik user yang bersangkutan (PID, command, CPU usage, dan memory usage).
- **daemon `<user>`**: Menjalankan proses monitoring secara daemon (latar belakang) dan mencatat hasil monitoring ke file log.
- **stop `<user>`**: Menghentikan daemon monitoring yang sedang berjalan dengan mengambil file PID.
- **fail `<user>`**: Menggagalkan (terminate) semua proses milik user tersebut dan mencatat status sebagai FAILED ke file log.
- **revert `<user>`**: Memulihkan status user sehingga dapat kembali menjalankan proses normal dan mencatat status revert pada file log.

## Penjelasan Kode

Kode `debugmon.c` dibangun menggunakan beberapa library standar C:
- **stdio.h**: Untuk fungsi input/output seperti `printf`, `fprintf`, `fopen`, dan `fclose`.
- **stdlib.h**: Untuk fungsi seperti `exit` dan konversi data.
- **string.h**: Untuk perbandingan dan manipulasi string (misalnya `strcmp` dan `snprintf`).
- **unistd.h**: Untuk melakukan operasi system-level seperti `fork()`, `execlp()`, `dup2()`, `chdir()`, dan `close()`.
- **sys/types.h** dan **sys/wait.h**: Untuk fungsi `wait()`, yang digunakan agar proses induk menunggu proses anak selesai.
- **signal.h**: Untuk mengirim sinyal pada proses, misalnya `SIGTERM` dan `SIGKILL` melalui fungsi `kill()`.
- **time.h**: Untuk mengambil waktu saat ini menggunakan fungsi `time()`, `localtime()`, dan `ctime()` untuk tujuan pencatatan log.
- **fcntl.h** dan **sys/stat.h**: Untuk pengaturan file dan permission, seperti fungsi `umask()`.

Berikut adalah penjelasan per bagian untuk masing-masing command:

### 0. Conditioning 

```c
int main(int argc, char *argv[]) {
if (argc != 3) {
        printf("Usage: %s <command> <user>\n", argv[0]);
        return 1;
    }

    char *command = argv[1];
    char *user = argv[2];
```

- **Tujuan:**  
  Meminta pengguna untuk menginput 3 argumen pada saat ingin melakukan command, apabila lebih atau kurang dari 3, maka terdapat output yang akan memandu pengguna.
(./debugmon -command- -user-)
  
### 1. Command `list <user>`

```c
// COMMAND LIST <USER>
if (strcmp(command, "list") == 0) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("Gagal membuat proses");
            return 1;
        }

        if (pid == 0) {
            execlp("ps", "ps", "-u", user, "-o", "pid,comm,%cpu,%mem", NULL);
            perror("Gagal menjalankan perintah ps");
            exit(1);
        } else {
            wait(NULL);
        }
```

- **Tujuan:**  
  Menampilkan daftar semua proses milik user tertentu dengan informasi PID, command, penggunaan CPU, dan penggunaan memori.
- **Cara Kerja:**  
  Program memanggil `fork()` untuk membuat proses anak. Pada proses anak, `execlp("ps", ...)` digunakan untuk menjalankan perintah `ps -u <user> -o pid,comm,%cpu,%mem` yang menghasilkan daftar proses. Proses induk menunggu hingga proses anak selesai dengan `wait(NULL)`.
- **Library Terkait:**  
  **stdio.h**, **unistd.h**, **sys/wait.h**, dan **string.h**.

### 2. Command `daemon <user>`

```c
// COMMAND DAEMON <USER>
} else if (strcmp(command, "daemon") == 0) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork gagal");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);
    setsid();
    chdir("/");

    // Simpan PID ke file
    char pidfile[256];
    snprintf(pidfile, sizeof(pidfile), "/tmp/debugmon_%s.pid", user);
    FILE *pidf = fopen(pidfile, "w");
    if (pidf != NULL) {
        fprintf(pidf, "%d", getpid());
        fclose(pidf);
    }

    // Jangan tutup file descriptor sebelum logging
    // Tambahan pencatatan log untuk DAEMON (log awal DAEMON START)
    FILE *log = fopen("/tmp/debugmon.log", "a");  // gunakan /tmp/debugmon.log
    time_t now = time(NULL);
    if (log) {
        struct tm *t = localtime(&now);
        fprintf(log, "[%02d:%02d:%04d]-%02d:%02d:%02d_daemon-%s_RUNNING\n",
                t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
                t->tm_hour, t->tm_min, t->tm_sec,
                user);
        fclose(log);
    }

    // Baru tutup STD
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Loop monitoring
    while (1) {
        FILE *fp = fopen("/tmp/debugmon.log", "a");
        if (fp == NULL) {
            exit(1);
        }

        time_t now = time(NULL);
        fprintf(fp, "\n=== Monitoring at %s", ctime(&now));

        pid_t child = fork();

        if (child == 0) {
            dup2(fileno(fp), STDOUT_FILENO); // Output langsung ke file log
            execlp("ps", "ps", "-u", user, "-o", "pid,comm,%cpu,%mem", NULL);
            exit(1);
        } else {
            wait(NULL);
        }

        fclose(fp);
        sleep(10);
    }
```

- **Tujuan:**  
  Menjalankan monitoring proses secara terus-menerus (daemon) di latar belakang. Hasil monitoring ditulis ke file log `/tmp/debugmon.log`. PID daemon disimpan di file `/tmp/debugmon_<user>.pid`.
- **Cara Kerja:**  
  - Proses utama melakukan `fork()` untuk membuat daemon. Jika `pid > 0` (proses induk), maka keluar dengan `exit(EXIT_SUCCESS)`.
  - Proses anak yang akan menjadi daemon melakukan pengaturan standar daemon: memanggil `umask(0)`, `setsid()`, dan `chdir("/")` untuk memutus keterikatan dengan terminal.
  - PID dari daemon disimpan ke file agar bisa digunakan untuk menghentikan daemon kemudian.
  - Sebelum menutup file descriptor standar, daemon menuliskan log awal ke `/tmp/debugmon.log` bahwa daemon telah dijalankan dengan status _RUNNING_.
  - Daemon kemudian menutup STDIN, STDOUT, dan STDERR agar tidak terikat ke terminal, lalu masuk ke loop monitoring yang setiap 10 detik menjalankan perintah `ps` (dengan bantuan proses anak lagi) dan menuliskan hasilnya ke log.
- **Library Terkait:**  
  **unistd.h**, **sys/stat.h**, **time.h**, **stdio.h**, dan **string.h**.

### 3. Command `stop <user>`

```c
// COMMAND STOP <USER> (MENGHENTIKAN DAEMON YANG TELAH DIJALANKAN)
} else if (strcmp(command, "stop") == 0) {
        char pidfile[256];
        snprintf(pidfile, sizeof(pidfile), "/tmp/debugmon_%s.pid", user);

        FILE *fp = fopen(pidfile, "r");
        if (fp == NULL) {
            printf("Tidak dapat menemukan file PID untuk user %s\n", user);
            return 1;
        }

        pid_t pid;
        fscanf(fp, "%d", &pid);
        fclose(fp);

        if (kill(pid, SIGTERM) == 0) {
            printf("Proses monitoring user %s berhasil dihentikan (PID: %d)\n", user, pid);
            remove(pidfile);

            // Tambahan pencatatan log untuk STOP
            FILE *log = fopen("debugmon.log", "a");
            time_t now = time(NULL);
            if (log) {
                fprintf(log, "[%02d:%02d:%04d]-%02d:%02d:%02d_stop-RUNNING\n",
                        localtime(&now)->tm_mday,
                        localtime(&now)->tm_mon + 1,
                        localtime(&now)->tm_year + 1900,
                        localtime(&now)->tm_hour,
                        localtime(&now)->tm_min,
                        localtime(&now)->tm_sec);
                fclose(log);
            }

        } else {
            perror("Gagal menghentikan proses");
            return 1;
        }
```

- **Tujuan:**  
  Menghentikan daemon monitoring yang sedang berjalan.
- **Cara Kerja:**  
  - Program membaca file PID (`/tmp/debugmon_<user>.pid`) untuk mendapatkan ID proses daemon.
  - Kemudian mengirim sinyal `SIGTERM` menggunakan fungsi `kill()` untuk menghentikan proses daemon.
  - Setelah berhasil, file PID dihapus dan log pencatatan untuk command _stop_ dituliskan ke file log.
- **Library Terkait:**  
  **unistd.h**, **signal.h**, **sys/wait.h**, dan **stdio.h**.

### 4. Command `fail <user>`

```c
// COMMAND FAIL <USER> (MENGGAGALKAN SEMUA PROSES YANG SEDANG BERJALAN DI USER)
} else if (strcmp(command, "fail") == 0) {
        char cmd[256];
        FILE *fp;
        FILE *log;
        char line[64];
        time_t now = time(NULL);

        snprintf(cmd, sizeof(cmd), "ps -u %s -o pid= --no-headers", argv[2]);
        fp = popen(cmd, "r");
        if (fp == NULL) {
            perror("popen");
            return 1;
        }

        log = fopen("debugmon.log", "a");
        if (log == NULL) {
            perror("fopen");
            pclose(fp);
            return 1;
        }

        while (fgets(line, sizeof(line), fp)) {
            int pid = atoi(line);
            if (pid > 0) {
                if (kill(pid, SIGKILL) == 0) {
                    fprintf(log, "[%02d:%02d:%04d]-%02d:%02d:%02d_%d-FAILED\n",
                            localtime(&now)->tm_mday,
                            localtime(&now)->tm_mon + 1,
                            localtime(&now)->tm_year + 1900,
                            localtime(&now)->tm_hour,
                            localtime(&now)->tm_min,
                            localtime(&now)->tm_sec, pid);
                }
            }
        }

        fclose(log);
        pclose(fp);
        printf("Semua proses milik user %s telah dihentikan dan dicatat di debugmon.log\n", argv[2]);
```

- **Tujuan:**  
  Menggagalkan semua proses yang sedang berjalan milik user yang bersangkutan dan mencatat setiap status proses yang dibunuh (FAILED) ke file log.
- **Cara Kerja:**  
  - Program menggunakan `popen()` untuk menjalankan perintah `ps -u <user> -o pid=` yang menghasilkan daftar PID dari proses milik user.
  - Setiap PID yang didapat kemudian dikonversi ke integer dan diberi perintah `kill(pid, SIGKILL)`. Jika berhasil, status _FAILED_ beserta PID dicatat ke log.
- **Library Terkait:**  
  **stdio.h** (untuk `popen()` dan `fgets()`), **stdlib.h**, dan **signal.h**.

### 5. Command `revert <user>`

```c
// COMMAND REVERT <USER> (MEMBALIKKAN PROSES YANG DIGAGALKAN OLEH COMMAND FAIL)
} else if (strcmp(argv[1], "revert") == 0) {
        FILE *log;
        time_t now = time(NULL);

        log = fopen("debugmon.log", "a");
        if (log == NULL) {
            perror("fopen");
            return 1;
        }

        fprintf(log, "[%02d:%02d:%04d]-%02d:%02d:%02d_revert-RUNNING\n",
                localtime(&now)->tm_mday,
                localtime(&now)->tm_mon + 1,
                localtime(&now)->tm_year + 1900,
                localtime(&now)->tm_hour,
                localtime(&now)->tm_min,
                localtime(&now)->tm_sec);
        fclose(log);

        printf("User %s telah dipulihkan dan sekarang dapat menjalankan proses seperti biasa.\n", argv[2]);
```

- **Tujuan:**  
  Mengembalikan status user agar dapat menjalankan proses kembali setelah sebelumnya di-_fail_. Log revert dituliskan ke file log.
- **Cara Kerja:**  
  Hanya menuliskan log bahwa user telah direvert dan statusnya kembali normal. Jika ingin memberikan fungsi tambahan seperti mengubah shell user atau hak akses, hal itu dapat ditambahkan.
- **Library Terkait:**  
  **stdio.h**, **time.h**, dan **string.h**.

## Kesimpulan

Program `debugmon.c` merupakan contoh pengaplikasian konsep *process* dan *daemon* di C menggunakan library dasar seperti **stdio.h**, **string.h**, **time.h**, dan **unistd.h**. Dengan:
- **fork()** untuk membuat proses baru
- **execlp()** untuk menjalankan perintah eksternal (misalnya, `ps`)
- **setsid(), chdir()**, dan **close()** untuk memisahkan proses daemon dari terminal
- **popen()** untuk membaca output perintah shell
- Serta fungsi-fungsi logging dengan **fprintf()** dan fungsi waktu dari **time.h**

Semua konsep tersebut diaplikasikan untuk membuat program yang mampu melakukan monitoring proses, menghentikan daemon, menggagalkan proses user, dan memulihkan status user secara terprogram.

---
