#include <https-client/cert_utils.hpp>

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <stdexcept>
#include <iostream>


void import_openssl_certificates(const SSL_CTX *ssl_ctx) {
    HCERTSTORE win_store = CertOpenSystemStore(NULL, "ROOT");
    if (!win_store) {
        throw std::runtime_error("failed to open certificate store");
    }
    X509_STORE *x509_store = SSL_CTX_get_cert_store(ssl_ctx);
    PCCERT_CONTEXT win_cert_ctx = NULL;
    while (win_cert_ctx = CertEnumCertificatesInStore(win_store, win_cert_ctx)) {
        const unsigned char* encoded = win_cert_ctx->pbCertEncoded;
        X509* cert = d2i_X509(NULL, &encoded, win_cert_ctx->cbCertEncoded);
        if (cert) {
            X509_STORE_add_cert(x509_store, cert);
            X509_free(cert);
        } else {
            std::cerr << "failed to decode certificate\n";
        }
    }
    CertCloseStore(win_store, 0);
}

#endif