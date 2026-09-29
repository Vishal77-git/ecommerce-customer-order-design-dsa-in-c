#include "order.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

Order* createOrder(const char* orderId, const char* customerId) {
    Order* o = (Order*)malloc(sizeof(Order));
    if (!o) return NULL;
    strncpy(o->orderId, orderId ? orderId : "", sizeof(o->orderId) - 1);
    o->orderId[sizeof(o->orderId) - 1] = '\0';
    strncpy(o->customerId, customerId ? customerId : "", sizeof(o->customerId) - 1);
    o->customerId[sizeof(o->customerId) - 1] = '\0';
    o->timestamp = (long long)time(NULL);
    o->itemCapacity = 4;
    o->itemCount = 0;
    o->items = (OrderItem*)malloc(sizeof(OrderItem) * o->itemCapacity);
    o->totalAmount = 0.0;
    o->status = ORDER_PENDING;
    return o;
}

void calculateOrderTotal(Order* order) {
    if (!order) return;
    double sum = 0.0;
    for (int i = 0; i < order->itemCount; i++) {
        sum += order->items[i].quantity * order->items[i].unitPrice;
    }
    order->totalAmount = sum;
}

void orderAddItem(Order* order, const char* prodId, const char* prodName, int qty, double price) {
    if (!order || !prodId || qty <= 0) return;

    // Check if item already exists in order
    for (int i = 0; i < order->itemCount; i++) {
        if (strcmp(order->items[i].productId, prodId) == 0) {
            order->items[i].quantity += qty;
            calculateOrderTotal(order);
            return;
        }
    }

    if (order->itemCount >= order->itemCapacity) {
        int newCap = order->itemCapacity * 2;
        OrderItem* newArr = (OrderItem*)realloc(order->items, sizeof(OrderItem) * newCap);
        if (!newArr) return;
        order->items = newArr;
        order->itemCapacity = newCap;
    }

    OrderItem* item = &order->items[order->itemCount++];
    strncpy(item->productId, prodId, sizeof(item->productId) - 1);
    item->productId[sizeof(item->productId) - 1] = '\0';
    strncpy(item->productName, prodName ? prodName : "", sizeof(item->productName) - 1);
    item->productName[sizeof(item->productName) - 1] = '\0';
    item->quantity = qty;
    item->unitPrice = price;

    calculateOrderTotal(order);
}

bool orderUpdateQuantity(Order* order, const char* prodId, int newQty) {
    if (!order || !prodId) return false;
    for (int i = 0; i < order->itemCount; i++) {
        if (strcmp(order->items[i].productId, prodId) == 0) {
            if (newQty <= 0) {
                // Remove item by shifting remaining items
                for (int j = i; j < order->itemCount - 1; j++) {
                    order->items[j] = order->items[j + 1];
                }
                order->itemCount--;
            } else {
                order->items[i].quantity = newQty;
            }
            calculateOrderTotal(order);
            return true;
        }
    }
    return false;
}

Order* cloneOrder(const Order* source) {
    if (!source) return NULL;
    Order* copy = (Order*)malloc(sizeof(Order));
    if (!copy) return NULL;
    memcpy(copy, source, sizeof(Order));
    copy->items = (OrderItem*)malloc(sizeof(OrderItem) * (copy->itemCapacity > 0 ? copy->itemCapacity : 4));
    for (int i = 0; i < source->itemCount; i++) {
        copy->items[i] = source->items[i];
    }
    return copy;
}

void restoreOrderState(Order* target, const Order* source) {
    if (!target || !source) return;
    if (target->items) {
        free(target->items);
    }
    target->items = (OrderItem*)malloc(sizeof(OrderItem) * source->itemCapacity);
    for (int i = 0; i < source->itemCount; i++) {
        target->items[i] = source->items[i];
    }
    target->itemCount = source->itemCount;
    target->itemCapacity = source->itemCapacity;
    target->totalAmount = source->totalAmount;
    target->status = source->status;
}

void freeOrder(Order* order) {
    if (order) {
        if (order->items) {
            free(order->items);
        }
        free(order);
    }
}

const char* orderStatusToString(OrderStatus status) {
    switch (status) {
        case ORDER_PENDING: return "PENDING";
        case ORDER_PROCESSED: return "PROCESSED";
        case ORDER_CANCELLED: return "CANCELLED";
        default: return "UNKNOWN";
    }
}

void printOrder(const Order* order) {
    if (!order) {
        printf("(null order)\n");
        return;
    }
    printf("Order #%s [Customer: %s | Status: %s | Total: $%.2f]\n",
           order->orderId, order->customerId, orderStatusToString(order->status), order->totalAmount);
    for (int i = 0; i < order->itemCount; i++) {
        const OrderItem* it = &order->items[i];
        printf("   - %s (x%d @ $%.2f = $%.2f)\n",
               it->productName, it->quantity, it->unitPrice, it->quantity * it->unitPrice);
    }
}
