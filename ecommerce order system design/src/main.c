#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "engine.h"

static void testSearch(ECommerceEngine* engine, const char* prefix) {
    printf("Searching prefix: \"%s\"\n", prefix);
    int count = 0;
    Product** results = engineSearchCatalog(engine, prefix, &count);
    if (!results || count == 0) {
        printf("   (No products found matching prefix)\n");
    } else {
        for (int i = 0; i < count; i++) {
            printf("   -> ");
            printProduct(results[i]);
        }
    }
    free(results);   /* searchTriePrefix now mallocs — always free */
    printf("\n");
}



static void runAutomatedDemo(ECommerceEngine* engine) {
    printf("\n================================================================================\n");
    printf("     RUNNING COMPREHENSIVE AUTOMATED VERIFICATION DEMONSTRATION\n");
    printf("================================================================================\n\n");

    /* -----------------------------------------------------------------------
     * Dynamically pick up to 6 real products from whatever catalog is loaded.
     * This makes the demo work whether the CSV or the default catalog is used.
     * ----------------------------------------------------------------------- */
    int catalogCount = 0;
    Product** catalog = engineGetAllProducts(engine, &catalogCount);
    if (!catalog || catalogCount < 2) {
        printf("[DEMO] Not enough products in catalog to run demo (need at least 2).\n");
        free(catalog);
        return;
    }
    /* pick evenly-spaced products so we get variety from large catalogs */
    int step = catalogCount / 6;
    if (step < 1) step = 1;
    Product* p[6];
    for (int i = 0; i < 6; i++) {
        int idx = i * step;
        if (idx >= catalogCount) idx = catalogCount - 1;
        p[i] = catalog[idx];
    }
    free(catalog);

    /* derive short search prefix from first product's name (first 3 letters) */
    char prefix1[8] = {0}, prefix2[8] = {0}, prefix3[8] = {0};
    if (p[0]->name[0]) { prefix1[0]=p[0]->name[0]; prefix1[1]=p[0]->name[1]; prefix1[2]=p[0]->name[2]; }
    if (p[2]->name[0]) { prefix2[0]=p[2]->name[0]; prefix2[1]=p[2]->name[1]; prefix2[2]=p[2]->name[2]; }
    if (p[4]->name[0]) { prefix3[0]=p[4]->name[0]; prefix3[1]=p[4]->name[1]; prefix3[2]=p[4]->name[2]; }

    /* -----------------------------------------------------------------------
     * 1. TRIE PREFIX SEARCH DEMO  (capped at 5 results shown per query)
     * ----------------------------------------------------------------------- */
    printf(">>> [1/3] DEMONSTRATING HIGH-SPEED PREFIX SEARCH (TRIE - O(L + K))\n");
    printf("--------------------------------------------------------------------------------\n");

    const char* prefixes[3] = {prefix1, prefix2, prefix3};
    for (int pi = 0; pi < 3; pi++) {
        int count = 0;
        Product** results = engineSearchCatalog(engine, prefixes[pi], &count);
        printf("Prefix search \"%s\" -> %d result(s)", prefixes[pi], count);
        if (count == 0) {
            printf(" (none)\n");
        } else {
            int show = count > 5 ? 5 : count;
            printf(" (showing first %d):\n", show);
            for (int i = 0; i < show; i++) {
                printf("   -> [%s | %s | $%.2f]\n",
                       results[i]->id, results[i]->name, results[i]->price);
            }
            if (count > 5) printf("   ... and %d more.\n", count - 5);
        }
        free(results);
        printf("\n");
    }

    printf("Direct O(1) Hash Table Lookup for '%s':\n", p[3]->id);
    Product* found = engineGetProduct(engine, p[3]->id);
    if (found) {
        printf("   Found: [%s | %s | $%.2f | Stock: %d]\n\n",
               found->id, found->name, found->price, found->stockQuantity);
    } else {
        printf("   (not found)\n\n");
    }

    /* -----------------------------------------------------------------------
     * 2. ORDER PLACEMENT IN SEQUENCE (FIFO QUEUE)
     * ----------------------------------------------------------------------- */
    printf(">>> [2/3] PLACING ORDERS IN CHRONOLOGICAL SEQUENCE (FIFO QUEUE)\n");
    printf("--------------------------------------------------------------------------------\n");

    OrderItem items1[] = { {"", "", 1, p[0]->price} };
    strncpy(items1[0].productId,   p[0]->id,   sizeof(items1[0].productId)   - 1);
    strncpy(items1[0].productName, p[0]->name, sizeof(items1[0].productName) - 1);
    Order* o1 = enginePlaceOrder(engine, "CUST-ALICE", items1, 1);
    printf("+ Order placed: %s for Alice  [%s x1 @ $%.2f]\n",
           o1->orderId, p[0]->name, p[0]->price);

    OrderItem items2[] = {
        {"", "", 2, p[1]->price},
        {"", "", 1, p[2]->price}
    };
    strncpy(items2[0].productId,   p[1]->id,   sizeof(items2[0].productId)   - 1);
    strncpy(items2[0].productName, p[1]->name, sizeof(items2[0].productName) - 1);
    strncpy(items2[1].productId,   p[2]->id,   sizeof(items2[1].productId)   - 1);
    strncpy(items2[1].productName, p[2]->name, sizeof(items2[1].productName) - 1);
    Order* o2 = enginePlaceOrder(engine, "CUST-BOB", items2, 2);
    printf("+ Order placed: %s for Bob    [%s x2 + %s x1]\n",
           o2->orderId, p[1]->name, p[2]->name);

    OrderItem items3[] = { {"", "", 1, p[3]->price} };
    strncpy(items3[0].productId,   p[3]->id,   sizeof(items3[0].productId)   - 1);
    strncpy(items3[0].productName, p[3]->name, sizeof(items3[0].productName) - 1);
    Order* o3 = enginePlaceOrder(engine, "CUST-CHARLIE", items3, 1);
    printf("+ Order placed: %s for Charlie[%s x1 @ $%.2f]\n",
           o3->orderId, p[3]->name, p[3]->price);

    OrderItem items4[] = { {"", "", 1, p[4]->price} };
    strncpy(items4[0].productId,   p[4]->id,   sizeof(items4[0].productId)   - 1);
    strncpy(items4[0].productName, p[4]->name, sizeof(items4[0].productName) - 1);
    Order* o4 = enginePlaceOrder(engine, "CUST-DIANA", items4, 1);
    printf("+ Order placed: %s for Diana  [%s x1 @ $%.2f]\n\n",
           o4->orderId, p[4]->name, p[4]->price);

    printOrderQueue(engine->orderQueue);
    printf("\n");

    /* -----------------------------------------------------------------------
     * 3. REVERSIBLE MODIFICATIONS & CANCELLATIONS (LIFO UNDO/REDO STACK)
     * ----------------------------------------------------------------------- */
    printf(">>> [3/3] DEMONSTRATING REVERSIBLE MODIFICATIONS & CANCELLATIONS (LIFO STACK)\n");
    printf("--------------------------------------------------------------------------------\n");

    printf("--- Event A: Bob edits Order %s (item[0] qty 2 -> 4) ---\n", o2->orderId);
    engineEditOrderItemQuantity(engine, o2->orderId, p[1]->id, 4);
    printOrder(findOrderInQueue(engine->orderQueue, o2->orderId));
    printf("\n");

    printf("--- Event B: Charlie cancels Order %s ---\n", o3->orderId);
    engineCancelOrder(engine, o3->orderId);
    printOrderQueue(engine->orderQueue);
    printf("\n");

    char msg[256];

    printf("--- Undo Step 1: Reversing Charlie's Cancellation ---\n");
    engineUndo(engine, msg, sizeof(msg));
    printf("Result: %s\n", msg);
    printOrderQueue(engine->orderQueue);
    printf("\n");

    printf("--- Undo Step 2: Reversing Bob's Quantity Edit ---\n");
    engineUndo(engine, msg, sizeof(msg));
    printf("Result: %s\n", msg);
    printOrder(findOrderInQueue(engine->orderQueue, o2->orderId));
    printf("\n");

    printf("--- Redo Step: Re-applying Bob's Quantity Edit ---\n");
    engineRedo(engine, msg, sizeof(msg));
    printf("Result: %s\n", msg);
    printOrder(findOrderInQueue(engine->orderQueue, o2->orderId));
    printf("\n");

    /* -----------------------------------------------------------------------
     * 4. SEQUENTIAL ORDER PROCESSING (FIFO DEQUEUE)
     * ----------------------------------------------------------------------- */
    printf("================================================================================\n");
    printf(">>> PROCESSING AND FULFILLING ALL ORDERS IN ARRIVAL SEQUENCE (FIFO)\n");
    printf("================================================================================\n");

    int procStep = 1;
    while (engine->orderQueue->size > 0) {
        Order* processed = engineProcessNextOrder(engine);
        printf("[%d] Processed: Order #%s | Customer: %s | Amount: $%.2f | Status: %s\n",
               procStep++, processed->orderId, processed->customerId,
               processed->totalAmount, orderStatusToString(processed->status));
        freeOrder(processed);
    }

    printf("\nFinal Queue Size: %d\n", engine->orderQueue->size);
    printf(">>> Automated verification demo completed successfully!\n\n");
}


