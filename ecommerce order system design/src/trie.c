#include "trie.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/* ========================================================================= */
/* 1. TRIE DATA STRUCTURE (Prefix Search & Autocomplete in O(L + K))         */
/* ========================================================================= */

/*
 * Compact 37-slot alphabet:
 *   a-z  -> index  0-25
 *   0-9  -> index 26-35
 *   '-'  -> index 36
 *   everything else is skipped during insert, breaks the word during search
 */
static int charToIdx(unsigned char c) {
    c = (unsigned char)tolower(c);
    if (c >= 'a' && c <= 'z') return c - 'a';          /* 0-25  */
    if (c >= '0' && c <= '9') return 26 + (c - '0');   /* 26-35 */
    if (c == '-')              return 36;               /* 36    */
    return -1; /* skip */
}

TrieNode* createTrieNode(void) {
    TrieNode* node = (TrieNode*)calloc(1, sizeof(TrieNode));
    return node;
}

/* Append product pointer to node's list — called only at word-end nodes */
static void addProductToNode(TrieNode* node, Product* product) {
    if (!node || !product) return;
    /* dedup check — only at end-of-word, so list is small */
    for (int i = 0; i < node->matchCount; i++) {
        if (node->matchedProducts[i] == product) return;
    }
    if (node->matchCount >= node->matchCapacity) {
        int newCap = node->matchCapacity == 0 ? 4 : node->matchCapacity * 2;
        Product** arr = (Product**)realloc(node->matchedProducts,
                                           sizeof(Product*) * newCap);
        if (!arr) return;
        node->matchedProducts = arr;
        node->matchCapacity   = newCap;
    }
    node->matchedProducts[node->matchCount++] = product;
}

void insertTrie(TrieNode* root, const char* word, Product* product) {
    if (!root || !word || !product) return;
    TrieNode* curr = root;
    for (int i = 0; word[i] != '\0'; i++) {
        int idx = charToIdx((unsigned char)word[i]);
        if (idx < 0) continue;          /* skip punctuation / spaces */
        if (!curr->children[idx]) {
            curr->children[idx] = createTrieNode();
            if (!curr->children[idx]) return;
        }
        curr = curr->children[idx];
        /* NOTE: products stored only at word-end below, NOT at every node */
    }
    curr->isEndOfWord = true;
    addProductToNode(curr, product);    /* store only at the word's leaf */
}

/* -----------------------------------------------------------------------
 * DFS collector: gather every product stored at end-of-word nodes within
 * the subtree rooted at 'node'. Results are written into *out / *n.
 * outCap tracks allocated capacity; caller passes address of both.
 * ----------------------------------------------------------------------- */
static void collectProducts(TrieNode* node,
                             Product*** out, int* n, int* outCap) {
    if (!node) return;
    /* add products stored at this node (end-of-word nodes only have items) */
    for (int i = 0; i < node->matchCount; i++) {
        Product* p = node->matchedProducts[i];
        /* dedup: skip if pointer already in list */
        int dup = 0;
        for (int j = 0; j < *n; j++) {
            if ((*out)[j] == p) { dup = 1; break; }
        }
        if (dup) continue;
        if (*n >= *outCap) {
            int nc = *outCap == 0 ? 16 : *outCap * 2;
            Product** na = (Product**)realloc(*out, sizeof(Product*) * nc);
            if (!na) return;
            *out    = na;
            *outCap = nc;
        }
        (*out)[(*n)++] = p;
    }
    /* recurse into children */
    for (int i = 0; i < TRIE_ALPHABET_SIZE; i++) {
        if (node->children[i])
            collectProducts(node->children[i], out, n, outCap);
    }
}

Product** searchTriePrefix(TrieNode* root, const char* prefix, int* resultCount) {
    *resultCount = 0;
    if (!root || !prefix) return NULL;

    /* walk trie to end of prefix */
    TrieNode* curr = root;
    for (int i = 0; prefix[i] != '\0'; i++) {
        int idx = charToIdx((unsigned char)prefix[i]);
        if (idx < 0 || !curr->children[idx]) {
            return NULL; /* prefix not in trie */
        }
        curr = curr->children[idx];
    }

    /* DFS from that node to collect all matching products */
    Product** results = NULL;
    int n = 0, cap = 0;
    collectProducts(curr, &results, &n, &cap);
    *resultCount = n;
    return results; /* caller must free() this pointer */
}

void freeTrie(TrieNode* root) {
    if (!root) return;
    for (int i = 0; i < TRIE_ALPHABET_SIZE; i++) {
        if (root->children[i]) freeTrie(root->children[i]);
    }
    free(root->matchedProducts);
    free(root);
}

/* ========================================================================= */
/* 2. HASH TABLE FOR PRODUCTS (O(1) Direct Lookup by SKU / Product ID)      */
/* ========================================================================= */

