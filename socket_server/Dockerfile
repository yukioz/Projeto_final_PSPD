FROM python:3.9-slim

WORKDIR /app

COPY . /app

RUN pip install --no-cache-dir -r requirements.txt

EXPOSE 30001
EXPOSE 30002/udp
EXPOSE 30003

CMD ["python", "-u", "main.py"]
