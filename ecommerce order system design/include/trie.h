#ifndef TRIE_H
#define TRIE_H

#include "product.h"
#include <stdbool.h>

/* 37 slots: a-z (0-25) | 0-9 (26-35) | hyphen (36) */
#define TRIE_ALPHABET_SIZE 37

/**
 * Node in Prefix Trie for product search.
 * Stores child transitions and a list of matching Product pointers.
 */
typedef struct TrieNode {
    struct TrieNode* children[TRIE_ALPHABET_SIZE];
    Product** matchedProducts;
    int matchCount;
    int matchCapacity;
    bool isEndOfWord;
} TrieNode;

/**
 * Hash Table entry for direct O(1) product lookup by ID.
 */
typedef struct ProductHashNode {
    char id[32];
    Product* product;
    struct ProductHashNode* next;
} ProductHashNode;

typedef struct ProductHashTable {
    ProductHashNode** buckets;
    int capacity;
    int size;
} ProductHashTable;

/**
 * Combined Product Catalog search component.
 */
typedef struct ProductCatalog {
    TrieNode* root;
    ProductHashTable* table;
} ProductCatalog;

TrieNode* createTrieNode(void);
void insertTrie(TrieNode* root, const char* word, Product* product);
Product** searchTriePrefix(TrieNode* root, const char* prefix, int* resultCount);
void freeTrie(TrieNode* root);

ProductHashTable* createProductHashTable(int capacity);
void insertProductHash(ProductHashTable* table, Product* product);
Product* findProductById(ProductHashTable* table, const char* id);
Product** getHashTableProducts(ProductHashTable* table, int* count);
void freeProductHashTable(ProductHashTable* table);

ProductCatalog* createProductCatalog(int hashCapacity);
void catalogAddProduct(ProductCatalog* catalog, Product* product);
Product** catalogSearchPrefix(ProductCatalog* catalog, const char* prefix, int* resultCount);
Product* catalogFindById(ProductCatalog* catalog, const char* id);
Product** catalogGetAllProducts(ProductCatalog* catalog, int* count);
void freeProductCatalog(ProductCatalog* catalog);

#endif // TRIE_H
