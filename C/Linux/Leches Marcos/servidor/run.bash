#!/bin/bash

echo "=== INICIANDO SISTEMA TIENDA ==="

# Compilar
gcc -o tienda_server programa_principal.c -pthread -lncurses -luuid

echo "1. Servidor iniciando en segundo plano..."
./tienda_server &

echo "2. Abriendo consola administrativa en 3 segundos..."
sleep 3

echo "3. Iniciando interfaz admin..."
./tienda_server

echo "4. Cerrando sistema..."