#include "order_queue.h"
#include <stdlib.h>
#include <string.h>

/* ========================================================================= */
/* 1. ORDER HASH MAP (orderId -> DoublyLinkedNode* in O(1))                  */
/* ========================================================================= */

static unsigned int hashOrder(const char* str, int capacity) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return (unsigned int)(hash % capacity);
}

static OrderMap* createOrderMap(int capacity) {
    OrderMap* map = (OrderMap*)malloc(sizeof(OrderMap));
    if (!map) return NULL;
    map->capacity = capacity > 0 ? capacity : 101;
    map->size = 0;
    map->buckets = (OrderMapNode**)calloc(map->capacity, sizeof(OrderMapNode*));
    return map;
}

static void mapPut(OrderMap* map, const char* orderId, DoublyLinkedNode* node) {
    if (!map || !orderId) return;
    unsigned int idx = hashOrder(orderId, map->capacity);
    OrderMapNode* curr = map->buckets[idx];
    while (curr) {
        if (strcmp(curr->orderId, orderId) == 0) {
            curr->listNode = node;
            return;
        }
        curr = curr->next;
    }
    OrderMapNode* entry = (OrderMapNode*)malloc(sizeof(OrderMapNode));
    if (!entry) return;
    strncpy(entry->orderId, orderId, sizeof(entry->orderId) - 1);
    entry->orderId[sizeof(entry->orderId) - 1] = '\0';
    entry->listNode = node;
    entry->next = map->buckets[idx];
    map->buckets[idx] = entry;
    map->size++;
}

static DoublyLinkedNode* mapGet(const OrderMap* map, const char* orderId) {
    if (!map || !orderId) return NULL;
    unsigned int idx = hashOrder(orderId, map->capacity);
    OrderMapNode* curr = map->buckets[idx];
    while (curr) {
        if (strcmp(curr->orderId, orderId) == 0) {
            return curr->listNode;
        }
        curr = curr->next;
    }
    return NULL;
}

static void mapRemove(OrderMap* map, const char* orderId) {
    if (!map || !orderId) return;
    unsigned int idx = hashOrder(orderId, map->capacity);
    OrderMapNode* curr = map->buckets[idx];
    OrderMapNode* prev = NULL;
    while (curr) {
        if (strcmp(curr->orderId, orderId) == 0) {
            if (prev) {
                prev->next = curr->next;
            } else {
                map->buckets[idx] = curr->next;
            }
            free(curr);
            map->size--;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

static void freeOrderMap(OrderMap* map) {
    if (!map) return;
    for (int i = 0; i < map->capacity; i++) {
        OrderMapNode* curr = map->buckets[i];
        while (curr) {
            OrderMapNode* tmp = curr;
            curr = curr->next;
            free(tmp);
        }
    }
    free(map->buckets);
    free(map);
}

/* ========================================================================= */
/* 2. ORDER QUEUE IMPLEMENTATION (FIFO Doubly Linked List + Hash Map)        */
/* ========================================================================= */

OrderQueue* createOrderQueue(int mapCapacity) {
    OrderQueue* q = (OrderQueue*)malloc(sizeof(OrderQueue));
    if (!q) return NULL;
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    q->map = createOrderMap(mapCapacity);
    return q;
}

void enqueueOrder(OrderQueue* q, Order* order) {
    if (!q || !order) return;
    // Disallow duplicate orderIds in queue
    if (mapGet(q->map, order->orderId) != NULL) return;

    order->status = ORDER_PENDING;
    DoublyLinkedNode* node = (DoublyLinkedNode*)malloc(sizeof(DoublyLinkedNode));
    if (!node) return;
    node->order = order;
    node->prev = q->tail;
    node->next = NULL;

    if (q->tail) {
        q->tail->next = node;
        q->tail = node;
    } else {
        q->head = node;
        q->tail = node;
    }

    q->size++;
    mapPut(q->map, order->orderId, node);
}

Order* dequeueOrder(OrderQueue* q) {
    if (!q || !q->head) return NULL;

    DoublyLinkedNode* node = q->head;
    Order* order = node->order;

    q->head = q->head->next;
    if (q->head) {
        q->head->prev = NULL;
    } else {
        q->tail = NULL;
    }

    mapRemove(q->map, order->orderId);
    q->size--;
    free(node);

    order->status = ORDER_PROCESSED;
    return order;
}

Order* peekOrder(const OrderQueue* q) {
    if (!q || !q->head) return NULL;
    return q->head->order;
}

Order* findOrderInQueue(const OrderQueue* q, const char* orderId) {
    if (!q || !orderId) return NULL;
    DoublyLinkedNode* node = mapGet(q->map, orderId);
    return node ? node->order : NULL;
}

Order* removeOrderById(OrderQueue* q, const char* orderId) {
    if (!q || !orderId) return NULL;
    DoublyLinkedNode* node = mapGet(q->map, orderId);
    if (!node) return NULL;

    // Unlink node in O(1)
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        q->head = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    } else {
        q->tail = node->prev;
    }

    mapRemove(q->map, orderId);
    q->size--;

    Order* order = node->order;
    order->status = ORDER_CANCELLED;
    free(node);
    return order;
}

void insertOrderAt(OrderQueue* q, Order* order, int position) {
    if (!q || !order) return;
    if (mapGet(q->map, order->orderId) != NULL) return;

    order->status = ORDER_PENDING;
    DoublyLinkedNode* node = (DoublyLinkedNode*)malloc(sizeof(DoublyLinkedNode));
    if (!node) return;
    node->order = order;
    node->prev = NULL;
    node->next = NULL;

    if (!q->head || position <= 0) {
        // Insert at head
        if (!q->head) {
            q->head = node;
            q->tail = node;
        } else {
            node->next = q->head;
            q->head->prev = node;
            q->head = node;
        }
    } else if (position >= q->size) {
        // Append at tail
        node->prev = q->tail;
        q->tail->next = node;
        q->tail = node;
    } else {
        // Traverse to position
        DoublyLinkedNode* curr = q->head;
        for (int i = 0; i < position && curr->next; i++) {
            curr = curr->next;
        }
        node->prev = curr->prev;
        node->next = curr;
        if (curr->prev) {
            curr->prev->next = node;
        }
        curr->prev = node;
    }

    q->size++;
    mapPut(q->map, order->orderId, node);
}

int getOrderPosition(const OrderQueue* q, const char* orderId) {
    if (!q || !orderId) return -1;
    DoublyLinkedNode* curr = q->head;
    int idx = 0;
    while (curr) {
        if (strcmp(curr->order->orderId, orderId) == 0) {
            return idx;
        }
        idx++;
        curr = curr->next;
    }
    return -1;
}

void printOrderQueue(const OrderQueue* q) {
    if (!q) return;
    printf("[FIFO Order Queue | Total Pending: %d]\n", q->size);
    DoublyLinkedNode* curr = q->head;
    int slot = 0;
    while (curr) {
        printf("   Slot %d: Order #%s (Customer: %s, Total: $%.2f)\n",
               slot++, curr->order->orderId, curr->order->customerId, curr->order->totalAmount);
        curr = curr->next;
    }
    if (q->size == 0) {
        printf("   (Queue is currently empty)\n");
    }
}

void freeOrderQueue(OrderQueue* q) {
    if (!q) return;
    DoublyLinkedNode* curr = q->head;
    while (curr) {
        DoublyLinkedNode* tmp = curr;
        curr = curr->next;
        free(tmp);
    }
    freeOrderMap(q->map);
    free(q);
}
