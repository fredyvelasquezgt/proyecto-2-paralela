#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <openssl/des.h> 

long heuristicSearch(int id, int N, char *plaintext, int ciphlen, MPI_Comm comm)
{
    long found = 0;
    long mask = ~((1 << 10) - 1); // Uso de herisircas. Utiliza una mascara para la llave y tratar de encontrar la llave.

    for (long i = id; i < (1L << 56); i += N)
    {
        long heuristicKey = i & mask;
        if (tryKey(heuristicKey, plaintext, ciphlen))
        {
            found = heuristicKey;
            for (int node = 0; node < N; node++)
            {
                MPI_Send(&found, 1, MPI_LONG, node, 0, comm);
            }
            return found;
        }
    }
    return found;
}

long searchForDesKey(int id, int N, char *plaintext, int ciphlen, MPI_Comm comm)
{
    return heuristicSearch(id, N, plaintext, ciphlen, comm); //BUsqueda de llave
}

void decrypt(long key, char *ciph, int len) //DEsencriptar
{
    DES_cblock des_key;
    DES_key_schedule schedule;
    DES_key_sched((DES_cblock *)&key, &schedule);

    for (int i = 0; i < len; i += 8)
    {
        DES_ecb_encrypt((DES_cblock *)(ciph + i), (DES_cblock *)(ciph + i), &schedule, DES_DECRYPT);
    }
}

void encrypt(long key, unsigned char *plain, int len) //ENcriptar
{
    long k = 0;
    for (int i = 0; i < 8; ++i)
    {
        key <<= 1;
        k += (key & (0xFE << i * 8));
    }
    DES_key_schedule schedule;
    DES_set_key((const DES_cblock *)&k, &schedule);
    for (int i = 0; i < len; i += 8)
    {
        DES_ecb_encrypt((const DES_cblock *)(plain + i), (DES_cblock *)(plain + i), &schedule, DES_ENCRYPT);
    }
}

char search[] = "es una prueba de";
int tryKey(long key, char *ciph, int len) //BUscar llave
{
    char temp[len + 1];
    memcpy(temp, ciph, len);
    temp[len] = 0;
    decrypt(key, temp, len);
    return strstr(temp, search) != NULL;
}

unsigned char cipher[] = {108, 245, 65, 63, 125, 200, 150, 66, 17, 170, 207, 170, 34, 31, 70, 215, 0};
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s <path_to_text_file> <encryption_key>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    long encryption_key = atol(argv[2]);

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
    MPI_Status st;
    MPI_Comm comm = MPI_COMM_WORLD;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(comm, &N);
    MPI_Comm_rank(comm, &id);

    encrypt(encryption_key, plaintext, fsize);
    printf("Encrypted text (Node %d): %s\n", id, plaintext);

    int ciphlen = fsize;

    long found = searchForDesKey(id, N, plaintext, ciphlen, comm);

    if (id == 0)
    {
        decrypt(found, plaintext, ciphlen);
        printf("Decrypted text (Node %d): %s\n", id, plaintext);
    }

    free(plaintext);
    MPI_Finalize();
    return 0;
}