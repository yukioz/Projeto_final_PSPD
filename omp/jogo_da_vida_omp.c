#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <omp.h> // Incluindo a biblioteca OpenMP

#define BUFFER_SIZE 1024
#define PORT 30005
#define ind2d(i, j) (i) * (tam + 2) + j

double wall_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec + tv.tv_usec / 1000000.0);
}

void UmaVida(int *tabulIn, int *tabulOut, int tam)
{
    int i, j, vizviv;

#pragma omp parallel for private(i, j, vizviv) // Paralelização com OpenMP
    for (i = 1; i <= tam; i++)
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
    int i;
    for (i = 0; i < (tam + 2) * (tam + 2); i++)
    {
        tabulIn[i] = 0;
        tabulOut[i] = 0;
    }
    // Padrão inicial do Jogo da Vida (Glider)
    tabulIn[ind2d(1, 2)] = 1;
    tabulIn[ind2d(2, 3)] = 1;
    tabulIn[ind2d(3, 1)] = 1;
    tabulIn[ind2d(3, 2)] = 1;
    tabulIn[ind2d(3, 3)] = 1;
}

int Correto(int *tabul, int tam)
{
    int cnt = 0, i;
    for (i = 0; i < (tam + 2) * (tam + 2); i++)
        cnt += tabul[i];
    return (cnt == 5 && tabul[ind2d(tam - 2, tam - 1)] && tabul[ind2d(tam - 1, tam)] &&
            tabul[ind2d(tam, tam - 2)] && tabul[ind2d(tam, tam - 1)] && tabul[ind2d(tam, tam)]);
}

int main()
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

    // Definindo as opções de socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Ligando o socket à porta 30005
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Falha ao fazer bind");
        exit(EXIT_FAILURE);
    }

    // Colocando o socket em modo de escuta
    if (listen(server_fd, 3) < 0)
    {
        perror("Falha ao escutar");
        exit(EXIT_FAILURE);
    }

    printf("Aguardando conexões na porta %d...\n", PORT);

    while (1)
    {
        // Aceitando uma conexão
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("Falha ao aceitar conexão");
            exit(EXIT_FAILURE);
        }

        // Limpando o buffer
        memset(buffer, 0, BUFFER_SIZE);

        // Recebendo valores powmin e powmax via TCP
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread > 0)
        {
            int powmin, powmax;
            sscanf(buffer, "%d %d", &powmin, &powmax);
            printf("Valores recebidos: powmin=%d, powmax=%d\n", powmin, powmax);

            for (int pow = powmin; pow <= powmax; pow++)
            {
                int tam = 1 << pow;
                double t0, t1, t2, t3;
                int *tabulIn, *tabulOut, i;

                t0 = wall_time();
                tabulIn = (int *)malloc((tam + 2) * (tam + 2) * sizeof(int));
                tabulOut = (int *)malloc((tam + 2) * (tam + 2) * sizeof(int));
                InitTabul(tabulIn, tabulOut, tam);
                t1 = wall_time();

                for (i = 0; i < 2 * (tam - 3); i++)
                {
                    UmaVida(tabulIn, tabulOut, tam);
                    UmaVida(tabulOut, tabulIn, tam);
                }

                t2 = wall_time();

                if (Correto(tabulIn, tam))
                    printf("**Ok, RESULTADO CORRETO para tamanho %d**\n", tam);
                else
                    printf("**Nok, RESULTADO ERRADO para tamanho %d**\n", tam);

                t3 = wall_time();
                char result[BUFFER_SIZE];
                snprintf(result, sizeof(result), "tam=%d,init=%7.7f,comp=%7.7f,fim=%7.7f,tot=%7.7f\n",
                         tam, t1 - t0, t2 - t1, t3 - t2, t3 - t0);

                // Enviando o resultado de volta ao cliente
                if (write(new_socket, result, strlen(result)) < 0)
                {
                    perror("Falha ao enviar dados");
                    break; // Em caso de erro de envio, encerre a conexão
                }

                free(tabulIn);
                free(tabulOut);
            }

            // Fechar o socket do cliente após enviar todos os resultados
            printf("Fechando conexão com o cliente.\n");
            close(new_socket);
        }
        else if (valread == 0)
        {
            // Se a leitura retornar 0, o cliente encerrou a conexão
            printf("Conexão encerrada pelo cliente.\n");
            close(new_socket);
        }
        else
        {
            // Em caso de erro de leitura
            perror("Erro na leitura dos dados");
            close(new_socket);
        }
    }

    // Fechando o socket do servidor
    close(server_fd);

    return 0;
}
