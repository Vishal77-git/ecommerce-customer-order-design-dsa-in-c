# Directive: E-Commerce Platform Data Structures & Algorithms System (Pure C)

## 1. Goal
Design and implement an integrated e-commerce backend platform in **pure C** satisfying three core requirements:
1. Process customer orders in strict placement sequence (FIFO).
2. Support reversible cancellations and order edits (Undo/Redo via LIFO Stack).
3. Deliver high-speed product search across a large catalog (Prefix search via Trie & direct lookup via Hash Table).

## 2. Component Specifications & Data Structure Justifications

| Component | Primary Data Structure | Auxiliary Data Structure | Time Complexity | Space Complexity | Technical Justification |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Order Processing** | Doubly Linked List (FIFO Queue) | Hash Map (`orderId -> DoublyLinkedNode*`) | Enqueue: $O(1)$<br>Dequeue: $O(1)$<br>Cancel/Edit: $O(1)$ | $O(N)$ where $N$ is pending orders | A pure queue requires $O(N)$ search to cancel or edit an arbitrary pending order. Pairing a Doubly Linked List with a Hash Map enables $O(1)$ node lookup and $O(1)$ pointer relinking without shifting array elements. |
| **Reversibility** | Stack (LIFO) | Redo Stack (LIFO) | Push: $O(1)$<br>Undo: $O(1)$<br>Redo: $O(1)$ | $O(M)$ where $M$ is modification history depth | LIFO semantics match reversible operations. The Command/Memento pattern records state diffs. Popping the top action restores prior state and reinserts/updates orders in the queue deterministically. |
| **Product Search** | Prefix Trie | Hash Table (`productId -> Product*`) | Prefix Search: $O(L + K)$<br>Direct ID Lookup: $O(1)$ | $O(\Sigma \cdot \text{total chars})$ where $\Sigma$ is alphabet size | Searching $N$ products via linear string comparison is $O(N \cdot L)$. A Trie decouples search time from catalog size $N$, depending only on query length $L$ and matching count $K$. |

## 3. Directory Layout
```
capstone project/
├── include/
│   ├── product.h        # Product struct & functions
│   ├── trie.h           # TrieNode & ProductHashTable
│   ├── order.h          # Order & OrderItem structs, deep clone
│   ├── order_queue.h    # DoublyLinkedNode, OrderMap, & OrderQueue
│   ├── undo_redo.h      # OrderAction, ActionStack, & UndoRedoManager
│   └── engine.h         # Integrated ECommerceEngine facade
├── src/
│   ├── product.c
│   ├── trie.c
│   ├── order.c
│   ├── order_queue.c
│   ├── undo_redo.c
│   ├── engine.c
│   └── main.c           # Comprehensive test runner
├── run.bat              # Batch compilation & execution
├── run.ps1              # PowerShell runner
└── directives/
    └── ecommerce_dsa_system.md
```

## 4. Compilation & Execution
- **Using PowerShell**:
  ```powershell
  .\run.ps1
  ```
- **Using Command Prompt**:
  ```cmd
  .\run.bat
  ```
- **Manual GCC Compilation**:
  ```powershell
  gcc -Wall -Wextra -Iinclude src/product.c src/trie.c src/order.c src/order_queue.c src/undo_redo.c src/engine.c src/main.c -o ecommerce_dsa.exe
  .\ecommerce_dsa.exe
  ```
