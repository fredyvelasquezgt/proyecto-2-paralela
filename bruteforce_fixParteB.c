#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <openssl/des.h> 

void decrypt(long key, char *ciph, int len) //Descriptar
{
    DES_key_schedule schedule;
    DES_set_key((const_DES_cblock *)&key, &schedule);

    for (int i = 0; i < len; i += 8)
    {
        DES_ecb_encrypt((DES_cblock *)(ciph + i), (DES_cblock *)(ciph + i), &schedule, DES_DECRYPT);
    }
}

void encrypt(long key, unsigned char *plain, int len) //ENcriptar
{
    DES_key_schedule schedule;
    DES_set_key((const_DES_cblock *)&key, &schedule);
    for (int i = 0; i < len; i += 8)
    {
        DES_ecb_encrypt((const_DES_cblock *)(plain + i), (DES_cblock *)(plain + i), &schedule, DES_ENCRYPT);
    }
}

char search[] = "es una prueba de";
int tryKey(long key, char *ciph, int len) //POrbar llave
{
    char temp[len + 1];
    memcpy(temp, ciph, len);
    temp[len] = 0;
    decrypt(key, temp, len);
    return strstr(temp, search) != NULL;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s <path_to_text_file> <encryption_key>\n", argv[0]); //INput
        return 1;
    }

    char *filename = argv[1];
    long encryption_key = atol(argv[2]);
    double start_time, end_time;

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Failed to open file");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *plaintext = malloc(fsize + 1);
    fread(plaintext, 1, fsize, file);
    fclose(file);
    plaintext[fsize] = '\0'; // Ensure null-termination

    int N, id;
    long upper = (1L << 56); // Upper bound DES keys 2^56
    int ready = 0;
    MPI_Comm comm = MPI_COMM_WORLD;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &N);
    MPI_Comm_rank(comm, &id);
    start_time = MPI_Wtime();

    encrypt(encryption_key, plaintext, fsize);
    printf("Encrypted text (Node %d): %s\n", id, plaintext);

    int ciphlen = fsize;

    long range_per_node = upper / N;
    long mylower = range_per_node * id;
    long myupper = range_per_node * (id + 1) - 1;
    if (id == N - 1)
    {
        myupper = upper;
    }

    long found = 0;
    MPI_Request req;
    MPI_Irecv(&found, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &req);

    for (long i = mylower; i < myupper; ++i)
    {
        MPI_Test(&req, &ready, MPI_STATUS_IGNORE);
        if (ready)
            break; // Already found, exit

        if (tryKey(i, plaintext, ciphlen))
        {
            found = i;
            for (int node = 0; node < N; node++)
            {
                MPI_Send(&found, 1, MPI_LONG, node, 0, MPI_COMM_WORLD); //Paralelización usando MPI
            }
            break;
        }
    }
     

end_time = MPI_Wtime();
    if (id == 0)
    {
        decrypt(found, plaintext, ciphlen);
        printf("Decrypted text (Node %d): %s\n", id, plaintext);
        printf("Time elapsed: %f seconds\n", end_time - start_time); //MEDICIṔN DE TIEMPO Y IMPRIMIR LA LLAVE.
    }

    free(plaintext);
    MPI_Finalize();

    return 0;
}