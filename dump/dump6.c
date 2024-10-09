#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

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
        command[i] = (char *)malloc(100 * sizeof(char)); // array dinamik
        printf("Command ke-%d = ", i + 1);
        scanf("%s", command[i]);
    }
    
    printf("\n");
    printf("Initial (Parent) pid = %d\n", getpid());
    
    // fork untuk setiap command dari command terakhir
    for (int i = 0; i < jumlah_command; i++) {
        pids[i] = fork();
        
        if (pids[i] < 0) {
            perror("fork failed");
            exit(1);
        }
        
        if (pids[i] == 0) { // child process
            sleep((jumlah_command - i) * 2); 
            printf("\n--------------------------------\n");

            printf("\n");
            printf("Eksekusi command=%s, pid = %d\n", command[i], getpid());
            execlp(command[i], command[i], NULL); // menjalankan command
            printf("\n");

            // jika gagal, baru cetak pesan error
            perror("execlp gagal");
            exit(1);
        }
    }

    // parent process menunggu semua child process selesai
    for (int i = jumlah_command - 1; i >= 0; i--) {
        waitpid(pids[i], NULL, 0); // menunggu child berdasarkan PID
    }
    
    // setelah semua command selesai
    printf("\n");
    printf("--------------------------------\n");
    printf("\nEksekusi command selesai\n");
    printf("Bersih bersih memory...\n");
    
    for (int i = 0; i < jumlah_command; i++) {
        free(command[i]);
    }

    printf("\n");
    printf("Proses terakhir (parent, pid=%d) permisi keluar..\n", getpid());
    printf("\n");
    return 0;
}