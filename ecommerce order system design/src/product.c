#include "product.h"

Product* createProduct(const char* id, const char* name, const char* category, double price, int stock) {
    Product* p = (Product*)malloc(sizeof(Product));
    if (!p) return NULL;
    strncpy(p->id, id ? id : "", sizeof(p->id) - 1);
    p->id[sizeof(p->id) - 1] = '\0';
    strncpy(p->name, name ? name : "", sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';
    strncpy(p->category, category ? category : "", sizeof(p->category) - 1);
    p->category[sizeof(p->category) - 1] = '\0';
    p->price = price;
    p->stockQuantity = stock;
    return p;
}

void freeProduct(Product* p) {
    if (p) {
        free(p);
    }
}

void printProduct(const Product* p) {
    if (!p) {
        printf("(null product)\n");
        return;
    }
    printf("[ID: %s | %s | Cat: %s | $%.2f | Stock: %d]\n",
           p->id, p->name, p->category, p->price, p->stockQuantity);
}
