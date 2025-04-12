# Sisop-2-2025-IT02

# Soal Pertama "action.c"

Repository ini berisi source code program `action.c`, sebuah program monitoring yang dibuat dengan bahasa C. Program ini memiliki beberapa command utama untuk mengelola proses milik user pada sistem Linux.

## Fitur Utama

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
