#ifndef ORDER_H
#define ORDER_H

#include <stdbool.h>
#include <stdio.h>

typedef enum {
    ORDER_PENDING,
    ORDER_PROCESSED,
    ORDER_CANCELLED
} OrderStatus;

typedef struct OrderItem {
    char productId[32];
    char productName[128];
    int quantity;
    double unitPrice;
} OrderItem;

typedef struct Order {
    char orderId[32];
    char customerId[64];
    long long timestamp;
    OrderItem* items;
    int itemCount;
    int itemCapacity;
    double totalAmount;
    OrderStatus status;
} Order;

Order* createOrder(const char* orderId, const char* customerId);
void orderAddItem(Order* order, const char* prodId, const char* prodName, int qty, double price);
bool orderUpdateQuantity(Order* order, const char* prodId, int newQty);
void calculateOrderTotal(Order* order);
Order* cloneOrder(const Order* source);
void restoreOrderState(Order* target, const Order* source);
void freeOrder(Order* order);
void printOrder(const Order* order);
const char* orderStatusToString(OrderStatus status);

#endif // ORDER_H
