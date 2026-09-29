#ifndef UNDO_REDO_H
#define UNDO_REDO_H

#include "order.h"
#include <stdbool.h>

typedef enum {
    ACTION_PLACE_ORDER,
    ACTION_EDIT_ORDER,
    ACTION_CANCEL_ORDER
} ActionType;

/**
 * Encapsulates an order action with deep-copied memento snapshots.
 */
typedef struct OrderAction {
    ActionType type;
    char orderId[32];
    Order* stateBefore;
    Order* stateAfter;
    int positionInQueue;
    char description[128];
} OrderAction;

typedef struct ActionStack {
    OrderAction** data;
    int top;
    int capacity;
} ActionStack;

typedef struct UndoRedoManager {
    ActionStack* undoStack;
    ActionStack* redoStack;
} UndoRedoManager;

OrderAction* createOrderAction(ActionType type, const char* orderId,
                              const Order* before, const Order* after,
                              int position, const char* desc);
void freeOrderAction(OrderAction* action);

UndoRedoManager* createUndoRedoManager(int capacity);
void recordAction(UndoRedoManager* mgr, ActionType type, const char* orderId,
                  const Order* before, const Order* after, int position, const char* desc);
bool canUndo(const UndoRedoManager* mgr);
bool canRedo(const UndoRedoManager* mgr);
OrderAction* popUndo(UndoRedoManager* mgr);
OrderAction* popRedo(UndoRedoManager* mgr);
void pushUndo(UndoRedoManager* mgr, OrderAction* action);
void pushRedo(UndoRedoManager* mgr, OrderAction* action);
void freeUndoRedoManager(UndoRedoManager* mgr);

#endif // UNDO_REDO_H
