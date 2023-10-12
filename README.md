# Proyecto 2 - Computacion Paralela

## Integrantes 

- Alejandro Gomez
- Roberto Vallecillos
- Fredy Velasquez

## Parte A

El programa arreglado es `brutalforce_fix2.c`. Para ejecutar dicho programa se deben de seguir las siguientes instrucciones:

### Ejecucion


1. Compilar utilizando el comando:

`mpicc -o bruteforce00 bruteforce00.c -I/path/to/openssl/include -lcrypto`

3. Ejecucion. El siguiente comando ejecuta el programa con 4 procesos 

`mpirun -np 4 ./bruteforce00`



