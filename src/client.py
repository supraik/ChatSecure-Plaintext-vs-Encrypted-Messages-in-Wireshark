#!/usr/bin/env python3
import socket
import base64
import os
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad, unpad

HOST = "127.0.0.1"
PORT = 8080

KEY = b"0123456789abcdef"
IV = b"abcdef9876543210"

def aes_decrypt(ciphertext_b64: str) -> str:
    cipher = AES.new(KEY, AES.MODE_CBC, IV)
    ciphertext = base64.b64decode(ciphertext_b64)
    plaintext = unpad(cipher.decrypt(ciphertext), AES.block_size)
    return plaintext.decode()

def send_message():
    secure = os.path.exists("/tmp/secmode")

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.connect((HOST, PORT))

        # Sending a POST request
        message = "Secret Message!"
        request = (
            f"POST / HTTP/1.1\r\n"
            f"Host: localhost\r\n"
            f"Content-Length: {len(message)}\r\n"
            f"\r\n"
            f"{message}"
        )

        sock.sendall(request.encode())
        data = sock.recv(4096).decode(errors="ignore")

        print("\n=== RAW SERVER RESPONSE ===")
        print(data)

        if secure:
            try:
                body = data.split("\r\n\r\n", 1)[-1]
                print("\n=== DECRYPTED RESPONSE ===")
                print(aes_decrypt(body))
            except Exception as e:
                print("\nError decrypting response:", e)

if __name__ == "__main__":
    send_message()
