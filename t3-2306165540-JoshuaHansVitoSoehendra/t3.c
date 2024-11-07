// Nama: Joshua Hans Vito Soehendra
// NPM: 2306165540
// Link video: https://youtu.be/GLhIawLJINg

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <ctype.h>

// Semaphore untuk sinkronisasi
sem_t sem_order, sem_supply, sem_available, sem_mutex, sem_stock;

// Sumber daya bersama
int *warehouse;
int warehouse_size;
int item_counter = 0;         // Jumlah item saat ini di gudang
int item_counter_total = 0;   // Total jumlah item yang telah disuplai
int buyers_active;
int buyers_remaining;         // Jumlah pembeli yang masih perlu membeli
int finished_buyers = 0;

// Mutex
pthread_mutex_t buyer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t buy_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex untuk memastikan hanya satu pembeli yang membeli pada satu waktu
pthread_mutex_t warehouse_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex untuk warehouse_empty_flag

// Flag untuk menunjukkan apakah gudang kosong
int warehouse_empty_flag = 0;

// Struct untuk menyimpan data pembeli
typedef struct {
    int id;
    int items_to_buy;
    int items_bought;
    int *items_list; // Array dinamis untuk menyimpan item yang dibeli oleh pembeli
} Buyer;

// Inisiasi nama fungsi
void *supplier_thread(void *arg);
void *seller_thread(void *arg);
void *buyer_thread(void *arg);


// Fungsi untuk memeriksa apakah string hanya berisi angka
int is_numeric(const char *str) {
    if (str == NULL || *str == '\0') {
        return 0; // String kosong atau NULL bukan numerik
    }
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit((unsigned char)str[i])) {
            return 0; // Karakter bukan digit
        }
    }
    return 1; // Semua karakter adalah digit
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Penggunaan: %s <besar gudang 1..10> <jumlah buyer 1..50> <jumlah item pembelian @buyer 1..100>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Validasi numerik untuk setiap argumen
    for (int i = 1; i < argc; i++) {
        if (!is_numeric(argv[i])) {
            printf("Seluruh parameter harus dalam numerik\n");
            return EXIT_FAILURE;
        }
    }

    // Memparsing argumen input
    warehouse_size = atoi(argv[1]);
    buyers_active = atoi(argv[2]);
    buyers_remaining = buyers_active;
    int items_per_buyer = atoi(argv[3]);

    // Validasi input
    if (warehouse_size < 1 || warehouse_size > 10 || buyers_active < 1 || buyers_active > 50 || items_per_buyer < 1 || items_per_buyer > 100) {
        printf("Besar gudang harus antara 1 hingga 10\n");
        printf("Jumlah buyer harus antara 1 hingga 50\n");
        printf("Jumlah item pembelian @buyer harus antara 1 hingga 100\n");
        return EXIT_FAILURE;
    }

    // Inisialisasi gudang
    warehouse = (int *)calloc(warehouse_size, sizeof(int));

    // Inisialisasi semaphore
    sem_init(&sem_order, 0, 0);
    sem_init(&sem_supply, 0, 0);
    sem_init(&sem_available, 0, 0); // Inisialisasi ke 0
    sem_init(&sem_mutex, 0, 1);
    sem_init(&sem_stock, 0, 0);

    // Membuat thread
    pthread_t supplier, seller;
    pthread_t *buyers = (pthread_t *)malloc(buyers_active * sizeof(pthread_t));
    Buyer *buyer_data = (Buyer *)malloc(buyers_active * sizeof(Buyer));

    // Memulai thread supplier dan seller
    printf("Membuat thread Supplier...\n");
    pthread_create(&supplier, NULL, supplier_thread, NULL);
    printf("Membuat thread Seller...\n");
    pthread_create(&seller, NULL, seller_thread, NULL);

    // Memulai %d thread pembeli
    printf("Membuat %d thread Buyer...\n", buyers_active);
    for (int i = 0; i < buyers_active; i++) {
        buyer_data[i].id = i + 1;
        buyer_data[i].items_to_buy = items_per_buyer;
        buyer_data[i].items_bought = 0;
        buyer_data[i].items_list = (int *)malloc(buyer_data[i].items_to_buy * sizeof(int)); // Alokasi memori untuk daftar item
        pthread_create(&buyers[i], NULL, buyer_thread, &buyer_data[i]);
    }

    // Menunggu thread pembeli selesai
    for (int i = 0; i < buyers_active; i++) {
        pthread_join(buyers[i], NULL);
    }

    // Menunggu thread supplier dan seller selesai
    pthread_join(seller, NULL);
    pthread_join(supplier, NULL);

    // Cetak ringkasan item yang dibeli oleh setiap pembeli
    printf("\nDaftar Belanja Buyer\n");
    printf("========================");
    for (int i = 0; i < buyers_active; i++) {
        printf("\nBuyer-%d\n", buyer_data[i].id);
        for (int j = 0; j < buyer_data[i].items_bought; j++) {
            printf("%d. item - %d\n", j + 1, buyer_data[i].items_list[j]);
        }
        free(buyer_data[i].items_list); // Bebaskan memori yang dialokasikan untuk daftar item
    }

    // Pembersihan
    free(warehouse);
    free(buyers);
    free(buyer_data);

    // Hancurkan semaphore
    sem_destroy(&sem_order);
    sem_destroy(&sem_supply);
    sem_destroy(&sem_available);
    sem_destroy(&sem_mutex);
    sem_destroy(&sem_stock);

    return EXIT_SUCCESS;
}

