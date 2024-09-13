## Instruções para execução

I. Instalar gerenciador de pacotes (helm)

(dependerá do sistema operacional e/ou )

II. Adicionar o repositório do helm 

``helm repo add spark-operator https://kubeflow.github.io/spark-operator`` 

III. Atualizar repositório

``helm repo update``

IV. Instalação

``helm install spark-operator spark-operator/spark-operator --namespace spark-operator --create-namespace``

V. Adiciona serviço

``kubectl apply -f spark-service-account.yaml``

VI. Adiciona aplicação

``kubectl apply -f jogodavida-spark.yaml``