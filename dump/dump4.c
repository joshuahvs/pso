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
    int n;

    printf("Masukkan jumlah command (max 10): ");
    scanf("%d", &n);

    if (n > MAX_COMMANDS) {
        printf("Jumlah command tidak boleh lebih dari 10\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        printf("Command ke-%d: ", i + 1);
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

    // Print the initial (parent) PID
    pid_t initial_pid = getpid();
    printf("Initial (Parent) PID = %d\n", initial_pid);

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();

        if (pid == 0) {  // Child process
            // Execute the current command
            printf("-----------------------\n");
            printf("Eksekusi command=%s, pid = %d\n", commands[i], getpid());
            sleep(2);  // Simulate some work
            sprintf((char *)shared_mem, "Child PID: %d, Command: %s", getpid(), commands[i]);

            // Execute the command
            execlp(commands[i], commands[i], NULL);
            perror("execlp");  // If execlp fails, this prints an error
            exit(EXIT_FAILURE);
        } else if (pid > 0) {  // Parent process
            wait(NULL);  // Wait for the current child to finish before starting the next
        } else {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // Only the initial parent reaches here
    if (getpid() == initial_pid) {
        printf("\n-----------------------\n");
        printf("Eksekusi command selesai\n");
        printf("Bersih bersih memory...\n");
        // Unmap and unlink shared memory
        munmap(shared_mem, 4096);
        shm_unlink(SHM_NAME);
        printf("Proses terakhir (parent, pid=%d) permisi keluar...\n", getpid());
    }

    return 0;
}