static void populateDefaultCatalog(ECommerceEngine* engine) {
    engineAddProduct(engine, "PROD-01", "Apple iPhone 15 Pro", "Smartphones", 999.99, 50);
    engineAddProduct(engine, "PROD-02", "Apple MacBook Pro M3", "Laptops", 1999.99, 30);
    engineAddProduct(engine, "PROD-03", "Apple AirPods Max", "Audio", 549.00, 40);
    engineAddProduct(engine, "PROD-04", "Samsung Galaxy S24 Ultra", "Smartphones", 1199.99, 45);
    engineAddProduct(engine, "PROD-05", "Sony WH-1000XM5 Headphones", "Audio", 399.99, 60);
    engineAddProduct(engine, "PROD-06", "Sony PlayStation 5 Console", "Gaming", 499.99, 25);
    engineAddProduct(engine, "PROD-07", "Dell XPS 15 Laptop", "Laptops", 1499.00, 20);
}

static void showMenu(void) {
    printf("================================================================================\n");
    printf("                 E-COMMERCE DSA PLATFORM - MAIN MENU\n");
    printf("================================================================================\n");
    printf("  0. Exit\n");
    printf("  1. List All Available Products & Details\n");
    printf("  2. Search Products by Prefix\n");
    printf("  3. Lookup Product by ID/SKU\n");
    printf("  4. Place a New Customer Order\n");
    printf("  5. Edit Item Quantity in Pending Order\n");
    printf("  6. Cancel a Pending Order\n");
    printf("  7. Undo Last Action\n");
    printf("  8. Redo Last Action\n");
    printf("  9. Process Next Order in Sequence\n");
    printf(" 10. View Current FIFO Order Queue\n");
    printf(" 11. Run Full Automated Verification Demo\n");
    printf("--------------------------------------------------------------------------------\n");
    printf("Enter choice [0-11]: ");
}

