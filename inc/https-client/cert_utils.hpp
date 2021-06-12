#pragma once

#ifdef _WIN32
#include <openssl/ssl.h>
void import_openssl_certificates(const SSL_CTX* ctx);

#endif