static unsigned int hashString(const char* str, int capacity) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return (unsigned int)(hash % capacity);
}

ProductHashTable* createProductHashTable(int capacity) {
    ProductHashTable* table = (ProductHashTable*)malloc(sizeof(ProductHashTable));
    if (!table) return NULL;
    table->capacity = capacity > 0 ? capacity : 101;
    table->size = 0;
    table->buckets = (ProductHashNode**)calloc(table->capacity, sizeof(ProductHashNode*));
    return table;
}

void insertProductHash(ProductHashTable* table, Product* product) {
    if (!table || !product) return;
    unsigned int index = hashString(product->id, table->capacity);
    ProductHashNode* curr = table->buckets[index];
    while (curr) {
        if (strcmp(curr->id, product->id) == 0) {
            curr->product = product;
            return;
        }
        curr = curr->next;
    }
    ProductHashNode* newNode = (ProductHashNode*)malloc(sizeof(ProductHashNode));
    if (!newNode) return;
    strncpy(newNode->id, product->id, sizeof(newNode->id) - 1);
    newNode->id[sizeof(newNode->id) - 1] = '\0';
    newNode->product = product;
    newNode->next = table->buckets[index];
    table->buckets[index] = newNode;
    table->size++;
}

Product* findProductById(ProductHashTable* table, const char* id) {
    if (!table || !id) return NULL;
    unsigned int index = hashString(id, table->capacity);
    ProductHashNode* curr = table->buckets[index];
    while (curr) {
        if (strcmp(curr->id, id) == 0) {
            return curr->product;
        }
        curr = curr->next;
    }
    return NULL;
}

static int compareProductsById(const void* a, const void* b) {
    const Product* p1 = *(const Product* const*)a;
    const Product* p2 = *(const Product* const*)b;
    if (!p1 || !p2) return 0;
    return strcmp(p1->id, p2->id);
}

Product** getHashTableProducts(ProductHashTable* table, int* count) {
    if (!table || table->size <= 0) {
        if (count) *count = 0;
        return NULL;
    }
    Product** list = (Product**)malloc(sizeof(Product*) * table->size);
    if (!list) {
        if (count) *count = 0;
        return NULL;
    }
    int idx = 0;
    for (int i = 0; i < table->capacity; i++) {
        ProductHashNode* curr = table->buckets[i];
        while (curr) {
            if (idx < table->size) {
                list[idx++] = curr->product;
            }
            curr = curr->next;
        }
    }
    qsort(list, idx, sizeof(Product*), compareProductsById);
    if (count) *count = idx;
    return list;
}

void freeProductHashTable(ProductHashTable* table) {
    if (!table) return;
    for (int i = 0; i < table->capacity; i++) {
        ProductHashNode* curr = table->buckets[i];
        while (curr) {
            ProductHashNode* tmp = curr;
            curr = curr->next;
            free(tmp);
        }
    }
    free(table->buckets);
    free(table);
}

/* ========================================================================= */
/* 3. COMBINED CATALOG COMPONENT                                             */
/* ========================================================================= */

ProductCatalog* createProductCatalog(int hashCapacity) {
    ProductCatalog* catalog = (ProductCatalog*)malloc(sizeof(ProductCatalog));
    if (!catalog) return NULL;
    catalog->root = createTrieNode();
    catalog->table = createProductHashTable(hashCapacity);
    return catalog;
}

void catalogAddProduct(ProductCatalog* catalog, Product* product) {
    if (!catalog || !product) return;

    // 1. Add to Hash Table for O(1) lookup
    insertProductHash(catalog->table, product);

    // 2. Tokenize name and category into Trie for O(L) prefix search
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s %s", product->name, product->category);

    char* token = strtok(buffer, " ,.-_/");
    while (token) {
        insertTrie(catalog->root, token, product);
        token = strtok(NULL, " ,.-_/");
    }
}

Product** catalogSearchPrefix(ProductCatalog* catalog, const char* prefix, int* resultCount) {
    if (!catalog) {
        *resultCount = 0;
        return NULL;
    }
    return searchTriePrefix(catalog->root, prefix, resultCount);
}

Product* catalogFindById(ProductCatalog* catalog, const char* id) {
    if (!catalog) return NULL;
    return findProductById(catalog->table, id);
}

Product** catalogGetAllProducts(ProductCatalog* catalog, int* count) {
    if (!catalog) {
        if (count) *count = 0;
        return NULL;
    }
    return getHashTableProducts(catalog->table, count);
}

void freeProductCatalog(ProductCatalog* catalog) {
    if (!catalog) return;
    if (catalog->root) freeTrie(catalog->root);
    if (catalog->table) freeProductHashTable(catalog->table);
    free(catalog);
}
