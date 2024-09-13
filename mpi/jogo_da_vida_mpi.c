#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define ind2d(i, j) ((i) * (tam + 2) + (j))
#define MASTER 0

#define PORT 30004
#define BUFFER_SIZE 1024

double wall_time(void)
{
  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);
  return (tv.tv_sec + tv.tv_usec / 1000000.0);
}

void UmaVida(int *tabulIn, int *tabulOut, int tam, int start, int end)
{
  int i, j, vizviv;

  for (i = start; i <= end; i++)
  {
    for (j = 1; j <= tam; j++)
    {
      vizviv = tabulIn[ind2d(i - 1, j - 1)] + tabulIn[ind2d(i - 1, j)] +
               tabulIn[ind2d(i - 1, j + 1)] + tabulIn[ind2d(i, j - 1)] +
               tabulIn[ind2d(i, j + 1)] + tabulIn[ind2d(i + 1, j - 1)] +
               tabulIn[ind2d(i + 1, j)] + tabulIn[ind2d(i + 1, j + 1)];
      if (tabulIn[ind2d(i, j)] && vizviv < 2)
        tabulOut[ind2d(i, j)] = 0;
      else if (tabulIn[ind2d(i, j)] && vizviv > 3)
        tabulOut[ind2d(i, j)] = 0;
      else if (!tabulIn[ind2d(i, j)] && vizviv == 3)
        tabulOut[ind2d(i, j)] = 1;
      else
        tabulOut[ind2d(i, j)] = tabulIn[ind2d(i, j)];
    }
  }
}

void InitTabul(int *tabulIn, int *tabulOut, int tam)
{
  int ij;

  for (ij = 0; ij < (tam + 2) * (tam + 2); ij++)
  {
    tabulIn[ij] = 0;
    tabulOut[ij] = 0;
  }

  tabulIn[ind2d(1, 2)] = 1;
  tabulIn[ind2d(2, 3)] = 1;
  tabulIn[ind2d(3, 1)] = 1;
  tabulIn[ind2d(3, 2)] = 1;
  tabulIn[ind2d(3, 3)] = 1;
}

int Correto(int *tabul, int tam)
{
  int ij, cnt;

  cnt = 0;
  for (ij = 0; ij < (tam + 2) * (tam + 2); ij++)
    cnt = cnt + tabul[ij];
  return (cnt == 5 && tabul[ind2d(tam - 2, tam - 1)] &&
          tabul[ind2d(tam - 1, tam)] && tabul[ind2d(tam, tam - 2)] &&
          tabul[ind2d(tam, tam - 1)] && tabul[ind2d(tam, tam)]);
}

// Função para receber powmin e powmax via TCP
void receive_values_from_socket(int *powmin, int *powmax)
{
  int server_fd, new_socket;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  char buffer[BUFFER_SIZE] = {0};

  // Criando o socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
    perror("Falha ao criar socket");
    exit(EXIT_FAILURE);
  }

  // Configurando endereço e porta do socket
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // Bind do socket
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    perror("Falha ao fazer bind");
    exit(EXIT_FAILURE);
  }

  // Escutando conexões
  if (listen(server_fd, 3) < 0)
  {
    perror("Falha ao escutar");
    exit(EXIT_FAILURE);
  }

  printf("Aguardando conexão na porta %d...\n", PORT);

  // Aceitando conexão
  if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
  {
    perror("Falha ao aceitar conexão");
    exit(EXIT_FAILURE);
  }

  // Recebendo valores powmin e powmax
  int valread = read(new_socket, buffer, BUFFER_SIZE);
  if (valread > 0)
  {
    sscanf(buffer, "%d %d", powmin, powmax);
    printf("Valores recebidos: powmin = %d, powmax = %d\n", *powmin, *powmax);
  }
  else
  {
    printf("Erro ao ler valores do socket.\n");
  }

  // Fechando socket
  close(new_socket);
  close(server_fd);
}

int main(int argc, char *argv[])
{
  int powmin, powmax;
  int i, tam, *tabulIn, *tabulOut;
  double t0, t1, t2, t3;

  int rank, size;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Receber valores powmin e powmax via socket no rank MASTER
  if (rank == MASTER)
  {
    receive_values_from_socket(&powmin, &powmax);
  }

  // Broadcast dos valores powmin e powmax para todos os processos
  MPI_Bcast(&powmin, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
  MPI_Bcast(&powmax, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

  // Iniciando o jogo da vida para valores de powmin a powmax
  for (int pow = powmin; pow <= powmax; pow++)
  {
    tam = 1 << pow;

    t0 = wall_time();
    tabulIn = (int *)malloc((tam + 2) * (tam + 2) * sizeof(int));
    tabulOut = (int *)malloc((tam + 2) * (tam + 2) * sizeof(int));
    InitTabul(tabulIn, tabulOut, tam);
    t1 = wall_time();

    int rows_per_proc = tam / size;
    int start = rank * rows_per_proc + 1;
    int end = (rank == size - 1) ? tam : start + rows_per_proc - 1;

    for (i = 0; i < 2 * (tam - 3); i++)
    {
      UmaVida(tabulIn, tabulOut, tam, start, end);
      MPI_Allgather(&tabulOut[ind2d(start, 0)], rows_per_proc * (tam + 2), MPI_INT,
                    &tabulIn[ind2d(1, 0)], rows_per_proc * (tam + 2), MPI_INT,
                    MPI_COMM_WORLD);
      UmaVida(tabulIn, tabulOut, tam, start, end);
      MPI_Allgather(&tabulOut[ind2d(start, 0)], rows_per_proc * (tam + 2), MPI_INT,
                    &tabulIn[ind2d(1, 0)], rows_per_proc * (tam + 2), MPI_INT,
                    MPI_COMM_WORLD);
    }

    t2 = wall_time();

    if (rank == MASTER)
    {
      if (Correto(tabulIn, tam))
        printf("**RESULTADO CORRETO**\n");
      else
        printf("**RESULTADO ERRADO**\n");

      t3 = wall_time();
      printf("tam=%d,init=%7.7f,comp=%7.7f,fim=%7.7f,tot=%7.7f \n",
             tam, t1 - t0, t2 - t1, t3 - t2, t3 - t0);
    }

    free(tabulIn);
    free(tabulOut);
  }

  MPI_Finalize();
  return 0;
}
