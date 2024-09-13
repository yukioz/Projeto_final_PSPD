import socket
import threading
import websockets
import json
from services import submit_values_to_engines

def handle_tcp_client(client_socket):
    while True:
        try:
            data = client_socket.recv(1024).decode()

            if ',' in data:
                powmin, powmax = data.split(',')
            else:
                # client_socket.sendall(b"Erro: Formato de dados incorreto. Use 'POWMIN,POWMAX'.")
                break
     
            responses = submit_values_to_engines(powmin, powmax)

            for response in responses:
                client_socket.sendall(response.encode())
        except Exception as e:
            print("TCP Error", e)
            client_socket.sendall(b"Erro na requisicao, tentar novamente.")
            break

    client_socket.close()

def tcp_server():
    tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    tcp_socket.bind(('0.0.0.0', 30001))
    tcp_socket.listen(5)
    print("TCP Server Listening on port 30001")
    
    while True:
        client_socket, addr = tcp_socket.accept()
        print(f"TCP Connection from {addr}")
        threading.Thread(target=handle_tcp_client, args=(client_socket,)).start()

def udp_server():
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp_socket.bind(('0.0.0.0', 30002))
    print("UDP Server Listening on port 30002")
    
    while True:
        try:
            data, addr = udp_socket.recvfrom(1024)

            decoded_data = data.decode()

            if ',' in decoded_data:
                powmin, powmax = decoded_data.split(',')
            else:
                # udp_socket.sendto(b"Erro: Formato de dados incorreto. Use 'POWMIN,POWMAX'.", addr)
                break

            responses = submit_values_to_engines(powmin, powmax)

            for response in responses:
                udp_socket.sendto(response.encode(), addr)
        except Exception as e:
            print("UDP Error", e)
            udp_socket.sendto(b"Erro na requisicao, tentar novamente.", addr)

async def websocket_handler(websocket, path):
    async for message in websocket:
        try:
            data = json.loads(message)

            powmin = data['powmin']
            powmax = data['powmax']

            responses = submit_values_to_engines(powmin, powmax)

            for response in responses:
                await websocket.send(response)            
        except Exception as e:
            print("WebSocket Error", e)
            await websocket.send("Erro no recebimento da requisição, tentar novamente.")

async def websocket_server():
    server = await websockets.serve(websocket_handler, "0.0.0.0", 30003)
    print("WebSocket Server Listening on port 30003")
    await server.wait_closed()
