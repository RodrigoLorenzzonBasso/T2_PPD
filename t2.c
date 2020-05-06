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
    MPI_Status status;
   
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);

    int buffer;
    int array_ativos[numprocs];

    int coordenador = numprocs-1;

    if(myrank != 0) // processos que participam da eleicao 1 até numprocs-1
    {
        while(1)
        {
            MPI_Recv(&array_ativos[0], numprocs, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if(status.MPI_TAG == TAG_CONTROLE)
            {
                printf("Sou o processo %d : ",myrank);
                printf("Recebi mensagem do controle!\n");
                if(array_ativos[0] != 0)
                    printf("Processo que caiu: %d : ",array_ativos[0]);
                printf("Criando nova eleição\n");

                // no caso de um processo que caiu voltar disparar eleicao com todos
                int caiu;
                if(array_ativos[0] != 0)
                    caiu = 1;
                else
                    caiu = 0;

                // Construir a mensagem de eleicao
                for(int i=1; i<numprocs - caiu; i++)
                {
                    array_ativos[i] = 0;
                }
                array_ativos[1] = myrank;

                // enviar para o próximo
                if(myrank == numprocs-1) // sou o ultimo, envia para o processo 1 (0 é o de controle)
                {
                    if(array_ativos[0] == 1)
                        MPI_Send(&array_ativos[0], numprocs, MPI_INT, 2, TAG_ELEICAO, MPI_COMM_WORLD);
                    else
                        MPI_Send(&array_ativos[0], numprocs, MPI_INT, 1, TAG_ELEICAO, MPI_COMM_WORLD);
                }
                else // se nao, envia para o próximo
                {
                    if(array_ativos[0] == myrank+1 && myrank != numprocs-2)
                        MPI_Send(&array_ativos[0], numprocs, MPI_INT, myrank+2, TAG_ELEICAO, MPI_COMM_WORLD);
                    else if(array_ativos[0] == myrank+1 && myrank == numprocs-2)
                        MPI_Send(&array_ativos[0], numprocs, MPI_INT, 1, TAG_ELEICAO, MPI_COMM_WORLD);
                    else
                        MPI_Send(&array_ativos[0], numprocs, MPI_INT, myrank+1, TAG_ELEICAO, MPI_COMM_WORLD);
                }

                // esperar pra receber o array completo
                MPI_Recv(&array_ativos[0], numprocs, MPI_INT, MPI_ANY_SOURCE, TAG_ELEICAO, MPI_COMM_WORLD, &status);

                printf("Sou o processo %d : Recebi o array dos ativos: ",myrank);

                // buscar o maior id entre os ativos
                int maior = -1;
                for(int i=1; i<numprocs - caiu; i++)
                {
                    printf("%d ", array_ativos[i]);
                    if(array_ativos[i] > maior)
                    {
                        maior = array_ativos[i];
                    }
                }
                printf("\n");

                // eleger coordenador e montar mensagem para informar os outros
                coordenador = maior;
                printf("Elegi %d a novo coordenador\n", coordenador);
                if(myrank == numprocs -1)
                {
                    if(array_ativos[0] == 1)
                        MPI_Send(&coordenador, 1, MPI_INT, 2, TAG_ELEICAO, MPI_COMM_WORLD);
                    else
                        MPI_Send(&coordenador, 1, MPI_INT, 1, TAG_ELEICAO, MPI_COMM_WORLD);
                }
                else
                {
                    if(array_ativos[0] == myrank+1 && myrank != numprocs-2)
                        MPI_Send(&coordenador, 1, MPI_INT, myrank+2, TAG_ELEICAO, MPI_COMM_WORLD);
                    else if(array_ativos[0] == myrank+1 && myrank == numprocs-2)
                        MPI_Send(&coordenador, 1, MPI_INT, 1, TAG_ELEICAO, MPI_COMM_WORLD);
                    else
                        MPI_Send(&coordenador, 1, MPI_INT, myrank+1, TAG_ELEICAO, MPI_COMM_WORLD);
                }

                // esperar receber a mensagem de volta e enviar a confirmação para o controle
                MPI_Recv(&buffer, 1, MPI_INT, MPI_ANY_SOURCE, TAG_ELEICAO, MPI_COMM_WORLD, &status);
                MPI_Send(&buffer, 1, MPI_INT, 0, TAG_CONTROLE, MPI_COMM_WORLD);
            }
            else if(status.MPI_TAG == TAG_ELEICAO)
            {
                printf("Sou o processo %d : ",myrank);
                printf("Recebi mensagem de eleição!\n");

                int caiu;
                if(array_ativos[0] != 0)
                    caiu = 1;
                else
                    caiu = 0;

                // coloca o rank atual no array de ativos
                for(int i=1; i<numprocs - caiu; i++)
                {
                    if(array_ativos[i] == 0)
                    {
                        array_ativos[i] = myrank;
                        break;
                    }
                }

                // envia para o próximo
                if(myrank == numprocs-1)
                {
                    if(array_ativos[0] == 1)
                        MPI_Send(&array_ativos[0], numprocs, MPI_INT, 2, TAG_ELEICAO, MPI_COMM_WORLD);
                    else
                        MPI_Send(&array_ativos[0], numprocs, MPI_INT, 1, TAG_ELEICAO, MPI_COMM_WORLD);
                }
                else
                {
                    if(array_ativos[0] == myrank+1 && myrank != numprocs-2)
                        MPI_Send(&array_ativos[0], numprocs, MPI_INT, myrank+2, TAG_ELEICAO, MPI_COMM_WORLD);
                    else if(array_ativos[0] == myrank+1 && myrank == numprocs-2)
                        MPI_Send(&array_ativos[0], numprocs, MPI_INT, 1, TAG_ELEICAO, MPI_COMM_WORLD);
                    else
                        MPI_Send(&array_ativos[0], numprocs, MPI_INT, myrank+1, TAG_ELEICAO, MPI_COMM_WORLD);
                }

                // espera receber qual o novo coordenador
                MPI_Recv(&coordenador, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                printf("Meu rank %d : Recebi que o novo coordenador é %d\n", myrank, coordenador);

                // envia para o próximo
                if(myrank == numprocs-1)
                {
                    if(array_ativos[0] == 1)
                        MPI_Send(&coordenador, 1, MPI_INT, 2, TAG_ELEICAO, MPI_COMM_WORLD);
                    else
                        MPI_Send(&coordenador, 1, MPI_INT, 1, TAG_ELEICAO, MPI_COMM_WORLD);
                }
                else
                {
                    if(array_ativos[0] == myrank+1 && myrank != numprocs-2)
                        MPI_Send(&coordenador, 1, MPI_INT, myrank+2, TAG_ELEICAO, MPI_COMM_WORLD);
                    else if(array_ativos[0] == myrank+1 && myrank == numprocs-2)
                        MPI_Send(&coordenador, 1, MPI_INT, 1, TAG_ELEICAO, MPI_COMM_WORLD);
                    else
                        MPI_Send(&coordenador, 1, MPI_INT, myrank+1, TAG_ELEICAO, MPI_COMM_WORLD);
                }
            }
        }
    }
    else //processo de controle, processo 0
    {
        // criar a mensagem de que o coordenador caiu
        buffer = coordenador;

        // enviar para um processo qualquer
        int p = 1;
        printf("Processo de controle enviando para %d que o processo %d caiu\n", p, buffer);
        MPI_Send(&buffer, 1, MPI_INT, p, TAG_CONTROLE, MPI_COMM_WORLD);

        MPI_Recv(&buffer, 1, MPI_INT, p, TAG_CONTROLE, MPI_COMM_WORLD, &status);
        printf("Eleicao acabou e o novo coordenador é %d\n", buffer);

        printf("Processo de controle enviando para %d para disparar nova eleição com todos os processos ativos\n",p);
        buffer = 0;
        MPI_Send(&buffer, 1, MPI_INT, p, TAG_CONTROLE, MPI_COMM_WORLD);

        MPI_Recv(&buffer, 1, MPI_INT, p, TAG_CONTROLE, MPI_COMM_WORLD, &status);
        printf("Eleicao acabou e o novo coordenador é %d\n", buffer);
    }

    MPI_Finalize();
    return 0;
}