#ifndef ENGINE_H
#define ENGINE_H
#ifndef ENGINE_HASH_CAPACITY
#define ENGINE_HASH_CAPACITY 200003   /* prime > max rows */
#endif

#include "product.h"
#include "trie.h"
#include "order.h"
#include "order_queue.h"
#include "undo_redo.h"

/**
 * Integrated E-Commerce Platform Facade in C.
 */
typedef struct ECommerceEngine {
    ProductCatalog* catalog;
    OrderQueue* orderQueue;
    UndoRedoManager* undoRedo;
    int orderSequence;
} ECommerceEngine;

ECommerceEngine* createEngine(void);
void engineAddProduct(ECommerceEngine* eng, const char* id, const char* name,
                      const char* category, double price, int stock);
Product** engineSearchCatalog(ECommerceEngine* eng, const char* prefix, int* count);
Product* engineGetProduct(ECommerceEngine* eng, const char* id);
Product** engineGetAllProducts(ECommerceEngine* eng, int* count);
int engineLoadCatalogFromCSV(ECommerceEngine* eng, const char* filepath);

Order* enginePlaceOrder(ECommerceEngine* eng, const char* customerId,
                        const OrderItem* items, int itemCount);
bool engineEditOrderItemQuantity(ECommerceEngine* eng, const char* orderId,
                                 const char* productId, int newQty);
bool engineCancelOrder(ECommerceEngine* eng, const char* orderId);

bool engineUndo(ECommerceEngine* eng, char* messageBuf, size_t bufSize);
bool engineRedo(ECommerceEngine* eng, char* messageBuf, size_t bufSize);

Order* engineProcessNextOrder(ECommerceEngine* eng);
void freeEngine(ECommerceEngine* eng);

#endif // ENGINE_H
