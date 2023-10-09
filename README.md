# Proyecto 2 - Computacion Paralela

## Integrantes 

- Alejandro Gomez
- Roberto Vallecillos
- Fredy Velasquez

## Parte A

El programa arreglado es `brutalforce_fix.c`. Para ejecutar dicho programa se deben de seguir las siguientes instrucciones:

### Ejecucion

1. Asegurarse de contar con las instalaciones correspondientes. Para instalar lo requerido por el programa ejecutar el siguiente comando:

`sudo apt-get install mpich libssl-dev`

2. Compilar utilizando el comando:

`mpicc -o bruteforce bruteforce.c -lssl -lcrypto`

3. Ejecucion. El siguiente comando ejecuta el programa con 4 procesos 

`mpirun -n 4 ./bruteforce`



