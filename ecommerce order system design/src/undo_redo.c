#include "undo_redo.h"
#include <stdlib.h>
#include <string.h>

OrderAction* createOrderAction(ActionType type, const char* orderId,
                              const Order* before, const Order* after,
                              int position, const char* desc) {
    OrderAction* a = (OrderAction*)malloc(sizeof(OrderAction));
    if (!a) return NULL;
    a->type = type;
    strncpy(a->orderId, orderId ? orderId : "", sizeof(a->orderId) - 1);
    a->orderId[sizeof(a->orderId) - 1] = '\0';
    a->stateBefore = before ? cloneOrder(before) : NULL;
    a->stateAfter = after ? cloneOrder(after) : NULL;
    a->positionInQueue = position;
    strncpy(a->description, desc ? desc : "", sizeof(a->description) - 1);
    a->description[sizeof(a->description) - 1] = '\0';
    return a;
}

void freeOrderAction(OrderAction* action) {
    if (action) {
        if (action->stateBefore) freeOrder(action->stateBefore);
        if (action->stateAfter) freeOrder(action->stateAfter);
        free(action);
    }
}

static ActionStack* createActionStack(int capacity) {
    ActionStack* s = (ActionStack*)malloc(sizeof(ActionStack));
    if (!s) return NULL;
    s->capacity = capacity > 0 ? capacity : 50;
    s->top = -1;
    s->data = (OrderAction**)malloc(sizeof(OrderAction*) * s->capacity);
    return s;
}

static void clearStack(ActionStack* s) {
    if (!s) return;
    while (s->top >= 0) {
        freeOrderAction(s->data[s->top--]);
    }
}

static void freeActionStack(ActionStack* s) {
    if (!s) return;
    clearStack(s);
    free(s->data);
    free(s);
}

UndoRedoManager* createUndoRedoManager(int capacity) {
    UndoRedoManager* mgr = (UndoRedoManager*)malloc(sizeof(UndoRedoManager));
    if (!mgr) return NULL;
    mgr->undoStack = createActionStack(capacity);
    mgr->redoStack = createActionStack(capacity);
    return mgr;
}

void recordAction(UndoRedoManager* mgr, ActionType type, const char* orderId,
                  const Order* before, const Order* after, int position, const char* desc) {
    if (!mgr) return;
    OrderAction* action = createOrderAction(type, orderId, before, after, position, desc);
    if (!action) return;

    // Check capacity for undoStack
    if (mgr->undoStack->top >= mgr->undoStack->capacity - 1) {
        // Drop bottom of stack
        freeOrderAction(mgr->undoStack->data[0]);
        for (int i = 0; i < mgr->undoStack->top; i++) {
            mgr->undoStack->data[i] = mgr->undoStack->data[i + 1];
        }
        mgr->undoStack->top--;
    }

    mgr->undoStack->data[++mgr->undoStack->top] = action;

    // New action invalidates redo branch
    clearStack(mgr->redoStack);
}

bool canUndo(const UndoRedoManager* mgr) {
    return mgr && mgr->undoStack && mgr->undoStack->top >= 0;
}

bool canRedo(const UndoRedoManager* mgr) {
    return mgr && mgr->redoStack && mgr->redoStack->top >= 0;
}

OrderAction* popUndo(UndoRedoManager* mgr) {
    if (!canUndo(mgr)) return NULL;
    return mgr->undoStack->data[mgr->undoStack->top--];
}

OrderAction* popRedo(UndoRedoManager* mgr) {
    if (!canRedo(mgr)) return NULL;
    return mgr->redoStack->data[mgr->redoStack->top--];
}

void pushUndo(UndoRedoManager* mgr, OrderAction* action) {
    if (!mgr || !action || mgr->undoStack->top >= mgr->undoStack->capacity - 1) return;
    mgr->undoStack->data[++mgr->undoStack->top] = action;
}

void pushRedo(UndoRedoManager* mgr, OrderAction* action) {
    if (!mgr || !action || mgr->redoStack->top >= mgr->redoStack->capacity - 1) return;
    mgr->redoStack->data[++mgr->redoStack->top] = action;
}

void freeUndoRedoManager(UndoRedoManager* mgr) {
    if (!mgr) return;
    freeActionStack(mgr->undoStack);
    freeActionStack(mgr->redoStack);
    free(mgr);
}
