#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
if (argc != 3) {
        printf("Usage: %s <command> <user>\n", argv[0]);
        return 1;
    }

    char *command = argv[1];
    char *user = argv[2];

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

// OUTPUT KETIKA COMMAND TIDAK ADA ATAU TIDAK DIKENALI SELAIN KODE DIATAS
} else {
        printf("Command tidak dikenal: %s\n", command);
}

    return 0;
}
