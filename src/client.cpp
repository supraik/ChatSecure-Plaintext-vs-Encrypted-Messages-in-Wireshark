// client.cpp
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

using namespace std;

const unsigned char PRE_SHARED_KEY[16] = "0123456789abcdef"; // demo only
const unsigned char PRE_SHARED_IV[16]  = "abcdef9876543210"; // demo only

string base64_encode(const unsigned char* input, int length) {
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO *bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, input, length);
    BIO_flush(b64);

    BUF_MEM *bptr;
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
    out.resize(input.size());
    int decoded_len = BIO_read(bmem, &out[0], input.size());
    if (decoded_len < 0) decoded_len = 0;
    out.resize(decoded_len);
    BIO_free_all(bmem);
    return out;
}

bool aes_encrypt(const string &plain, string &out_b64) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, PRE_SHARED_KEY, PRE_SHARED_IV)) {
        EVP_CIPHER_CTX_free(ctx); return false;
    }
    vector<unsigned char> ciphertext(plain.size() + EVP_CIPHER_block_size(EVP_aes_128_cbc()));
    int len=0, ciphertext_len=0;
    if (1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len, (const unsigned char*)plain.data(), plain.size())) {
        EVP_CIPHER_CTX_free(ctx); return false;
    }
    ciphertext_len = len;
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext.data()+len, &len)) {
        EVP_CIPHER_CTX_free(ctx); return false;
    }
    ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    out_b64 = base64_encode(ciphertext.data(), ciphertext_len);
    return true;
}

bool aes_decrypt(const string &cipher_b64, string &out_plain) {
    string cipher = base64_decode(cipher_b64);
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, PRE_SHARED_KEY, PRE_SHARED_IV)) {
        EVP_CIPHER_CTX_free(ctx); return false;
    }
    vector<unsigned char> plaintext(cipher.size() + EVP_CIPHER_block_size(EVP_aes_128_cbc()));
    int len=0, plaintext_len=0;
    if (1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len, (const unsigned char*)cipher.data(), cipher.size())) {
        EVP_CIPHER_CTX_free(ctx); return false;
    }
    plaintext_len = len;
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext.data()+len, &len)) {
        EVP_CIPHER_CTX_free(ctx); return false;
    }
    plaintext_len += len;
    EVP_CIPHER_CTX_free(ctx);
    out_plain.assign((char*)plaintext.data(), plaintext_len);
    return true;
}

int main() {
    const int PORT = 8080;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { cerr << "socket failed\n"; return 1; }

    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr) <= 0) { cerr << "inet_pton failed\n"; return 1; }

    if (connect(sock, (sockaddr*)&serv, sizeof(serv)) < 0) { cerr << "connect failed\n"; return 1; }

    // toggle secure by creating /tmp/secmode (exists -> secure)
    bool secure = (access("/tmp/secmode", F_OK) == 0);

    string body = "Secret Message!"; // plaintext to send
    string send_body = body;
    string request_headers;

    if (secure) {
        string enc;
        if (!aes_encrypt(body, enc)) {
            cerr << "encryption failed\n";
            close(sock);
            return 1;
        }
        send_body = enc;
        request_headers = "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: " + to_string(send_body.size()) + "\r\nX-Encrypted: 1\r\n\r\n";
    } else {
        request_headers = "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: " + to_string(send_body.size()) + "\r\n\r\n";
    }

    string full = request_headers + send_body;
    send(sock, full.c_str(), full.size(), 0);

    // recv response (simple single recv)
    char buf[8192];
    memset(buf, 0, sizeof(buf));
    ssize_t r = recv(sock, buf, sizeof(buf)-1, 0);
    if (r <= 0) { cerr << "no response\n"; close(sock); return 1; }
    string resp(buf, r);
    cout << "Raw response captured by client:\n" << resp << "\n\n";

    // If server responded encrypted, parse header and decrypt
    if (resp.find("X-Encrypted: 1") != string::npos) {
        size_t bstart = resp.find("\r\n\r\n");
        string resp_body = (bstart == string::npos) ? "" : resp.substr(bstart+4);
        string dec;
        if (aes_decrypt(resp_body, dec)) {
            cout << "Decrypted response body:\n" << dec << "\n";
        } else {
            cout << "Failed to decrypt server response\n";
        }
    }

    close(sock);
    return 0;
}
