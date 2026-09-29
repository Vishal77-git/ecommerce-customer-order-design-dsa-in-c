@echo off
echo ===================================================
echo   Compiling and Running E-Commerce DSA System (C)
echo ===================================================

echo Compiling C source files with GCC...
gcc -Wall -Wextra -Iinclude src\product.c src\trie.c src\order.c src\order_queue.c src\undo_redo.c src\engine.c src\main.c -o ecommerce_dsa.exe

if %ERRORLEVEL% NEQ 0 (
    echo Compilation failed!
    pause
    exit /b %ERRORLEVEL%
)

echo Compilation successful! Running binary...
echo.
ecommerce_dsa.exe
echo.
pause
