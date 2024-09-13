import random
import socket
from elastic import send_metrics_to_elastic

engine_hosts = {
    "mpi": {
        "host": "jogodavida-mpi",  # Nome do serviço MPI no Kubernetes ou Docker
        "port": 30004,
    },
    "omp": {
        "host": "jogodavida-omp",  # Nome do serviço OMP no Kubernetes ou Docker
        "port": 30005,
    },
    "spark": {
        "host": "jogodavida-spark",  # Nome do serviço Spark no Kubernetes ou Docker
        "port": 30006,
    },
}

def submit_values_to_engines(powmin, powmax):
    chosen_engine = random.choice(["mpi"]) # random.choice(["omp", "mpi", "spark"])
    # chosen_engine = random.choice([
    #     "omp", 
    #     # todo consetar mpi
    #     # "mpi", 
    #     # todo adaptar spark para receber valores via tcp
    #     # "spark"
    # ])
    engine_host = engine_hosts[chosen_engine]["host"]
    engine_port = engine_hosts[chosen_engine]["port"]

    print(f"Send to {chosen_engine}, address {engine_host}:{engine_port}, powmin {powmin} and powmax {powmax}")

    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((engine_host, engine_port))

            message = f"{powmin} {powmax}".encode()
            s.sendall(message)
            
            all_responses = []

            while True:
                response = s.recv(1024)

                if not response:
                    break

                decoded_response = response.decode().strip()

                for line in decoded_response.split('\n'):
                    if len(line) > 1:
                        send_metrics_to_elastic(line, chosen_engine)
                        print(f"Received response from {chosen_engine}: {line}")
                        all_responses.append(f"{chosen_engine} {line}")

            return all_responses
    except Exception as e:
        print(f"Error connecting to {chosen_engine}: {e}")
        return f"Erro {e}"
