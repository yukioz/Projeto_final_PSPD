FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    libomp-dev \
    && rm -rf /var/lib/apt/lists/*

COPY jogo_da_vida_omp.c /app/jogo_da_vida_omp.c

RUN gcc -fopenmp /app/jogo_da_vida_omp.c -o /app/jogo_da_vida_omp

EXPOSE 30005

CMD ["/app/jogo_da_vida_omp"]