int main(int argc, char* argv[]) {
    ECommerceEngine* engine = createEngine();

    /* -----------------------------------------------------------------------
     * Dataset loading: try products.csv first; fall back to hardcoded defaults.
     * The CSV file must have this header line:
     *   id,name,category,price,stock
     * Each subsequent line is one product record.
     * ----------------------------------------------------------------------- */
    const char* csvPath = "products.csv";
    int loaded = engineLoadCatalogFromCSV(engine, csvPath);
    if (loaded < 0) {
        printf("[INFO] '%s' not found. Loading built-in default catalog...\n\n", csvPath);
        populateDefaultCatalog(engine);
    } else {
        printf("[INFO] Loaded %d product(s) from '%s'.\n\n", loaded, csvPath);
    }


    // If "--demo" is passed as a command-line argument, run demo directly
    if (argc > 1 && strcmp(argv[1], "--demo") == 0) {
        runAutomatedDemo(engine);
        freeEngine(engine);
        return 0;
    }

    int choice = -1;
    char buffer[256];

    while (1) {
        showMenu();
        if (scanf("%d", &choice) != 1) {
            // Clear invalid input
            while (getchar() != '\n');
            printf("\nInvalid input. Please enter a number.\n\n");
            continue;
        }
        while (getchar() != '\n'); // clear newline

        if (choice == 0) {
            printf("\nExiting E-Commerce DSA Platform. Goodbye!\n");
            break;
        }

        switch (choice) {
            case 1: {
                int count = 0;
                Product** list = engineGetAllProducts(engine, &count);
                printf("\n================================================================================\n");
                printf("                     AVAILABLE PRODUCT CATALOG & DETAILS\n");
                printf("================================================================================\n");
                if (!list || count == 0) {
                    printf("  (No products currently available in the catalog)\n");
                } else {
                    printf("  Total Products Available: %d\n", count);
                    printf("  ------------------------------------------------------------------------------\n");
                    printf("  %-10s %-30s %-15s %-12s %-10s\n", "SKU / ID", "PRODUCT NAME", "CATEGORY", "PRICE ($)", "STOCK");
                    printf("  ------------------------------------------------------------------------------\n");
                    for (int i = 0; i < count; i++) {
                        Product* p = list[i];
                        printf("  %-10s %-30s %-15s %-12.2f %-10d\n",
                               p->id, p->name, p->category, p->price, p->stockQuantity);
                    }
                    printf("  ------------------------------------------------------------------------------\n");
                    free(list);
                }
                printf("\n");
                break;
            }
            case 2: {
                printf("\nEnter prefix to search (e.g., 'app', 'son', 'lap'): ");
                if (fgets(buffer, sizeof(buffer), stdin)) {
                    buffer[strcspn(buffer, "\r\n")] = '\0';
                    testSearch(engine, buffer);
                }
                break;
            }
            case 3: {
                printf("\nEnter Product ID (e.g., 'PROD-01' to 'PROD-07'): ");
                if (fgets(buffer, sizeof(buffer), stdin)) {
                    buffer[strcspn(buffer, "\r\n")] = '\0';
                    Product* p = engineGetProduct(engine, buffer);
                    if (p) {
                        printf("Found in O(1): ");
                        printProduct(p);
                    } else {
                        printf("Product '%s' not found.\n", buffer);
                    }
                }
                printf("\n");
                break;
            }
            case 4: {
                char cust[64], pid[32];
                int qty = 1;
                printf("\nEnter Customer Name/ID: ");
                if (!fgets(cust, sizeof(cust), stdin)) break;
                cust[strcspn(cust, "\r\n")] = '\0';

                printf("Enter Product ID to order (e.g. PROD-01): ");
                if (!fgets(pid, sizeof(pid), stdin)) break;
                pid[strcspn(pid, "\r\n")] = '\0';

                Product* p = engineGetProduct(engine, pid);
                if (!p) {
                    printf("Error: Product '%s' not found in catalog!\n\n", pid);
                    break;
                }

                printf("Enter Quantity: ");
                if (scanf("%d", &qty) != 1 || qty <= 0) {
                    while (getchar() != '\n');
                    qty = 1;
                }
                while (getchar() != '\n');

                OrderItem item;
                strncpy(item.productId, p->id, sizeof(item.productId));
                strncpy(item.productName, p->name, sizeof(item.productName));
                item.quantity = qty;
                item.unitPrice = p->price;

                Order* placed = enginePlaceOrder(engine, cust, &item, 1);
                printf("\n+ Successfully placed order:\n");
                printOrder(placed);
                printf("\n");
                break;
            }
            case 5: {
                char oid[32], pid[32];
                int newQty;
                printf("\nEnter Order ID to edit (e.g. ORD-1001): ");
                if (!fgets(oid, sizeof(oid), stdin)) break;
                oid[strcspn(oid, "\r\n")] = '\0';

                printf("Enter Product ID in order: ");
                if (!fgets(pid, sizeof(pid), stdin)) break;
                pid[strcspn(pid, "\r\n")] = '\0';

                printf("Enter new quantity (0 to remove): ");
                if (scanf("%d", &newQty) != 1) {
                    while (getchar() != '\n');
                    break;
                }
                while (getchar() != '\n');

                if (engineEditOrderItemQuantity(engine, oid, pid, newQty)) {
                    printf("\n+ Order updated successfully. State recorded to Undo stack.\n");
                    printOrder(findOrderInQueue(engine->orderQueue, oid));
                } else {
                    printf("\nError: Order or Product not found in active queue.\n");
                }
                printf("\n");
                break;
            }
            case 6: {
                char oid[32];
                printf("\nEnter Order ID to cancel (e.g. ORD-1001): ");
                if (!fgets(oid, sizeof(oid), stdin)) break;
                oid[strcspn(oid, "\r\n")] = '\0';

                if (engineCancelOrder(engine, oid)) {
                    printf("\n+ Order #%s cancelled and removed in O(1) from FIFO queue.\n", oid);
                    printf("  Action snapshot stored in Undo stack (can be reversed).\n");
                } else {
                    printf("\nError: Order #%s not found in active queue.\n", oid);
                }
                printf("\n");
                break;
            }
            case 7: {
                char msg[256];
                if (engineUndo(engine, msg, sizeof(msg))) {
                    printf("\n[UNDO SUCCESS]: %s\n", msg);
                } else {
                    printf("\n[UNDO INFO]: %s\n", msg);
                }
                printf("\n");
                break;
            }
            case 8: {
                char msg[256];
                if (engineRedo(engine, msg, sizeof(msg))) {
                    printf("\n[REDO SUCCESS]: %s\n", msg);
                } else {
                    printf("\n[REDO INFO]: %s\n", msg);
                }
                printf("\n");
                break;
            }
            case 9: {
                Order* proc = engineProcessNextOrder(engine);
                if (proc) {
                    printf("\n[ORDER FULFILLED - FIFO HEAD]:\n");
                    printOrder(proc);
                    freeOrder(proc);
                } else {
                    printf("\nNo pending orders to process.\n");
                }
                printf("\n");
                break;
            }
            case 10: {
                printOrderQueue(engine->orderQueue);
                printf("\n");
                break;
            }
            case 11: {
                runAutomatedDemo(engine);
                break;
            }
            default:
                printf("\nInvalid choice. Please select 0-11.\n\n");
        }
    }

    freeEngine(engine);
    return 0;
}
