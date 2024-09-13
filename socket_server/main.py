import threading
from server import tcp_server, udp_server, websocket_server
import asyncio

if __name__ == "__main__":
    threading.Thread(target=tcp_server).start()
    threading.Thread(target=udp_server).start()

    asyncio.get_event_loop().run_until_complete(websocket_server())
    asyncio.get_event_loop().run_forever()