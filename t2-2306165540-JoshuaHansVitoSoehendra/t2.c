#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#define SHM_NAME "/mySharedMemory" 
#define MAX_COMMANDS 10 

// NAMA: Joshua Hans Vito Soehendra
// NPM: 2306165540
// Kelas: PSO A

int main() {
    int n;
    pid_t *pids;
    // Meminta input jumlah command dari pengguna dengan validasi menggunakan while loop
    while (1) {
        printf("Jumlah command (max=10) = ");
        if (scanf("%d", &n) != 1) {  // Cek apakah input adalah bilangan bulat
            printf("Input harus dalam bentuk bilangan\n");
            printf("\n");
            while (getchar() != '\n');  // Mengosongkan input buffer
        } else if (n <= 0) {  // Validasi jumlah command
            printf("Jumlah command harus lebih besar dari 0\n");
            printf("\n");
        } else if (n>MAX_COMMANDS){
            printf("Jumlah command tidak bisa melebih 10\n");
            printf("\n");
        } else {
            break;  // Keluar dari loop jika input valid
        }
    }

    // Alokasi memori dinamis untuk menyimpan nama-nama command menggunakan malloc
    char **commands = (char **)malloc(n * sizeof(char *));
    if (commands == NULL) {
        perror("malloc gagal");
        exit(EXIT_FAILURE);
    }

    // Alokasi memori untuk setiap string command
    for (int i = 0; i < n; i++) {
        commands[i] = (char *)malloc(100 * sizeof(char));
        if (commands[i] == NULL) {
            perror("malloc gagal untuk command");
            exit(EXIT_FAILURE);
        }
    }

    // Alokasi memori untuk menyimpan PID dari setiap proses
    pids = (pid_t *)malloc(n * sizeof(pid_t));
    if (pids == NULL) {
        perror("malloc gagal untuk PIDs");
        exit(EXIT_FAILURE);
    }

    // Meminta input nama-nama command dari pengguna
    for (int i = 0; i < n; i++) {
        printf("Command ke-%d: ", i + 1);
        scanf("%s", commands[i]);  // Menyimpan input command ke dalam array
    }

    // Membuat shared memory
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Mengatur ukuran shared memory
    ftruncate(shm_fd, 4096);

    // Melakukan mapping shared memory
    void *shared_mem = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Menampilkan PID awal (parent process)
    pid_t initial_pid = getpid();
    printf("\n");
    printf("Initial (Parent) PID = %d\n", initial_pid);

    // Menjalankan setiap command menggunakan fork dan execlp
    for (int i = 0; i < n; i++) {
        pids[i] = fork();  // Membuat proses baru
        if (pids[i] == 0) {  // Proses anak
            sleep((n - i) * 2);  // Penundaan untuk output yang lebih teratur
            printf("\n------------------------\n\n");
            printf("Eksekusi command=%s, pid = %d\n", commands[i], getpid());

            // Menyimpan informasi ke dalam shared memory
            sprintf((char *)shared_mem, "Child PID: %d, Command: %s\n", getpid(), commands[i]);

            execlp(commands[i], commands[i], NULL);  // Menjalankan command
            perror("execlp gagal");  // Menampilkan pesan error jika execlp gagal
            exit(EXIT_FAILURE);  // Keluar dari proses anak jika terjadi kesalahan
        }
        if (pids[i] < 0) {  // Menangani kesalahan fork
            perror("Fork gagal");
            exit(EXIT_FAILURE);
        }
    }

    // Proses parent menunggu semua proses anak selesai
    for (int i = 0; i < n; i++) {
        waitpid(pids[i], NULL, 0);  // Menunggu proses anak dengan PID yang sesuai
    }

    // Menampilkan pesan setelah semua eksekusi selesai
    printf("\n\n------------------------\n");
    printf("Eksekusi command selesai\n");
    printf("Bersih-bersih memory...\n");
    printf("\n");

    // Membebaskan memori yang dialokasikan untuk nama-nama command
    for (int i = 0; i < n; i++) {
        free(commands[i]);
    }
    // Membebaskan array commands dan pids
    free(commands); 
    free(pids);

    // Unmap dan unlink shared memory
    munmap(shared_mem, 4096);
    shm_unlink(SHM_NAME);

    printf("Proses terakhir (parent, pid=%d) permisi keluar...\n", getpid());
    return 0;
}