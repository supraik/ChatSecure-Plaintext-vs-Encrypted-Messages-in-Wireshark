// server.cpp
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

using namespace std;

const unsigned char PRE_SHARED_KEY[16] = "0123456789abcdef"; // demo only
const unsigned char PRE_SHARED_IV[16]  = "abcdef9876543210"; // demo only

// Base64 encode using OpenSSL BIO
string base64_encode(const unsigned char* input, int length) {
    BIO *bmem = nullptr, *b64 = nullptr;
    BUF_MEM *bptr = nullptr;

    b64 = BIO_new(BIO_f_base64());
    // Do not use newlines
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, input, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    string out(bptr->data, bptr->length);
    BIO_free_all(b64);
    return out;
}

string base64_decode(const string &input) {
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO *bmem = BIO_new_mem_buf(input.c_str(), input.size());
    bmem = BIO_push(b64, bmem);

    string out;
    out.resize(input.size()); // safe upper bound
    int decoded_len = BIO_read(bmem, &out[0], input.size());
    if (decoded_len < 0) decoded_len = 0;
    out.resize(decoded_len);
    BIO_free_all(bmem);
    return out;
}

bool aes_decrypt(const string &cipher_b64, string &out_plain) {
    string cipher = base64_decode(cipher_b64);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, PRE_SHARED_KEY, PRE_SHARED_IV)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    vector<unsigned char> plaintext(cipher.size() + EVP_CIPHER_block_size(EVP_aes_128_cbc()));
    int len = 0, plaintext_len = 0;
    if (1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, (const unsigned char*)cipher.data(), cipher.size())) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    plaintext_len = len;
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);

    out_plain.assign((char*)plaintext.data(), plaintext_len);
    return true;
}

bool aes_encrypt(const string &plain, string &out_b64) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, PRE_SHARED_KEY, PRE_SHARED_IV)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    vector<unsigned char> ciphertext(plain.size() + EVP_CIPHER_block_size(EVP_aes_128_cbc()));
    int len = 0, ciphertext_len = 0;
    if (1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, (const unsigned char*)plain.data(), plain.size())) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    ciphertext_len = len;
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);

    out_b64 = base64_encode(ciphertext.data(), ciphertext_len);
    return true;
}

string get_header_value(const string &req, const string &header) {
    size_t pos = req.find(header + ":");
    if (pos == string::npos) return "";
    size_t eol = req.find("\r\n", pos);
    if (eol == string::npos) return "";
    size_t start = pos + header.size() + 1;
    // skip spaces
    while (start < eol && (req[start] == ' ' || req[start] == '\t')) start++;
    return req.substr(start, eol - start);
}

string handle_request(const string &req, int client_sock) {
    bool encrypted_header = (get_header_value(req, "X-Encrypted") == "1");
    // Extract body
    size_t body_start = req.find("\r\n\r\n");
    string body = (body_start == string::npos) ? "" : req.substr(body_start + 4);

    string plain_body = body;
    if (encrypted_header) {
        string decrypted;
        if (!aes_decrypt(body, decrypted)) {
            string resp = "HTTP/1.1 400 Bad Request\r\n\r\nFailed to decrypt\n";
            send(client_sock, resp.c_str(), resp.size(), 0);
            return resp;
        }
        plain_body = decrypted;
    }

    cout << "Server received (interpreted):\n" << plain_body << endl;

    // Build response
    string response_body = "Server received: " + plain_body;
    string resp;
    if (encrypted_header) {
        string enc;
        aes_encrypt(response_body, enc);
        resp = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nX-Encrypted: 1\r\nContent-Length: " + to_string(enc.size()) + "\r\n\r\n" + enc;
    } else {
        resp = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + to_string(response_body.size()) + "\r\n\r\n" + response_body;
    }

    send(client_sock, resp.c_str(), resp.size(), 0);
    return resp;
}

int main() {
    const int PORT = 8080;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { cerr << "socket failed\n"; return 1; }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) { cerr << "bind failed\n"; return 1; }
    if (listen(sock, 5) < 0) { cerr << "listen failed\n"; return 1; }

    cout << "Server listening on 0.0.0.0:" << PORT << " (ctrl-c to stop)\n";

    while (true) {
        int client_sock = accept(sock, nullptr, nullptr);
        if (client_sock < 0) { cerr << "accept failed\n"; continue; }

        char buf[8192];
        memset(buf, 0, sizeof(buf));
        ssize_t r = recv(client_sock, buf, sizeof(buf)-1, 0);
        if (r <= 0) { close(client_sock); continue; }

        string req(buf, r);
        handle_request(req, client_sock);
        close(client_sock);
    }

    close(sock);
    return 0;
}
