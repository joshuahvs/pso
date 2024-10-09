#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>

#define SHM_NAME "/mySharedMemory"

int main() {
    int num_commands;
    char commands[10][50]; // Adjust size as needed
    pid_t pids[10];

    printf("Enter the number of commands (max 10): ");
    scanf("%d", &num_commands);

    for (int i = 0; i < num_commands; i++) {
        printf("Enter command %d: ", i + 1);
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

    // Creating child processes
    for (int i = 0; i < num_commands; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            // Child process code
            // Synchronization using shared memory
            // Wait for your turn
            // ...

            // Optional 2-second delay
            sleep(2);

            // Replace process image with the command
            execlp(commands[i], commands[i], NULL);

            // If execlp fails
            perror("execlp failed");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            pids[i] = pid; // Store child's PID
        }
    }

    // Parent process prints PIDs in decreasing order
    printf("Child PIDs in decreasing order:\n");
    for (int i = num_commands - 1; i >= 0; i--) {
        printf("%d\n", pids[i]);
    }

    // Wait for all child processes to finish
    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }

    // Cleanup shared memory
    // ...
    printf("Eksekusi command selesai\n");
    printf("Bersih bersih memory...\n");
    // Hapus shared memory setelah selesai
    munmap(shared_mem, 4096);
    shm_unlink(SHM_NAME);

    printf("Proses terakhir (parent, pid=%d) permisi keluar...\n", getpid());

    return 0;
}
