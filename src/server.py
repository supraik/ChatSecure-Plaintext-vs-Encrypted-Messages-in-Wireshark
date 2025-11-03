#!/usr/bin/env python3
import socket
import base64
import os
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad, unpad

HOST = "0.0.0.0"
PORT = 8080

# Predefined 16-byte key & IV (for AES-128-CBC)
KEY = b"0123456789abcdef"
IV = b"abcdef9876543210"

def aes_encrypt(plaintext: bytes) -> str:
    cipher = AES.new(KEY, AES.MODE_CBC, IV)
    ciphertext = cipher.encrypt(pad(plaintext, AES.block_size))
    return base64.b64encode(ciphertext).decode()

def aes_decrypt(ciphertext_b64: str) -> bytes:
    cipher = AES.new(KEY, AES.MODE_CBC, IV)
    ciphertext = base64.b64decode(ciphertext_b64)
    return unpad(cipher.decrypt(ciphertext), AES.block_size)

def handle_request(request: str, secure: bool) -> bytes:
    if request.startswith("GET /"):
        body = "<html><body><h1>Simple Python HTTP Server!</h1></body></html>"
        resp = f"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n{body}"
    elif request.startswith("POST /"):
        body = request.split("\r\n\r\n", 1)[-1]
        resp = f"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nReceived POST: {body}"
    else:
        resp = "HTTP/1.1 404 Not Found\r\n\r\n"

    if secure:
        encrypted = aes_encrypt(resp.encode())
        resp = f"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n{encrypted}"

    return resp.encode()

def start_server():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST, PORT))
        s.listen(5)
        print(f"Server listening on {HOST}:{PORT}")

        while True:
            conn, addr = s.accept()
            with conn:
                print(f"Connection from {addr}")
                data = conn.recv(4096)
                if not data:
                    continue

                secure = os.path.exists("/tmp/secmode")
                req = data.decode(errors="ignore")
                response = handle_request(req, secure)
                conn.sendall(response)
                print(f"Secure Mode: {secure}")

if __name__ == "__main__":
    start_server()
