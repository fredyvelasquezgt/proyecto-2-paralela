#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <openssl/des.h>

void decrypt(long key, unsigned char *ciph, int len){
    // Set parity of key and do decrypt
    long k = 0;
    for(int i=0; i<8; ++i){
        key <<= 1;
        k += (key & (0xFE << i*8));
    }
    DES_key_schedule schedule;
    DES_set_key((const_DES_cblock *)&k, &schedule);
    DES_ecb_encrypt((const_DES_cblock *)ciph, (DES_cblock *)ciph, &schedule, DES_DECRYPT);
}

char search[] = " the ";
int tryKey(long key, unsigned char *ciph, int len){
    unsigned char temp[len+1];
    memcpy(temp, ciph, len);
    temp[len] = 0;
    decrypt(key, temp, len);
    return strstr((char *)temp, search) != NULL;
}

unsigned char cipher[] = {108, 245, 65, 63, 125, 200, 150, 66, 17, 170, 207, 170, 34, 31, 70, 215, 0};

int main(int argc, char *argv[]){
    int N, id;
    long upper = (1L << 56); // Upper bound DES keys 2^56
    long mylower, myupper;
    MPI_Status st;
    MPI_Request req;
    int ciphlen = sizeof(cipher)-1; // 17 as calculated before
    MPI_Comm comm = MPI_COMM_WORLD;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(comm, &N);
    MPI_Comm_rank(comm, &id);

    int range_per_node = upper / N;
    mylower = range_per_node * id;
    myupper = range_per_node * (id+1) - 1;
    if(id == N-1){
        myupper = upper; // Compensate residue
    }

    long found = 0;
    MPI_Irecv(&found, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &req);

    for(int i = mylower; i < myupper && (found == 0); ++i){
        if(tryKey(i, cipher, ciphlen)){
            found = i;
            for(int node = 0; node < N; node++){
                MPI_Send(&found, 1, MPI_LONG, node, 0, MPI_COMM_WORLD);
            }
            break;
        }
    }

    if(id == 0){
        MPI_Wait(&req, &st);
        decrypt(found, cipher, ciphlen);
        printf("%li %s\n", found, cipher);
    }

    MPI_Finalize();
}

