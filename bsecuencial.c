#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/des.h> 
#include <time.h> 

void decrypt(long key, char *ciph, int len) { //FUNCIÓN DE ENCRIPTAR
    DES_cblock des_key;
    DES_key_schedule schedule;
    DES_key_sched((DES_cblock *)&key, &schedule);
    
    for (int i = 0; i < len; i += 8) {
        DES_ecb_encrypt((DES_cblock *)(ciph + i), (DES_cblock *)(ciph + i), &schedule, DES_DECRYPT);
    }
}

char search[] = " the ";
int tryKey(long key, char *ciph, int len) {
    char temp[len + 1];
    memcpy(temp, ciph, len);
    temp[len] = 0;
    decrypt(key, temp, len);
    return strstr(temp, search) != NULL;
}

unsigned char cipher[] = {108, 245, 65, 63, 125, 200, 150, 66, 17, 170, 207, 170, 34, 31, 70, 215, 0};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <key_length>\n", argv[0]);
        return 1;
    }

    int key_length = atoi(argv[1]);
    if (key_length > 56 || key_length < 1) {
        fprintf(stderr, "Key length must be between 1 and 56\n");
        return 1;
    }

    // Calculo de upper bound
    long upper = (1L << key_length); // Llave es 2^size
    int ciphlen = sizeof(cipher) - 1; // tamaño del len
    long found = -1; // LLave

    // Start measuring time
    clock_t begin = clock();

    for (long i = 0; i < upper; ++i) {
        if (tryKey(i, (char *)cipher, ciphlen)) {
            found = i;
            break;
        }
    }

    // Medir hasta que encuentra la llave.
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    if (found != -1) {
        decrypt(found, (char *)cipher, ciphlen);
        printf("Key found: %li\nDecrypted text: %s\n", found, cipher); //DEscirar texto.
    } else {
        printf("Key not found within the range.\n");
    }

    printf("Time taken for the search: %f seconds\n", time_spent);

    return 0;
}
