#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

enum{
    TAG_CONTROLE = 0,
    TAG_ELEICAO = 1
};

int main(int argc, char **argv)
{
    int myrank, //who am i
       numprocs; //how many process

    int buffer;
    int array_ativos[numprocs];

    MPI_Status status;
   
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);

    int coordenador = numprocs-1;

    if(myrank != 0) // processos que participam da eleicao 1 até numprocs-1
    {
        MPI_Recv(&array_ativos[0], numprocs, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if(status.MPI_TAG == TAG_CONTROLE)
        {
            printf("Sou o processo %d\n",myrank);
            printf("Recebi mensagem do controle!\nProcesso que caiu: %d\n",array_ativos[0]);

            // no caso de um processo que caiu voltar disparar eleicao com todos
            if(array_ativos[0] != 0)
            {
                // Construir a mensagem de eleicao
                for(int i=1, i<numprocs; i++)
                {
                    array_ativos[i] = 0;
                }
                array_ativos[1] = myrank;

                // enviar para o próximo
                if(myrank == numprocs-1) // sou o ultimo, envia para o processo 1 (0 é o de controle)
                    MPI_Send(&array_ativos[0], numprocs, MPI_INT, 1, TAG_ELEICAO, MPI_COMM_WORLD);
                else // se nao, envia para o próximo
                    MPI_Send(&array_ativos[0], numprocs, MPI_INT, myrank+1, TAG_ELEICAO, MPI_COMM_WORLD);

                // esperar pra receber o array completo
                MPI_Recv(&array_ativos[0], numprocs, MPI_INT, MPI_ANY_SOURCE, TAG_ELEICAO, MPI_COMM_WORLD, &status);

                printf("Recebi o array dos ativos: ");

                // buscar o maior id entre os ativos
                int maior = -1;
                for(int i=1, i<numprocs, i++)
                {
                    printf("%d ", array_ativos[i]);
                    if(array_ativos[i] > maior)
                    {
                        maior = array_ativos[i];
                    }
                }

                // eleger coordenador e montar mensagem para informar os outros
                coordenador = maior;
                if(myrank == numprocs -1)
                    MPI_Send(&coordenador, 1, MPI_INT, 1, TAG_ELEICAO, MPI_COMM_WORLD);
                else
                    MPI_Send(&coordenador, 1, MPI_INT, myrank+1, TAG_ELEICAO, MPI_COMM_WORLD);

                // esperar receber a mensagem de volta e enviar a confirmação para o controle
                MPI_Recv(&buffer, 1, MPI_INT, MPI_ANY_SOURCE, TAG_ELEICAO, MPI_COMM_WORLD, &status);
                MPI_Send(&buffer, 1, MPI_INT, 0, TAG_CONTROLE, MPI_COMM_WORLD);
            }
        }
        else(status.MPI_TAG == TAG_ELEICAO)
        {
            printf("Recebi mensagem de eleição!\n");
        }
    }
    else //processo de controle, processo 0
    {
        
    }

    MPI_Finalize();
    return 0;
}