void *supplier_thread(void *arg) {
    while (1) {
        sem_wait(&sem_supply); // Menunggu pesanan suplai dari seller

        // Periksa apakah semua pembeli telah selesai
        pthread_mutex_lock(&buyer_mutex);
        if (buyers_remaining <= 0) {
            pthread_mutex_unlock(&buyer_mutex);
            break;
        }
        pthread_mutex_unlock(&buyer_mutex);

        sleep(1); // Simulasikan waktu pengiriman

        sem_wait(&sem_mutex); // Kunci akses ke gudang
        for (int i = 0; i < warehouse_size; i++) {
            warehouse[item_counter++] = ++item_counter_total;
            printf("Supplier: memasok item - %d ke gudang seller\n", item_counter_total);
        }
        sem_post(&sem_mutex);
        sem_post(&sem_stock); // Beri tahu seller bahwa item telah disuplai
    }
    printf("Supplier TERMINATE\n");
    pthread_exit(NULL);
    return NULL;
}

void *seller_thread(void *arg) {
    // Di awal, pesan item dari supplier
    printf("\nSeller: order barang ke supplier\n\n");
    sem_post(&sem_supply);

    while (1) {
        sem_wait(&sem_stock); // Tunggu supplier untuk menyuplai item

        // Periksa apakah semua pembeli telah selesai
        pthread_mutex_lock(&buyer_mutex);
        if (buyers_remaining <= 0) {
            pthread_mutex_unlock(&buyer_mutex);
            break;
        }
        pthread_mutex_unlock(&buyer_mutex);

        // Reset warehouse_empty_flag
        pthread_mutex_lock(&warehouse_mutex);
        warehouse_empty_flag = 0;
        pthread_mutex_unlock(&warehouse_mutex);

        // Beri tahu pembeli bahwa item tersedia
        printf("\nSeller: barang TERSEDIA, silahkan dibeli\n\n");

        pthread_mutex_lock(&buyer_mutex);
        int current_buyers = buyers_remaining;
        pthread_mutex_unlock(&buyer_mutex);

        // Sinyalkan pembeli bahwa item tersedia
        for (int i = 0; i < current_buyers; i++) {
            sem_post(&sem_available);
        }

        // Tunggu pesanan dari pembeli yang menunjukkan stok habis
        sem_wait(&sem_order);

        if (buyers_remaining != 0) {
            printf("\nSeller: stok barang HABIS, order dulu ke supplier\n\n");
        }

        // Periksa apakah semua pembeli telah selesai
        pthread_mutex_lock(&buyer_mutex);
        if (buyers_remaining <= 0) {
            pthread_mutex_unlock(&buyer_mutex);
            break;
        }
        pthread_mutex_unlock(&buyer_mutex);

        sem_post(&sem_supply); // Beri tahu supplier
    }
    printf("Seller TERMINATE\n");
    pthread_exit(NULL);
    return NULL;
}

void *buyer_thread(void *arg) {
    Buyer *buyer = (Buyer *)arg;

    // Tunggu item tersedia awalnya
    sem_wait(&sem_available);

    while (buyer->items_bought < buyer->items_to_buy) {
        // Pastikan hanya satu pembeli yang membeli pada satu waktu
        pthread_mutex_lock(&buy_mutex);

        sem_wait(&sem_mutex); // Kunci akses ke gudang

        // Pembeli membeli item sampai gudang kosong atau pembeli telah membeli semua item yang diperlukan
        while (buyer->items_bought < buyer->items_to_buy && item_counter > 0) {
            int item_number = warehouse[--item_counter]; // Dapatkan nomor item
            printf("Buyer-%d: membeli item - %d\n", buyer->id, item_number);
            buyer->items_list[buyer->items_bought++] = item_number; // Simpan item di daftar pembeli
        }

        // Periksa apakah gudang kosong
        int warehouse_empty = (item_counter == 0);

        sem_post(&sem_mutex); // Buka kunci gudang

        if (warehouse_empty) {
            pthread_mutex_lock(&warehouse_mutex);
            if (warehouse_empty_flag == 0) {
                warehouse_empty_flag = 1;
                sem_post(&sem_order); // Beri tahu seller untuk memesan ulang
            }
            pthread_mutex_unlock(&warehouse_mutex);
        }

        pthread_mutex_unlock(&buy_mutex); // Buka kunci buy_mutex

        // Jika pembeli telah membeli semua item yang dibutuhkan, keluar dari loop
        if (buyer->items_bought >= buyer->items_to_buy) {
            break;
        }

        // Tunggu pemberitahuan bahwa item tersedia
        sem_wait(&sem_available);
    }

    // Pembeli telah selesai membeli
    pthread_mutex_lock(&buyer_mutex);
    buyers_remaining--;
    printf("\nBuyer-%d: SELESAI BELANJA (TERMINATE)\n\n", buyer->id);

    // Jika ini adalah pembeli terakhir, sinyalkan semaphore untuk membuka blokir Seller dan Supplier
    if (buyers_remaining == 0) {
        sem_post(&sem_order);
        sem_post(&sem_stock);
        sem_post(&sem_supply);
        // Sinyalkan pembeli yang menunggu di sem_available
        sem_post(&sem_available);
    }
    pthread_mutex_unlock(&buyer_mutex);

    pthread_exit(NULL);
    return NULL;
}
