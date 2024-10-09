#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#define MAX_COMMANDS 10
#define MAX_LENGTH 100
#define SHM_NAME "/mySharedMemory"

int main() {
    char commands[MAX_COMMANDS][MAX_LENGTH];
    pid_t pids[MAX_COMMANDS];
    int n;

    printf("Masukkan jumlah command (max 10): ");
    scanf("%d", &n);

    if (n > MAX_COMMANDS) {
        printf("Jumlah command tidak boleh lebih dari 10\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        printf("Command ke-%d: ", i+1);
        scanf("%s", commands[i]);
    }

    // Membuat shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Mengatur ukuran shared memory
    ftruncate(shm_fd, 4096);

    // Map shared memory
    void *shared_mem = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    } 

    for (int i = 0; i < n; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork failed");
            exit(1);
        }
        if (pids[i] == 0) { // child process
            exit(0);
        }
    }

    // Print the initial (parent) PID
    pid_t initial_pid = getpid();
    printf("Initial (Parent) PID = %d\n", initial_pid);


    // Eksekusi setiap command dengan fork dan execlp
    for (int i = n - 1; i >= 0; i--) {
        printf("-------------------------------------------------------------\n");
        printf("\n");
        pid_t pid = fork();
        if (pid == 0) {  // Child process
            // Child process prints its PID
            printf("Eksekusi command=%s, pid = %d\n", commands[i], pids[i]);

            // Simpan informasi di shared memory
            sprintf((char *)shared_mem, "Child PID: %d, Command: %s\n", pids[i], commands[i]);

            execlp(commands[i], commands[i], NULL);  // Menjalankan command
            perror("execlp");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {  // Parent process
            wait(NULL);  // Wait for child to finish
            sleep(2);
        } else {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        printf("\n");
    }

    printf("Eksekusi command selesai\n");
    printf("Bersih bersih memory...\n");
    // Hapus shared memory setelah selesai
    munmap(shared_mem, 4096);
    shm_unlink(SHM_NAME);

    printf("Proses terakhir (parent, pid=%d) permisi keluar...\n", getpid());

    return 0;
}
