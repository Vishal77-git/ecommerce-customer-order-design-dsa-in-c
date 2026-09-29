#ifndef ORDER_QUEUE_H
#define ORDER_QUEUE_H

#include "order.h"
#include <stdbool.h>

/**
 * Node in the Doubly Linked List for FIFO Queue.
 * Enables O(1) removal from anywhere in queue when combined with hash map.
 */
typedef struct DoublyLinkedNode {
    Order* order;
    struct DoublyLinkedNode* prev;
    struct DoublyLinkedNode* next;
} DoublyLinkedNode;

/**
 * Hash Table entry mapping orderId -> DoublyLinkedNode* in O(1).
 */
typedef struct OrderMapNode {
    char orderId[32];
    DoublyLinkedNode* listNode;
    struct OrderMapNode* next;
} OrderMapNode;

typedef struct OrderMap {
    OrderMapNode** buckets;
    int capacity;
    int size;
} OrderMap;

/**
 * High-performance Order Processing Queue.
 * Combines Doubly Linked List (FIFO sequence) + Hash Map (O(1) random access).
 */
typedef struct OrderQueue {
    DoublyLinkedNode* head;
    DoublyLinkedNode* tail;
    OrderMap* map;
    int size;
} OrderQueue;

OrderQueue* createOrderQueue(int mapCapacity);
void enqueueOrder(OrderQueue* q, Order* order);
Order* dequeueOrder(OrderQueue* q);
Order* peekOrder(const OrderQueue* q);
Order* findOrderInQueue(const OrderQueue* q, const char* orderId);
Order* removeOrderById(OrderQueue* q, const char* orderId);
void insertOrderAt(OrderQueue* q, Order* order, int position);
int getOrderPosition(const OrderQueue* q, const char* orderId);
void printOrderQueue(const OrderQueue* q);
void freeOrderQueue(OrderQueue* q);

#endif // ORDER_QUEUE_H
