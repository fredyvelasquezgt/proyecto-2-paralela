#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <openssl/des.h> // Include OpenSSL's DES header

void decrypt(long key, char *ciph, int len) {
    DES_cblock des_key;
    DES_key_schedule schedule;
    DES_key_sched((DES_cblock *)&key, &schedule);
    
    for (int i = 0; i < len; i += 8) {
        DES_ecb_encrypt((DES_cblock *)(ciph + i), (DES_cblock *)(ciph + i), &schedule, DES_DECRYPT);
    }
}

void encrypt(long key, char *ciph, int len) {
    DES_cblock des_key;
    DES_key_schedule schedule;
    DES_key_sched((DES_cblock *)&key, &schedule);
    
    for (int i = 0; i < len; i += 8) {
        DES_ecb_encrypt((DES_cblock *)(ciph + i), (DES_cblock *)(ciph + i), &schedule, DES_ENCRYPT);
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
    int N, id;
    long upper = (1L << 56); // Upper bound DES keys 2^56
    long mylower, myupper;
    MPI_Status st;
    MPI_Request req;
    int flag;
    int ciphlen = strlen(cipher);
    MPI_Comm comm = MPI_COMM_WORLD;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(comm, &N);
    MPI_Comm_rank(comm, &id);

    long range_per_node = upper / N;
    mylower = range_per_node * id;
    myupper = range_per_node * (id + 1) - 1;
    if (id == N - 1) {
        // Compensate for the residue
        myupper = upper;
    }

    long found = 0;
    int ready = 0;

    MPI_Irecv(&found, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &req);

    for (long i = mylower; i < myupper; ++i) {
        MPI_Test(&req, &ready, MPI_STATUS_IGNORE);
        if (ready)
            break; // Already found, exit

        if (tryKey(i, (char *)cipher, ciphlen)) {
            found = i;
            for (int node = 0; node < N; node++) {
                MPI_Send(&found, 1, MPI_LONG, node, 0, MPI_COMM_WORLD);
            }
            break;
        }
    }

    if (id == 0) {
        MPI_Wait(&req, &st);
        decrypt(found, (char *)cipher, ciphlen);
        printf("%li %s\n", found, cipher);
    }

    MPI_Finalize();
}
