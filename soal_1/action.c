#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

// Helper: Jalankan command pakai execvp
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

// Download dan ekstrak Clues.zip
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

// Filter file yang hanya 1 karakter namanya
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

// Sort helper
int compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// Gabungkan isi file angka dan huruf
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

// Decode ROT13
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

// MAIN
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
    return 0;
}
