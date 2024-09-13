FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    mpich \
    && rm -rf /var/lib/apt/lists/*

COPY jogo_da_vida_mpi.c /app/jogo_da_vida_mpi.c

RUN mpicc /app/jogo_da_vida_mpi.c -o /app/jogo_da_vida_mpi

EXPOSE 30004

CMD ["mpirun", "-np", "1", "/app/jogo_da_vida_mpi"]
 