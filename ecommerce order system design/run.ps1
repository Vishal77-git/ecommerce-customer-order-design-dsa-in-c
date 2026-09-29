# PowerShell runner for E-Commerce DSA System (C)
param (
    [switch]$Demo
)

$ErrorActionPreference = "Stop"

Write-Host "===================================================" -ForegroundColor Cyan
Write-Host "   Compiling and Running E-Commerce DSA System (C)" -ForegroundColor Cyan
Write-Host "===================================================" -ForegroundColor Cyan

Write-Host "Compiling C source files with GCC..." -ForegroundColor Yellow
gcc -Wall -Wextra -Iinclude src/product.c src/trie.c src/order.c src/order_queue.c src/undo_redo.c src/engine.c src/main.c -o ecommerce_dsa.exe

if ($LASTEXITCODE -eq 0) {
    Write-Host "Compilation successful! Running platform...`n" -ForegroundColor Green
    if ($Demo) {
        .\ecommerce_dsa.exe --demo
    } else {
        .\ecommerce_dsa.exe
    }
} else {
    Write-Host "Compilation failed!" -ForegroundColor Red
}
