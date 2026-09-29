#include "engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ECommerceEngine* createEngine(void) {
    ECommerceEngine* eng = (ECommerceEngine*)malloc(sizeof(ECommerceEngine));
    if (!eng) return NULL;
    eng->catalog       = createProductCatalog(ENGINE_HASH_CAPACITY);
    eng->orderQueue    = createOrderQueue(101);
    eng->undoRedo      = createUndoRedoManager(50);
    eng->orderSequence = 1000;
    return eng;
}

void engineAddProduct(ECommerceEngine* eng, const char* id, const char* name,
                      const char* category, double price, int stock) {
    if (!eng) return;
    Product* p = createProduct(id, name, category, price, stock);
    if (p) {
        catalogAddProduct(eng->catalog, p);
    }
}

Product** engineSearchCatalog(ECommerceEngine* eng, const char* prefix, int* count) {
    if (!eng) {
        *count = 0;
        return NULL;
    }
    return catalogSearchPrefix(eng->catalog, prefix, count);
}

Product* engineGetProduct(ECommerceEngine* eng, const char* id) {
    if (!eng) return NULL;
    return catalogFindById(eng->catalog, id);
}

Product** engineGetAllProducts(ECommerceEngine* eng, int* count) {
    if (!eng) {
        if (count) *count = 0;
        return NULL;
    }
    return catalogGetAllProducts(eng->catalog, count);
}

/* -------------------------------------------------------------------------
 * CSV Loader — reads products.csv with header: id,name,category,price,stock
 * Returns: number of products successfully loaded, or -1 if file not found.
 * Handles quoted fields (e.g. "Dell XPS 15, Gen 2"), skips blank lines and
 * lines starting with '#'. Trims leading/trailing spaces from each field.
 * -------------------------------------------------------------------------*/
static void trimInPlace(char* s) {
    if (!s) return;
    /* strip trailing whitespace and CR/LF */
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' ||
                        s[len-1] == '\r' || s[len-1] == '\n')) {
        s[--len] = '\0';
    }
    /* strip leading whitespace */
    int start = 0;
    while (s[start] == ' ' || s[start] == '\t') start++;
    if (start > 0) memmove(s, s + start, len - start + 1);
    /* strip surrounding double-quotes */
    len = (int)strlen(s);
    if (len >= 2 && s[0] == '"' && s[len-1] == '"') {
        s[len-1] = '\0';
        memmove(s, s + 1, len - 1);
    }
}

/* Split one CSV line into exactly 5 fields. Returns 1 on success, 0 on fail.
 * Simple parser: honours double-quoted fields (no escaped quotes inside). */
static int parseCSVLine(const char* line, char* f0, char* f1,
                         char* f2, char* f3, char* f4) {
    char* fields[5] = {f0, f1, f2, f3, f4};
    int fi = 0;
    int oi = 0;
    int inQuote = 0;

    for (int ci = 0; line[ci] != '\0' && fi < 5; ci++) {
        char c = line[ci];
        if (c == '"') {
            inQuote = !inQuote;
        } else if (c == ',' && !inQuote) {
            fields[fi][oi] = '\0';
            fi++;
            oi = 0;
        } else {
            if (oi < 127) fields[fi][oi++] = c;
        }
    }
    if (fi < 5) { fields[fi][oi] = '\0'; fi++; }
    return (fi == 5) ? 1 : 0;
}

int engineLoadCatalogFromCSV(ECommerceEngine* eng, const char* filepath) {
    if (!eng || !filepath) return -1;

    FILE* fp = fopen(filepath, "r");
    if (!fp) return -1;  /* file not found */

    char line[512];
    int loaded = 0;
    int lineNo = 0;

    while (fgets(line, sizeof(line), fp)) {
        lineNo++;

        /* skip blank lines and comment lines */
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '#') continue;

        /* skip header row (first non-blank, non-comment line) */
        if (lineNo == 1) continue;

        char f0[128], f1[128], f2[128], f3[128], f4[128];
        if (!parseCSVLine(line, f0, f1, f2, f3, f4)) continue;

        trimInPlace(f0); trimInPlace(f1);
        trimInPlace(f2); trimInPlace(f3); trimInPlace(f4);

        if (f0[0] == '\0' || f1[0] == '\0') continue; /* id or name empty */

        double price = atof(f3);
        int stock    = atoi(f4);

        engineAddProduct(eng, f0, f1, f2, price, stock);
        loaded++;
    }

    fclose(fp);
    return loaded;
}

Order* enginePlaceOrder(ECommerceEngine* eng, const char* customerId,
                        const OrderItem* items, int itemCount) {
    if (!eng || !items || itemCount <= 0) return NULL;

    char orderId[32];
    snprintf(orderId, sizeof(orderId), "ORD-%d", ++eng->orderSequence);

    Order* order = createOrder(orderId, customerId);
    if (!order) return NULL;

    for (int i = 0; i < itemCount; i++) {
        orderAddItem(order, items[i].productId, items[i].productName,
                     items[i].quantity, items[i].unitPrice);
    }

    int pos = eng->orderQueue->size;
    enqueueOrder(eng->orderQueue, order);

    recordAction(eng->undoRedo, ACTION_PLACE_ORDER, orderId, NULL, order, pos, "Placed order");
    return order;
}

