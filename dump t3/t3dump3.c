#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_ITEM_NAME_LENGTH 20

// Semaphores for synchronization
sem_t sem_order, sem_supply, sem_available, sem_mutex, sem_stock;

// Shared resources
int *warehouse;
int warehouse_size;
int item_counter = 0;         // Number of items currently in the warehouse
int item_counter_total = 0;   // Total number of items supplied
int buyers_active;
int finished_buyers = 0;

// Struct to hold buyer data
typedef struct {
    int id;
    int items_to_buy;
    int items_bought;
    int *items_list; // Dynamic array to store items bought by the buyer
} Buyer;

// Function prototypes
void *supplier_thread(void *arg);
void *seller_thread(void *arg);
void *buyer_thread(void *arg);

pthread_mutex_t buyer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t buy_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex to ensure only one buyer buys at a time

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <warehouse size 1..10> <number of buyers 1..50> <items per buyer 1..100>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Parse input arguments
    warehouse_size = atoi(argv[1]);
    buyers_active = atoi(argv[2]);
    int items_per_buyer = atoi(argv[3]);

    // Validate input
    if (warehouse_size < 1 || warehouse_size > 10 || buyers_active < 1 || buyers_active > 50 || items_per_buyer < 1 || items_per_buyer > 100) {
        fprintf(stderr, "Error: Input out of range. Ensure 1 <= n <= 10, 1 <= k <= 50, and 1 <= m <= 100.\n");
        return EXIT_FAILURE;
    }

    // Initialize warehouse
    warehouse = (int *)calloc(warehouse_size, sizeof(int));

    // Initialize semaphores
    sem_init(&sem_order, 0, 0);
    sem_init(&sem_supply, 0, 0);
    sem_init(&sem_available, 0, 0);
    sem_init(&sem_mutex, 0, 1);
    sem_init(&sem_stock, 0, 0);

    // Create threads
    pthread_t supplier, seller;
    pthread_t *buyers = (pthread_t *)malloc(buyers_active * sizeof(pthread_t));
    Buyer *buyer_data = (Buyer *)malloc(buyers_active * sizeof(Buyer));

    // Start supplier and seller threads
    printf("Membuat thread Supplier...\n");
    pthread_create(&supplier, NULL, supplier_thread, NULL);
    printf("Membuat thread Seller...\n");
    pthread_create(&seller, NULL, seller_thread, NULL);

    // Start buyer threads
    printf("Membuat %d thread Buyer...\n\n", buyers_active);
    for (int i = 0; i < buyers_active; i++) {
        buyer_data[i].id = i + 1;
        buyer_data[i].items_to_buy = items_per_buyer;
        buyer_data[i].items_bought = 0;
        buyer_data[i].items_list = (int *)malloc(buyer_data[i].items_to_buy * sizeof(int)); // Allocate memory for items list
        pthread_create(&buyers[i], NULL, buyer_thread, &buyer_data[i]);
    }

    // Join buyer threads
    for (int i = 0; i < buyers_active; i++) {
        pthread_join(buyers[i], NULL);
    }

    // Cancel supplier and seller threads
    pthread_cancel(supplier);
    pthread_cancel(seller);
    pthread_join(supplier, NULL);
    pthread_join(seller, NULL);

    // Print summary of items bought by each buyer
    printf("Daftar Belanja Buyer\n");
    printf("========================\n");
    for (int i = 0; i < buyers_active; i++) {
        printf("Buyer-%d\n", buyer_data[i].id);
        for (int j = 0; j < buyer_data[i].items_bought; j++) {
            printf("%d. item - %d\n", j + 1, buyer_data[i].items_list[j]);
        }
        free(buyer_data[i].items_list); // Free allocated memory for items list
    }

    // Cleanup
    free(warehouse);
    free(buyers);
    free(buyer_data);

    // Destroy semaphores
    sem_destroy(&sem_order);
    sem_destroy(&sem_supply);
    sem_destroy(&sem_available);
    sem_destroy(&sem_mutex);
    sem_destroy(&sem_stock);

    printf("\nProgram completed successfully.\n");
    return EXIT_SUCCESS;
}

void *supplier_thread(void *arg) {
    while (1) {
        sem_wait(&sem_supply); // Wait for supply order from seller

        // Check if all buyers are finished
        pthread_mutex_lock(&buyer_mutex);
        if (finished_buyers >= buyers_active) {
            pthread_mutex_unlock(&buyer_mutex);
            break;
        }
        pthread_mutex_unlock(&buyer_mutex);

        sleep(1); // Simulate delivery time
        sem_wait(&sem_mutex);
        for (int i = 0; i < warehouse_size; i++) {
            warehouse[item_counter++] = ++item_counter_total;
            printf("Supplier: memasok item - %d ke gudang seller\n", item_counter_total);
        }
        sem_post(&sem_mutex);
        sem_post(&sem_stock); // Notify seller that items are supplied
        // Notify buyers that items are available
        for (int i = 0; i < warehouse_size; i++) {
            sem_post(&sem_available);
        }
    }
    printf("Supplier TERMINATE\n");
    pthread_exit(NULL);
    return NULL;
}

void *seller_thread(void *arg) {
    // At the beginning, order items from supplier
    printf("Seller: order barang ke supplier\n");
    sem_post(&sem_supply);

    while (1) {
        sem_wait(&sem_stock); // Wait for supplier to supply items

        // Notify buyers that items are available
        printf("Seller: barang TERSEDIA, silahkan dibeli\n");

        // Wait for order from buyer
        sem_wait(&sem_order);

        // Check if all buyers are finished
        pthread_mutex_lock(&buyer_mutex);
        if (finished_buyers >= buyers_active) {
            pthread_mutex_unlock(&buyer_mutex);
            break;
        }
        pthread_mutex_unlock(&buyer_mutex);

        printf("Seller: stok barang HABIS, order dulu ke supplier\n");
        sem_post(&sem_supply); // Notify supplier
    }
    printf("Seller TERMINATE\n");
    pthread_exit(NULL);
    return NULL;
}

void *buyer_thread(void *arg) {
    Buyer *buyer = (Buyer *)arg;

    while (buyer->items_bought < buyer->items_to_buy) {
        // Ensure only one buyer buys at a time
        pthread_mutex_lock(&buy_mutex);

        sem_wait(&sem_mutex); // Lock warehouse access
        if (item_counter > 0) {
            int item_number = warehouse[--item_counter]; // Get the item number
            printf("Buyer-%d: membeli item - %d\n", buyer->id, item_number);
            buyer->items_list[buyer->items_bought++] = item_number; // Store the item in buyer's list
            sem_post(&sem_mutex); // Unlock warehouse
            pthread_mutex_unlock(&buy_mutex); // Unlock the buy_mutex
            sleep(1); // Simulate buying time
        } else {
            sem_post(&sem_mutex); // Unlock warehouse
            pthread_mutex_unlock(&buy_mutex); // Unlock the buy_mutex
            sem_post(&sem_order); // Notify seller to reorder
            sem_wait(&sem_available); // Wait for items to be available
            continue; // Try purchasing again
        }

        // Check if buyer has finished buying
        if (buyer->items_bought >= buyer->items_to_buy) {
            pthread_mutex_lock(&buyer_mutex);
            finished_buyers++;
            printf("Buyer-%d: SELESAI BELANJA (TERMINATE)\n\n", buyer->id);
            pthread_mutex_unlock(&buyer_mutex);
            break;
        }
    }

    pthread_exit(NULL);
    return NULL;
}
