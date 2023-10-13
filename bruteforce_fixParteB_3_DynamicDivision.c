#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <openssl/des.h>

long dynamicDivisionOfWork(int id, int N, char *plaintext, int ciphlen, MPI_Comm comm)
{
    long found = 0;
    long chunkSize = 1000000;
    long currentLower = 0;

    while (!found && currentLower < (1L << 56))
    {
        for (long i = currentLower; i < currentLower + chunkSize && i < (1L << 56); ++i)
        {
            if (tryKey(i, plaintext, ciphlen))
            {
                found = i;
                for (int node = 0; node < N; node++)
                {
                    MPI_Send(&found, 1, MPI_LONG, node, 0, comm);
                }
                return found;
            }
        }
        currentLower += chunkSize * N;
    }
    return found;
}

long searchForDesKey(int id, int N, char *plaintext, int ciphlen, MPI_Comm comm)
{
    return dynamicDivisionOfWork(id, N, plaintext, ciphlen, comm);
}

void decrypt(long key, char *ciph, int len)
{
    DES_cblock des_key;
    DES_key_schedule schedule;
    DES_key_sched((DES_cblock *)&key, &schedule);

    for (int i = 0; i < len; i += 8)
    {
        DES_ecb_encrypt((DES_cblock *)(ciph + i), (DES_cblock *)(ciph + i), &schedule, DES_DECRYPT);
    }
}

void encrypt(long key, unsigned char *plain, int len)
{
    DES_key_schedule schedule;
    DES_set_key((const DES_cblock *)&key, &schedule);

    for (int i = 0; i < len; i += 8)
    {
        DES_ecb_encrypt((const DES_cblock *)(plain + i), (DES_cblock *)(plain + i), &schedule, DES_ENCRYPT);
    }
}

char search[] = "es una prueba de";
int tryKey(long key, char *ciph, int len)
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
    plaintext[fsize] = '\0';

    int N, id;
    MPI_Status st;
    MPI_Comm comm = MPI_COMM_WORLD;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(comm, &N);
    MPI_Comm_rank(comm, &id);

    encrypt(encryption_key, plaintext, fsize);
    printf("Encrypted text (Node %d): %s\n", id, plaintext);

    long found = searchForDesKey(id, N, plaintext, fsize, comm);

    if (id == 0)
    {
        decrypt(found, plaintext, fsize);
        printf("Decrypted text (Node %d): %s\n", id, plaintext);
    }

    free(plaintext);
    MPI_Finalize();
    return 0;
}
