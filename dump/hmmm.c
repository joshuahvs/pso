#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>

#define max_command 10 // maksimal command

int main() {
    int jumlah_command;
    char *command[max_command]; // simpen command
    pid_t pids[max_command]; // simpen PID
    
    // input jumlah command
    while (1) {
        printf("Jumlah command (max=10) = ");

        // validasi input
        if (scanf("%d", &jumlah_command) != 1) { 
            printf("Input harus dalam bentuk bilangan\n");
            printf("\n");
            while (getchar() != '\n');
            continue;
        }
    
        if (jumlah_command > max_command) {
            printf("Jumlah command tidak bisa melebihi 10\n");
            printf("\n");
        } else if (jumlah_command <= 0) {
            printf("Jumlah command harus lebih besar dari 0\n");
            printf("\n");
        } else {
            break; 
        }
    }
    
    // input command
    for (int i = 0; i < jumlah_command; i++) {
        command[i] = (char *)malloc(100 * sizeof(char)); // alokasi memori dinamis
        printf("Command ke-%d = ", i + 1);
        scanf("%s", command[i]);
    }
    
    for (int i = 0; i < jumlah_command; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork failed");
            exit(1);
        }
        if (pids[i] == 0) { // child process
            exit(0);
        }
    }

    printf("\n");
    printf("Initial (Parent) pid = %d\n", getpid());
    printf("--------------------------------\n");
    
    // menjalankan command dari yang terakhir
    for (int i = jumlah_command - 1; i >= 0; i--) {
        pid_t pid = fork();
        
        if (pid == 0) { 
            printf("\n");
            printf("Eksekusi command=%s, pid = %d\n", command[i], pids[i]);
            execlp(command[i], command[i], NULL); // menjalankan command
            perror("execlp gagal");
            exit(1);
        }
        else if (pid > 0) {
            // menunggu child selesai
            wait(NULL);
            sleep(2); 
            printf("\n");
            printf("--------------------------------\n");
        }
        else {
            perror("Fork gagal");
            exit(1);
        }
    }
    
    // setelah semua command selesai
    printf("\n");
    printf("Eksekusi command selesai\n");
    printf("Bersih bersih memory...\n");
    
    for (int i = 0; i < jumlah_command; i++) {
        free(command[i]);
    }

    printf("\n");
    printf("Proses terakhir (parent, pid=%d) permisi keluar..\n", getpid());
    printf("\n");
    return 0;
}