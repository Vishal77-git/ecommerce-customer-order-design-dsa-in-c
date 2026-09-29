# Capstone Project: High-Performance E-Commerce Data Structures & Algorithms Engine

> **Course:** 23CSE203 Data Structures and Algorithms (DSA)  
> **Language:** Pure C (C99 standard, Compiled via GCC)  
> **Project Scope:** Integrated system for chronological order processing, reversible operations (undo/redo), and instant prefix catalog search.

---

## 📑 Table of Contents (Topic Locator Index)

Click on any topic below to jump directly to that section:

1. [1. Executive Summary](#1-executive-summary)
2. [2. High-Level Workflow](#2-high-level-workflow)
3. [3. Inputs to the System](#3-inputs-to-the-system)
4. [4. Database & In-Memory Data Storage Architecture](#4-database--in-memory-data-storage-architecture)
5. [5. Outputs & State Visualizations](#5-outputs--state-visualizations)
6. [6. System Initialization Routine](#6-system-initialization-routine)
7. [7. Data Structures & Algorithms Used (Technical Justification)](#7-data-structures--algorithms-used-in-depth-technical-justification)
8. [8. Is This Project Menu-Driven? (Options 0–11 In-Depth Breakdown)](#8-is-this-project-menu-driven)
   - [Option 1: List All Available Products & Details](#option-1-list-all-available-products--details-hash-table-traversal---on)
   - [Option 2: Search Products by Prefix](#option-2-search-products-by-prefix-trie---ol--k)
   - [Option 3: Lookup Product by ID/SKU](#option-3-lookup-product-by-idsku-hash-table---o1)
   - [Option 4: Place a New Customer Order](#option-4-place-a-new-customer-order-fifo-queue-enqueue---o1)
   - [Option 5: Edit Item Quantity in Pending Order](#option-5-edit-item-quantity-in-pending-order-o1-lookup--memento-snapshot)
   - [Option 6: Cancel a Pending Order](#option-6-cancel-a-pending-order-o1-dll-unlink--undo-record)
   - [Option 7: Undo Last Action](#option-7-undo-last-action-lifo-stack---o1)
   - [Option 8: Redo Last Action](#option-8-redo-last-action-lifo-stack---o1)
   - [Option 9: Process Next Order in Sequence](#option-9-process-next-order-in-sequence-fifo-head-dequeue---o1)
   - [Option 10: View Current FIFO Order Queue](#option-10-view-current-fifo-order-queue)
   - [Option 11: Run Full Automated Verification Demo](#option-11-run-full-automated-verification-demo)
   - [Option 0: Exit Program](#option-0-exit-program)
9. [9. How to Run the Program to Get Results](#9-how-to-run-the-program-to-get-results)
10. [10. Deep-Dive: How Searching Products Works in the Background](#10-deep-dive-how-searching-products-works-in-the-background)
11. [11. Viva / Review Q&A Cheat Sheet](#11-viva--review-qa-cheat-sheet)

---

## 1. Executive Summary

This project designs and implements an e-commerce backend platform in **pure C** addressing three fundamental computational challenges:
1. **FIFO Sequence Order Processing:** Processing customer orders strictly in the chronological sequence they were placed without starvation.
2. **Reversible Mutations (Undo / Redo):** Enabling order edits and cancellations to be instantly reversible, restoring canceled orders back to their exact original queue positions.
3. **High-Speed Product Catalog Search:** Providing sub-millisecond prefix search (autocomplete) and exact SKU lookups over a catalog without performing costly linear scans.

---

## 2. High-Level Workflow

The engine acts as a centralized orchestrator connecting three discrete subsystems:

```
[Customer / Admin Input]
          │
          ├── 1. Prefix Search ────────> [Trie Index]
          │                                  │ Matches prefix tokens in O(L + K)
          │                                  ▼ Returns Product pointers
          ├── 2. Direct Lookup ────────> [Product Hash Table]
          │                                  │ O(1) direct SKU retrieval
          │                                  ▼
          ├── 3. Place Order ──────────> [FIFO Order Queue]
          │                                  │ Appends node to DLL tail in O(1)
          │                                  │ Maps orderId -> DLL Node in O(1)
          │                                  ▼
          ├── 4. Edit / Cancel Order ──> [OrderMap + DLL Unlink + Undo Stack]
          │                                  │ 1. O(1) DLL unlinking/in-place edit
          │                                  │ 2. Deep-copies Memento snapshot
          │                                  │ 3. Pushes action to LIFO Undo Stack
          │                                  ▼
          ├── 5. Undo / Redo Request ──> [Undo / Redo Stacks]
          │                                  │ Pops top action in O(1)
          │                                  │ Re-inserts/restores order at exact slot
          │                                  ▼
          └── 6. Dispatch / Process ───> [Queue Head Dequeue]
                                             │ Dequeues oldest order in O(1)
                                             │ Updates live product inventory
```

---

## 3. Inputs to the System

The platform accepts inputs either interactively through the CLI menu or through the automated demonstration pipeline:

| Input Category | Parameters | Data Types | Example Values |
| :--- | :--- | :--- | :--- |
| **Catalog Products** | `id`, `name`, `category`, `price`, `stock` | `char[]`, `char[]`, `char[]`, `double`, `int` | `"PROD-01"`, `"Apple iPhone 15 Pro"`, `"Smartphones"`, `999.99`, `50` |
| **Search Query** | `prefix` | `char[]` | `"app"`, `"son"`, `"lap"`, `"gam"` |
| **Direct Lookup** | `productId` | `char[]` | `"PROD-04"` |
| **Order Placement** | `customerId`, `items` (productId, qty) | `char[]`, `OrderItem[]` | `"CUST-ALICE"`, `[{"PROD-01", 1}]` |
| **Order Quantity Edit**| `orderId`, `productId`, `newQuantity` | `char[]`, `char[]`, `int` | `"ORD-1002"`, `"PROD-05"`, `4` |
| **Order Cancellation** | `orderId` | `char[]` | `"ORD-1003"` |
| **Reversibility** | Menu command trigger | `int` | Choice `7` (Undo), Choice `8` (Redo) |

---

## 4. Database & In-Memory Data Storage Architecture

To achieve $O(1)$ and $O(L)$ asymptotic performance guarantees, the system uses an **in-memory database architecture** consisting of four interconnected data structures:

```
┌────────────────────────────────────────────────────────────────────────┐
│                        IN-MEMORY DATA ENGINE                           │
├───────────────────────────────┬────────────────────────────────────────┤
│ Catalog Storage               │ Order Queue & Operations Storage       │
│                               │                                        │
│ 1. ProductCatalog (Trie):     │ 3. OrderQueue (Doubly Linked List):    │
│    Root TrieNode with 128     │    Chronological sequence from head    │
│    ASCII pointer branches     │    (oldest) to tail (newest).          │
│                               │                                        │
│ 2. ProductHashTable:          │ 4. OrderMap (Hash Table):              │
│    Chained hash table mapping │    Separate-chaining hash table mapping│
│    productId -> Product*      │    orderId -> DoublyLinkedNode*        │
│                               │                                        │
│                               │ 5. UndoRedoManager (Dual LIFO Stacks): │
│                               │    Array-based dynamic stacks storing  │
│                               │    OrderAction memento snapshots.      │
└───────────────────────────────┴────────────────────────────────────────┘
```

* **No external SQL/NoSQL bottlenecks:** Eliminates disk I/O latency, achieving million-operations-per-second throughput.
* **Deep Memento Clones:** Ensures complete memory isolation between historical snapshots and active state.

---

## 5. Outputs & State Visualizations

1. **Search Results:** Formatted list of all catalog products whose name or category matches the queried prefix.
2. **Queue State View:** Displays the pending queue in order of arrival, listing zero-based slot indices, order IDs, customer names, and total monetary amounts.
3. **Order State Cards:** Detailed breakdown of an individual order showing status (`PENDING`, `PROCESSED`, `CANCELLED`), line items, quantities, unit prices, and subtotals.
4. **Undo/Redo Feedback:** Explicit status messages indicating the nature of the reversed or re-applied operation (e.g., `"Restored Order #ORD-1003 to queue slot 2"`).
5. **Inventory Audit:** Live updates showing inventory deducted from product stock once an order is dispatched.

---

## 6. System Initialization Routine

When the program boots up (`main()` in `src/main.c`):
1. **Engine Allocation:** Calls `createEngine()`, allocating memory for:
   - Prefix Trie root node (`createTrieNode()`).
   - Catalog Hash Table with bucket size 101 (`createProductHashTable(101)`).
   - Order Queue Doubly Linked List with hash index (`createOrderQueue(101)`).
   - Undo/Redo Stacks with 50-action history capacity (`createUndoRedoManager(50)`).
2. **Catalog Seeding:** Invokes `populateDefaultCatalog()`, registering 7 baseline products across Smartphones, Laptops, Audio, and Gaming categories.
3. **Token Indexing:** For every seeded product, the engine tokenizes its title and category, inserting each token into the Trie in $O(\text{words} \times \text{length})$ time.
4. **Execution Mode Selection:**
   - If invoked with `--demo`, executes the automated verification script.
   - If invoked without arguments, launches the interactive 12-option terminal menu (Options 0–11).

---

## 7. Data Structures & Algorithms Used (In-Depth Technical Justification)

| Requirement | Data Structure Chosen | Primary Operations & Time Complexity | Space Complexity | Why This Data Structure? (Academic Justification) |
| :--- | :--- | :--- | :--- | :--- |
| **Order Processing (Sequence Preservation)** | **Doubly Linked List + Hash Map Index** | Enqueue: $O(1)$<br>Dequeue: $O(1)$<br>Lookup: $O(1)$<br>Cancel/Unlink: $O(1)$ | $O(N)$ where $N$ is pending orders | A pure queue or array requires $O(N)$ scanning to locate an arbitrary order for cancellation/edit, and array removal requires $O(N)$ memory shifts. Pairing a Doubly Linked List with a Hash Map (`orderId -> Node*`) enables $O(1)$ node location and $O(1)$ pointer relinking (`prev->next = next`). |
| **Reversible Edits & Cancellations** | **Dual LIFO Stacks (Command & Memento Pattern)** | Push: $O(1)$<br>Undo: $O(1)$<br>Redo: $O(1)$ | $O(M)$ where $M$ is history depth | Undo operations strictly require Last-In, First-Out (LIFO) semantics. Deep-copy mementos store snapshots of `stateBefore`, `stateAfter`, and `positionInQueue`. Popping from the undo stack restores previous quantities or re-inserts cancelled orders at their exact historical queue slot in $O(1)$ time. |
| **Fast Product Search Across Large Catalog** | **Prefix Trie (128 ASCII transitions) + Hash Table** | Prefix Search: $O(L + K)$<br>Direct SKU Lookup: $O(1)$ | $O(\Sigma \cdot \text{chars})$ where $\Sigma$ is alphabet size | Linear scanning takes $O(N \cdot L)$, which is unacceptable for large catalogs. In a Trie, search time depends solely on the length of the query prefix $L$ and result count $K$, completely independent of total catalog size $N$. The Hash Table provides $O(1)$ SKU lookup. |

---

## 8. Is This Project Menu-Driven?

**YES, ABSOLUTELY!** The platform offers a full **interactive terminal menu** with choices `0` through `10`.

```text
================================================================================
                 E-COMMERCE DSA PLATFORM - MAIN MENU
================================================================================
  1. Search Products by Prefix (Trie - O(L + K))
  2. Lookup Product by ID/SKU (Hash Table - O(1))
  3. Place a New Customer Order (Enqueue to FIFO Queue - O(1))
  4. Edit Item Quantity in Pending Order (O(1) Map Lookup & State Snapshot)
  5. Cancel a Pending Order (O(1) Map Lookup & DLL Unlink)
  6. Undo Last Action (LIFO Stack - O(1))
  7. Redo Last Action (LIFO Stack - O(1))
  8. Process Next Order in Sequence (FIFO Dequeue from Head - O(1))
  9. View Current FIFO Order Queue
 10. Run Full Automated Verification Demo
  0. Exit
--------------------------------------------------------------------------------
Enter choice [0-10]:
```

---

### Detailed Breakdown of Menu Options (0 through 10)

#### Option 1: Search Products by Prefix (Trie - $O(L + K)$)
* **Purpose:** Allows instant autocomplete search across all product names and categories using word prefixes.
* **Input Prompted:** String prefix (e.g., `app`, `son`, `lap`, `gam`).
* **Underlying Function:** `engineSearchCatalog(engine, prefix, &count)` $\to$ `searchTriePrefix()`.
* **DSA Mechanism:** Traverses down the Prefix Trie following ASCII transitions for each letter in the query string.
* **Time Complexity:** $O(L + K)$ where $L$ is prefix length and $K$ is the number of matching items returned. Completely independent of the total catalog size $N$.
* **Example Output:**
  ```text
  Searching prefix: "app"
     -> [ID: PROD-01 | Apple iPhone 15 Pro | Cat: Smartphones | $999.99 | Stock: 50]
     -> [ID: PROD-02 | Apple MacBook Pro M3 | Cat: Laptops | $1999.99 | Stock: 30]
     -> [ID: PROD-03 | Apple AirPods Max | Cat: Audio | $549.00 | Stock: 40]
  ```

---

#### Option 2: Lookup Product by ID/SKU (Hash Table - $O(1)$)
* **Purpose:** Fetches complete product details instantly when the exact product code (SKU) is known.
* **Input Prompted:** Product ID (e.g., `PROD-01`, `PROD-04`).
* **Underlying Function:** `engineGetProduct(engine, id)` $\to$ `findProductById()`.
* **DSA Mechanism:** Runs DJB2 hash algorithm on the ID string modulo table capacity, then searches the chained bucket.
* **Time Complexity:** $O(1)$ average time.
* **Example Output:**
  ```text
  Found in O(1): [ID: PROD-04 | Samsung Galaxy S24 Ultra | Cat: Smartphones | $1199.99 | Stock: 45]
  ```

---

#### Option 3: Place a New Customer Order (FIFO Queue Enqueue - $O(1)$)
* **Purpose:** Creates a customer order, calculates monetary totals, and appends it to the tail of the waiting line.
* **Input Prompted:**
  1. Customer Name/ID (e.g., `CUST-ALICE`)
  2. Product ID to order (e.g., `PROD-01`)
  3. Quantity (e.g., `2`)
* **Underlying Function:** `enginePlaceOrder(engine, customerId, items, count)` $\to$ `enqueueOrder()`.
* **DSA Mechanism:** 
  1. Allocates an `Order` struct and auto-increments unique order ID (`ORD-1001`, `ORD-1002`).
  2. Attaches a new `DoublyLinkedNode` at the tail of the list in $O(1)$.
  3. Inserts `orderId -> Node*` mapping into `OrderMap` in $O(1)$.
  4. Records `ACTION_PLACE_ORDER` onto the LIFO Undo stack.
* **Time Complexity:** $O(1)$.
* **Example Output:**
  ```text
  + Successfully placed order:
  Order #ORD-1001 [Customer: CUST-ALICE | Status: PENDING | Total: $999.99]
     - Apple iPhone 15 Pro (x1 @ $999.99 = $999.99)
  ```

---

#### Option 4: Edit Item Quantity in Pending Order ($O(1)$ Lookup & Memento Snapshot)
* **Purpose:** Allows a customer to change the quantity of an item in their order while it is still waiting in the queue.
* **Input Prompted:**
  1. Order ID (e.g., `ORD-1002`)
  2. Product ID (e.g., `PROD-05`)
  3. New quantity (e.g., `4` to increase, or `0` to remove the item)
* **Underlying Function:** `engineEditOrderItemQuantity(engine, orderId, prodId, newQty)`.
* **DSA Mechanism:**
  1. Finds order in $O(1)$ via `OrderMap`.
  2. Deep-copies a snapshot of `stateBefore`.
  3. Updates quantity in-place and recalculates `totalAmount`.
  4. Deep-copies `stateAfter` and pushes `ACTION_EDIT_ORDER` to Undo stack.
* **Time Complexity:** $O(1)$.
* **Example Output:**
  ```text
  + Order updated successfully. State recorded to Undo stack.
  Order #ORD-1002 [Customer: CUST-BOB | Status: PENDING | Total: $2148.96]
     - Sony WH-1000XM5 Headphones (x4 @ $399.99 = $1599.96)
     - Apple AirPods Max (x1 @ $549.00 = $549.00)
  ```

---

#### Option 5: Cancel a Pending Order ($O(1)$ DLL Unlink & Undo Record)
* **Purpose:** Cancels an order and immediately frees up queue capacity without making subsequent orders wait.
* **Input Prompted:** Order ID (e.g., `ORD-1003`).
* **Underlying Function:** `engineCancelOrder(engine, orderId)` $\to$ `removeOrderById()`.
* **DSA Mechanism:**
  1. Looks up the `DoublyLinkedNode*` in $O(1)$ from `OrderMap`.
  2. Records its zero-based position in the queue.
  3. Splices the node out of the doubly linked list by updating `node->prev->next` and `node->next->prev` in $O(1)$.
  4. Removes the key from `OrderMap`.
  5. Deep-copies a memento and pushes `ACTION_CANCEL_ORDER` onto the Undo stack.
* **Time Complexity:** $O(1)$ (No scanning, no array shifts!).
* **Example Output:**
  ```text
  + Order #ORD-1003 cancelled and removed in O(1) from FIFO queue.
    Action snapshot stored in Undo stack (can be reversed).
  ```

---

#### Option 6: Undo Last Action (LIFO Stack - $O(1)$)
* **Purpose:** Reverses the most recent modification (reverses order placement, edit, or cancellation).
* **Input Prompted:** None (triggers automatically).
* **Underlying Function:** `engineUndo(engine, messageBuf, bufSize)` $\to$ `popUndo()`.
* **DSA Mechanism:**
  1. Pops the top `OrderAction*` from the LIFO `undoStack` in $O(1)$.
  2. **If Cancel Action:** Restores the order by calling `insertOrderAt(queue, restoredOrder, positionInQueue)`, putting the customer back into their **exact former place in line**!
  3. **If Edit Action:** Reverts item quantities back to `stateBefore`.
  4. **If Place Action:** Unlinks the order from the queue.
  5. Pushes the action onto `redoStack` in $O(1)$ to allow redo.
* **Time Complexity:** $O(1)$.
* **Example Output:**
  ```text
  [UNDO SUCCESS]: Undid Cancellation: Restored Order #ORD-1003 to queue slot 2.
  ```

---

#### Option 7: Redo Last Action (LIFO Stack - $O(1)$)
* **Purpose:** Re-applies the action that was just undone.
* **Input Prompted:** None (triggers automatically).
* **Underlying Function:** `engineRedo(engine, messageBuf, bufSize)` $\to$ `popRedo()`.
* **DSA Mechanism:**
  1. Pops the top action from `redoStack` in $O(1)$.
  2. Re-applies the forward operation.
  3. Pushes action back onto `undoStack`.
* **Time Complexity:** $O(1)$.
* **Example Output:**
  ```text
  [REDO SUCCESS]: Redid Cancellation: Cancelled Order #ORD-1003 from queue.
  ```

---

#### Option 8: Process Next Order in Sequence (FIFO Head Dequeue - $O(1)$)
* **Purpose:** Dispatches and fulfills the oldest pending order in the queue, ensuring fair first-come, first-served delivery.
* **Input Prompted:** None.
* **Underlying Function:** `engineProcessNextOrder(engine)` $\to$ `dequeueOrder()`.
* **DSA Mechanism:**
  1. Pulls the order from `head` of the Doubly Linked List in $O(1)$.
  2. Advances `head = head->next`.
  3. Removes the entry from `OrderMap`.
  4. Automatically deducts ordered quantities from the live product catalog stock.
  5. Sets status to `ORDER_PROCESSED`.
* **Time Complexity:** $O(1)$.
* **Example Output:**
  ```text
  [ORDER FULFILLED - FIFO HEAD]:
  Order #ORD-1001 [Customer: CUST-ALICE | Status: PROCESSED | Total: $999.99]
     - Apple iPhone 15 Pro (x1 @ $999.99 = $999.99)
  ```

---

#### Option 9: View Current FIFO Order Queue
* **Purpose:** Displays an ordered list of all active orders currently waiting in line.
* **Input Prompted:** None.
* **Underlying Function:** `printOrderQueue(engine->orderQueue)`.
* **DSA Mechanism:** Traverses the Doubly Linked List from `head` to `tail`, printing each order's slot index, ID, customer name, and total price.
* **Time Complexity:** $O(N)$ where $N$ is pending orders.
* **Example Output:**
  ```text
  [FIFO Order Queue | Total Pending: 4]
     Slot 0: Order #ORD-1001 (Customer: CUST-ALICE, Total: $999.99)
     Slot 1: Order #ORD-1002 (Customer: CUST-BOB, Total: $2148.96)
     Slot 2: Order #ORD-1003 (Customer: CUST-CHARLIE, Total: $499.99)
     Slot 3: Order #ORD-1004 (Customer: CUST-DIANA, Total: $1999.99)
  ```

---

#### Option 10: Run Full Automated Verification Demo
* **Purpose:** Automatically executes the complete end-to-end verification sequence (Catalog search, order placement, editing, cancelling, undoing, redoing, and sequential processing) without requiring any typing.
* **Input Prompted:** None.
* **Underlying Function:** `runAutomatedDemo(engine)`.
* **DSA Mechanism:** Executes all component test suites sequentially and logs the internal data structure state after every step.
* **Time Complexity:** $O(N)$ overall.

---

#### Option 0: Exit Program
* **Purpose:** Gracefully terminates the application.
* **DSA Mechanism:** Calls `freeEngine()`, recursively freeing all allocated Trie nodes, hash buckets, linked list nodes, stack mementos, and products to ensure zero memory leaks.

---

## 9. How to Run the Program to Get Results

### Method 1: Using the PowerShell Runner (Recommended)
```powershell
# Interactive Menu Mode:
.\run.ps1

# Automated Demo Mode:
.\run.ps1 -Demo
```

### Method 2: Using the Windows Batch File
```cmd
.\run.bat
```

### Method 3: Direct GCC Compilation & Execution
```powershell
gcc -Wall -Wextra -Iinclude src/product.c src/trie.c src/order.c src/order_queue.c src/undo_redo.c src/engine.c src/main.c -o ecommerce_dsa.exe

# Run interactive menu:
.\ecommerce_dsa.exe

# Run automated test:
.\ecommerce_dsa.exe --demo
```

---

## 10. Deep-Dive: How Searching Products Works in the Background

### Step 1: Tokenization at Product Insertion Time
When `engineAddProduct()` is called for `PROD-01: "Apple iPhone 15 Pro"`:
1. The string is converted to lowercase: `"apple iphone 15 pro smartphones"`.
2. `strtok()` splits the text on whitespace and punctuation into distinct tokens: `["apple", "iphone", "15", "pro", "smartphones"]`.

### Step 2: Trie Traversal and Prefix Pointers
For each token (e.g., `"apple"`):
1. Start at `root` node.
2. For character `'a'` (ASCII 97):
   - Access `curr->children[97]`. If NULL, allocate a new `TrieNode`.
   - Add pointer `Product*` (`PROD-01`) to `curr->matchedProducts`.
3. For character `'p'` (ASCII 112):
   - Access `curr->children[112]`. If NULL, allocate a new `TrieNode`.
   - Add pointer `Product*` to `curr->matchedProducts`.
4. Repeat for `'p'`, `'l'`, `'e'`.
5. Mark `curr->isEndOfWord = true`.

```
                    [ROOT]
                   /      \
               'a'          's'
               /              \
             'p'              'o'
             /                  \
           'p'                  'n'
     (Path: "app")         (Path: "son")
     matchedProducts:      matchedProducts:
       - PROD-01 (iPhone)    - PROD-05 (Headphones)
       - PROD-02 (MacBook)   - PROD-06 (PS5)
       - PROD-03 (AirPods)
```

### Step 3: Fast Retrieval at Search Time
When the user enters the search prefix `"app"`:
1. The engine iterates over characters `'a'`, `'p'`, `'p'`.
2. Each transition is a **single memory dereference** (`curr = curr->children[(unsigned char)c]`).
3. Total transitions = length of query $L = 3$ operations.
4. Once the node corresponding to `"app"` is reached, the function immediately returns its `matchedProducts` array.
5. **No iteration over the catalog:** Whether there are 7 products or 10,000,000 products, searching `"app"` takes exactly 3 pointer jumps!

---

## 11. Viva / Review Q&A Cheat Sheet

### Q1: Why did you not use a standard Circular Array for the Queue?
> **Answer:** A circular array queue gives $O(1)$ enqueue and dequeue, but fails when cancellations or order edits happen in the middle of the queue. Locating an order in an array takes $O(N)$ linear scan, and removing it requires shifting up to $N$ elements ($O(N)$). Our hybrid **Doubly Linked List + Hash Map** provides $O(1)$ enqueue, $O(1)$ dequeue, and $O(1)$ random unlinking/editing of any queued order.

### Q2: How does undoing a cancellation restore an order to its original position in line?
> **Answer:** When an order is cancelled, we capture an `OrderAction` snapshot containing the order's zero-based `positionInQueue`. When the user triggers Undo, we pop the action from the LIFO Undo stack and call `insertOrderAt(queue, restoredOrder, positionInQueue)`. This splices the node back into its exact prior slot between its former neighbors, preventing the customer from losing their place in line.

### Q3: Why is a Trie better than standard String Matching (like `strstr()` or regex)?
> **Answer:** `strstr()` or linear scanning compares the pattern against every product name in the catalog, requiring $O(N \cdot L)$ time, which slows down as the catalog grows. A Trie organizes characters into a tree where common prefixes are shared. Search time is $O(L + K)$ (query length $L$ plus match count $K$), which is completely independent of the catalog size $N$.

### Q4: How is memory managed in C to prevent leaks?
> **Answer:** Every allocation has a dedicated destructor:
> - `freeProduct()` frees individual products.
> - `freeTrie()` recursively traverses and deallocates every Trie node and matched pointer array.
> - `freeOrderQueue()` unlinks and frees all doubly linked nodes and the hash map buckets.
> - `freeUndoRedoManager()` drains both undo and redo stacks, freeing all cloned mementos.
> - `freeEngine()` unifies and cleans up all subsystems on program termination.
