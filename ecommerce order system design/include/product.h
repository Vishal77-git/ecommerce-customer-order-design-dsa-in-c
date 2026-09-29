#ifndef PRODUCT_H
#define PRODUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Product representation in the e-commerce catalog.
 */
typedef struct Product {
    char id[32];
    char name[128];
    char category[64];
    double price;
    int stockQuantity;
} Product;

Product* createProduct(const char* id, const char* name, const char* category, double price, int stock);
void freeProduct(Product* p);
void printProduct(const Product* p);

#endif // PRODUCT_H
