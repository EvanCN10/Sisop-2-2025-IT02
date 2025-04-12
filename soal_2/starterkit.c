#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>

void ensure_directory(const char *dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        mkdir(dir, 0755);
    }
}

void download_and_extract() {
    char url[] = "https://drive.usercontent.google.com/u/0/uc?id=1_5GxIGfQr3mNKuavJbte_AoRkEQLXSKS&export=download";
    char zip_name[] = "starter_kit.zip";

    // Download ZIP file
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "wget --no-check-certificate '%s' -O '%s'", url, zip_name);
    system(cmd);

    // Extract ZIP
    snprintf(cmd, sizeof(cmd), "unzip -o '%s' -d starter_kit", zip_name);
    system(cmd);

    // Remove ZIP
    snprintf(cmd, sizeof(cmd), "rm '%s'", zip_name);
    system(cmd);

    printf("Starter kit berhasil didownload, diekstrak, dan file zip dihapus.\n");
}

void move_files(const char *source_dir, const char *target_dir) {
    ensure_directory(target_dir);

    DIR *dir = opendir(source_dir);
    if (!dir) {
        perror("Gagal membuka direktori");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char old_path[256], new_path[256];
            snprintf(old_path, sizeof(old_path), "%s/%s", source_dir, entry->d_name);
            snprintf(new_path, sizeof(new_path), "%s/%s", target_dir, entry->d_name);

            if (rename(old_path, new_path) != 0) {
                perror("Gagal memindahkan file");
            } else {
                printf("File '%s' dipindahkan dari '%s' ke '%s'.\n", entry->d_name, source_dir, target_dir);
            }
        }
    }
    closedir(dir);
}

void decrypt_filenames() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); // Parent exit

    umask(0);
    setsid();
    // Tetap di current working directory

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    ensure_directory("karantina/decrypted");

    while (1) {
        DIR *dir = opendir("karantina");
        if (!dir) {
            sleep(5);
            continue;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type != DT_REG) continue;

            char old_path[256], new_path[256], decoded[256];
            snprintf(old_path, sizeof(old_path), "karantina/%s", entry->d_name);

            char cmd[512];
            snprintf(cmd, sizeof(cmd), "echo %s | base64 -d", entry->d_name);
            FILE *fp = popen(cmd, "r");
            if (!fp) continue;

            if (fgets(decoded, sizeof(decoded), fp) != NULL) {
                decoded[strcspn(decoded, "\n")] = 0;  // Hapus newline
                snprintf(new_path, sizeof(new_path), "karantina/decrypted/%s", decoded);
                rename(old_path, new_path);
            }
            pclose(fp);
        }
        closedir(dir);
        sleep(10);
    }
}

int main(int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "--decrypt") == 0) {
        printf("Menjalankan daemon untuk mendekripsi file di folder karantina...\n");
        decrypt_filenames();
    } else if (argc == 2 && strcmp(argv[1], "--quarantine") == 0) {
        printf("Memindahkan file dari starter kit ke karantina...\n");
        ensure_directory("karantina");
        move_files("starter_kit", "karantina");
    } else if (argc == 2 && strcmp(argv[1], "--return") == 0) {
        printf("Memindahkan file dari karantina ke starter kit...\n");
        ensure_directory("starter_kit");
        move_files("karantina", "starter_kit");
    } else {
        download_and_extract();
    }

    return 0;
}