bool engineEditOrderItemQuantity(ECommerceEngine* eng, const char* orderId,
                                 const char* productId, int newQty) {
    if (!eng || !orderId || !productId) return false;

    Order* order = findOrderInQueue(eng->orderQueue, orderId);
    if (!order) return false;

    Order* before = cloneOrder(order);
    int pos = getOrderPosition(eng->orderQueue, orderId);

    bool updated = orderUpdateQuantity(order, productId, newQty);
    if (!updated) {
        freeOrder(before);
        return false;
    }

    Order* after = cloneOrder(order);
    recordAction(eng->undoRedo, ACTION_EDIT_ORDER, orderId, before, after, pos, "Updated item quantity");

    freeOrder(before);
    freeOrder(after);
    return true;
}

bool engineCancelOrder(ECommerceEngine* eng, const char* orderId) {
    if (!eng || !orderId) return false;

    Order* order = findOrderInQueue(eng->orderQueue, orderId);
    if (!order) return false;

    Order* before = cloneOrder(order);
    int pos = getOrderPosition(eng->orderQueue, orderId);

    // Unlink from queue in O(1)
    removeOrderById(eng->orderQueue, orderId);

    Order* after = cloneOrder(before);
    after->status = ORDER_CANCELLED;

    recordAction(eng->undoRedo, ACTION_CANCEL_ORDER, orderId, before, after, pos, "Order cancelled");

    freeOrder(before);
    freeOrder(after);
    return true;
}

bool engineUndo(ECommerceEngine* eng, char* messageBuf, size_t bufSize) {
    if (!eng || !canUndo(eng->undoRedo)) {
        if (messageBuf && bufSize > 0) {
            snprintf(messageBuf, bufSize, "No actions available to undo.");
        }
        return false;
    }

    OrderAction* action = popUndo(eng->undoRedo);

    switch (action->type) {
        case ACTION_PLACE_ORDER: {
            removeOrderById(eng->orderQueue, action->orderId);
            if (messageBuf && bufSize > 0) {
                snprintf(messageBuf, bufSize, "Undid Order Placement: Removed Order #%s from queue.", action->orderId);
            }
            break;
        }
        case ACTION_EDIT_ORDER: {
            Order* curr = findOrderInQueue(eng->orderQueue, action->orderId);
            if (curr && action->stateBefore) {
                restoreOrderState(curr, action->stateBefore);
                if (messageBuf && bufSize > 0) {
                    snprintf(messageBuf, bufSize, "Undid Edit: Reverted Order #%s items to previous state.", action->orderId);
                }
            }
            break;
        }
        case ACTION_CANCEL_ORDER: {
            Order* restored = cloneOrder(action->stateBefore);
            insertOrderAt(eng->orderQueue, restored, action->positionInQueue);
            if (messageBuf && bufSize > 0) {
                snprintf(messageBuf, bufSize, "Undid Cancellation: Restored Order #%s to queue slot %d.",
                         action->orderId, action->positionInQueue);
            }
            break;
        }
    }

    pushRedo(eng->undoRedo, action);
    return true;
}

bool engineRedo(ECommerceEngine* eng, char* messageBuf, size_t bufSize) {
    if (!eng || !canRedo(eng->undoRedo)) {
        if (messageBuf && bufSize > 0) {
            snprintf(messageBuf, bufSize, "No actions available to redo.");
        }
        return false;
    }

    OrderAction* action = popRedo(eng->undoRedo);

    switch (action->type) {
        case ACTION_PLACE_ORDER: {
            Order* reorder = cloneOrder(action->stateAfter);
            insertOrderAt(eng->orderQueue, reorder, action->positionInQueue);
            if (messageBuf && bufSize > 0) {
                snprintf(messageBuf, bufSize, "Redid Order Placement: Re-queued Order #%s.", action->orderId);
            }
            break;
        }
        case ACTION_EDIT_ORDER: {
            Order* curr = findOrderInQueue(eng->orderQueue, action->orderId);
            if (curr && action->stateAfter) {
                restoreOrderState(curr, action->stateAfter);
                if (messageBuf && bufSize > 0) {
                    snprintf(messageBuf, bufSize, "Redid Edit: Re-applied item modifications to Order #%s.", action->orderId);
                }
            }
            break;
        }
        case ACTION_CANCEL_ORDER: {
            removeOrderById(eng->orderQueue, action->orderId);
            if (messageBuf && bufSize > 0) {
                snprintf(messageBuf, bufSize, "Redid Cancellation: Cancelled Order #%s from queue.", action->orderId);
            }
            break;
        }
    }

    pushUndo(eng->undoRedo, action);
    return true;
}

Order* engineProcessNextOrder(ECommerceEngine* eng) {
    if (!eng) return NULL;
    Order* processed = dequeueOrder(eng->orderQueue);
    if (processed) {
        // Deduct inventory
        for (int i = 0; i < processed->itemCount; i++) {
            Product* p = engineGetProduct(eng, processed->items[i].productId);
            if (p) {
                p->stockQuantity -= processed->items[i].quantity;
                if (p->stockQuantity < 0) p->stockQuantity = 0;
            }
        }
    }
    return processed;
}

void freeEngine(ECommerceEngine* eng) {
    if (!eng) return;
    if (eng->catalog) freeProductCatalog(eng->catalog);
    if (eng->orderQueue) freeOrderQueue(eng->orderQueue);
    if (eng->undoRedo) freeUndoRedoManager(eng->undoRedo);
    free(eng);
}
