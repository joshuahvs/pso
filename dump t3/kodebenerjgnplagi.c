#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

// Semaphore variables
sem_t NotifSeller;
sem_t EmptyWarehouse;
sem_t FullWarehouse;
sem_t StartBuy;

// Global variables
int n, k, m;  
int in = 0;
int out = 0;
int stock = 0;
int item_indexing = 0;
int consumer_to_be_serve;

// Dynamic arrays
int *storage;
int **transaction_history;

// Mutex
pthread_mutex_t mutex;

// Declare function
void *seller(void *);
void *buyer(void *);
void *supplier(void *);

int is_numeric(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Penggunaan: %s <besar gudang 1..10> <jumlah buyer 1..50> <jumlah item pembelian @buyer 1..100>\n", argv[0]);
        return -1;
    }
    for (int i = 1; i < 4; i++) {
        if (!is_numeric(argv[i])) {
            printf("Seluruh parameter harus dalam numerik\n");
            return -1;
        }
    }

    n = atoi(argv[1]);
    k = atoi(argv[2]);
    m = atoi(argv[3]);

    int error = 0;
    if (n < 1 || n > 10) {
        printf("Besar gudang harus antara 1 hingga 10\n");
        error = 1;
    }
    if (k < 1 || k > 50) {
        printf("Jumlah buyer harus antara 1 hingga 50\n");
        error = 1;
    }
    if (m < 1 || m > 100) {
        printf("Jumlah item per buyer harus antara 1 hingga 100\n");
        error = 1;
    }
    if (error) {
        return -1;
    }

    consumer_to_be_serve = k;

    storage = (int *)malloc(n * sizeof(int));
    transaction_history = (int **)malloc(k * sizeof(int *));
    for (int i = 0; i < k; i++) {
        transaction_history[i] = (int *)malloc(m * sizeof(int));
    }

    pthread_t cus[k], slr, sup;
    pthread_mutex_init(&mutex, NULL);

    sem_init(&NotifSeller, 0, 0);
    sem_init(&EmptyWarehouse, 0, 0);
    sem_init(&FullWarehouse, 0, 0);
    sem_init(&StartBuy, 0, 1);

    printf("Membuat thread Supplier...\n");
    printf("Membuat thread Seller...\n");
    printf("Membuat %d thread Buyer...\n", k);

    int indexing[k];
    for (int i = 0; i < k; i++) {
        indexing[i] = i + 1;
        pthread_create(&cus[i], NULL, buyer, &indexing[i]);
    }
    pthread_create(&slr, NULL, seller, NULL);
    pthread_create(&sup, NULL, supplier, NULL);

    for (int i = 0; i < k; i++) {
        pthread_join(cus[i], NULL);
    }
    pthread_join(slr, NULL);
    pthread_join(sup, NULL);

    printf("\nDaftar Belanja Buyer\n");
    printf("======================\n");
    for (int i = 0; i < k; i++) {
        printf("Buyer-%d\n", i + 1);
        for (int j = 0; j < m; j++) {
            printf("%d. item-%d\n", j + 1, transaction_history[i][j]);
        }
        printf("\n");
    }

    free(storage);
    for (int i = 0; i < k; i++) {
        free(transaction_history[i]);
    }
    free(transaction_history);

    pthread_mutex_destroy(&mutex);
    sem_destroy(&NotifSeller);
    sem_destroy(&EmptyWarehouse);
    sem_destroy(&FullWarehouse);
    sem_destroy(&StartBuy);

    return 0;
}

int isRestocking = 0;

void *seller(void *slr) {
    while (consumer_to_be_serve > 0) {
        sem_wait(&NotifSeller);
        if (consumer_to_be_serve == 0) break;

        if (stock == 0 && !isRestocking) {
            printf("\nSeller: stok barang HABIS, order ke supplier\n\n");
            isRestocking = 1;
            sem_post(&EmptyWarehouse);
        } else if (stock > 0) {
            printf("\nSeller: barang tersedia, silakan beli\n\n");
            sem_post(&FullWarehouse);
        }
    }
    printf("Seller TERMINATE\n");
}

void *supplier(void *sup) {
    while (consumer_to_be_serve > 0) {
        sem_wait(&EmptyWarehouse);
        if (consumer_to_be_serve == 0) break;

        sleep(1);
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < n; i++) {
            item_indexing++;
            storage[in] = item_indexing;
            in = (in + 1) % n;
            stock++;
            printf("Supplier: memasok item-%d ke gudang\n", item_indexing);
        }
        pthread_mutex_unlock(&mutex);

        isRestocking = 0;
        sem_post(&NotifSeller);
    }
    printf("Supplier TERMINATE\n");
}

void *buyer(void *cus) {
    int buyer_id = *((int *)cus);

    sem_wait(&StartBuy);
    for (int i = 0; i < m; i++) {
        if (stock == 0) {
            sem_post(&NotifSeller);
            sem_wait(&FullWarehouse);
        }

        pthread_mutex_lock(&mutex);
        int item = storage[out];
        printf("Buyer-%d: membeli item-%d\n", buyer_id, item);
        transaction_history[buyer_id - 1][i] = item;
        stock--;
        out = (out + 1) % n;
        pthread_mutex_unlock(&mutex);
    }

    printf("\nBuyer-%d: selesai belanja (TERMINATE)\n\n", buyer_id);

    sem_post(&StartBuy);
    consumer_to_be_serve--;

    if (consumer_to_be_serve == 0) {
        sem_post(&NotifSeller);
        sem_post(&EmptyWarehouse);
    }
}