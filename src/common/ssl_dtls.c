/*
 * ssl_dtls.c - DTLS handshake helpers for encrypted UDP (nue/naue),
 * and SSL I/O wrappers that keep SIGPIPE from killing the process.
 */

#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200809L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "../h/gsupport.h"

#if HAVE_LIBSSL
#if UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif                                  /* UNIX */
#ifdef NT
#include <winsock2.h>
#include <ws2tcpip.h>
#endif                                  /* NT */
#endif                                  /* HAVE_LIBSSL */

#if HAVE_LIBSSL

/*
 * A write on a socket whose peer has already closed raises SIGPIPE. The
 * default disposition kills the process, so the SSL call never gets to
 * return the error. Block SIGPIPE in this thread for the call, drop a
 * signal the call itself raised, and put the previous mask back
 * unchanged. errno from the SSL call is kept: the cleanup calls would
 * otherwise replace it (sigtimedwait sets EAGAIN when nothing is left).
 *
 * A standard signal is one bit. One that was already pending is left
 * alone. A new one is taken without waiting. sigwait would block the
 * thread that loses a race for a process-directed SIGPIPE.
 */
#if UNIX
#if defined(__APPLE__) && defined(Concurrent)
static pthread_mutex_t ossl_sigpipe_mu = PTHREAD_MUTEX_INITIALIZER;
#endif

/* macOS and BSD: this socket then fails with EPIPE and raises nothing. */
static void ossl_nosigpipe(SSL *ssl)
{
#ifdef SO_NOSIGPIPE
   int fd, on = 1;

   if (ssl == NULL)
      return;
   fd = SSL_get_fd(ssl);
   if (fd >= 0)
      (void)setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof on);
#else                                   /* SO_NOSIGPIPE */
   (void)ssl;
#endif                                  /* SO_NOSIGPIPE */
}

static int ossl_sigpipe_block(sigset_t *save, int *was_pending)
{
   sigset_t block, pend;
   int rc;

   *was_pending = 0;
   if (sigpending(&pend) == 0 && sigismember(&pend, SIGPIPE))
      *was_pending = 1;

   sigemptyset(&block);
   sigaddset(&block, SIGPIPE);
#ifdef Concurrent
   rc = pthread_sigmask(SIG_BLOCK, &block, save);
#else                                   /* Concurrent */
   rc = sigprocmask(SIG_BLOCK, &block, save);
#endif                                  /* Concurrent */
   return rc == 0;
}

/* Drop a SIGPIPE this call raised. The caller's mask is not modified. */
static void ossl_sigpipe_discard(int was_pending)
{
   sigset_t pend;

   if (was_pending)
      return;
   if (sigpending(&pend) < 0 || !sigismember(&pend, SIGPIPE))
      return;

#if !defined(__APPLE__)
   {
   sigset_t one;
   struct timespec zero;

   sigemptyset(&one);
   sigaddset(&one, SIGPIPE);
   memset(&zero, 0, sizeof zero);
   while (sigtimedwait(&one, NULL, &zero) > 0)
      ;
   }
#else                                   /* __APPLE__ */
   {
   struct sigaction ign, old;

   /*
    * No sigtimedwait here. Setting SIG_IGN discards a pending SIGPIPE
    * even while it is blocked. The previous action is restored before
    * the caller's mask is.
    */
#if defined(Concurrent)
   pthread_mutex_lock(&ossl_sigpipe_mu);
#endif                                  /* Concurrent */
   memset(&ign, 0, sizeof ign);
   ign.sa_handler = SIG_IGN;
   sigemptyset(&ign.sa_mask);
   if (sigaction(SIGPIPE, &ign, &old) == 0)
      sigaction(SIGPIPE, &old, NULL);
#if defined(Concurrent)
   pthread_mutex_unlock(&ossl_sigpipe_mu);
#endif                                  /* Concurrent */
   }
#endif                                  /* __APPLE__ */
}

static void ossl_sigpipe_unblock(int blocked, const sigset_t *save,
                                 int was_pending)
{
   int saved = errno;

   if (blocked) {
      ossl_sigpipe_discard(was_pending);
#ifdef Concurrent
      pthread_sigmask(SIG_SETMASK, save, NULL);
#else                                   /* Concurrent */
      sigprocmask(SIG_SETMASK, save, NULL);
#endif                                  /* Concurrent */
      }
   errno = saved;
}

#define OSSL_CALL(ssl, call) \
   do { \
      sigset_t ossl_save; \
      int ossl_was = 0; \
      int ossl_blocked; \
      ossl_nosigpipe(ssl); \
      ossl_blocked = ossl_sigpipe_block(&ossl_save, &ossl_was); \
      call; \
      ossl_sigpipe_unblock(ossl_blocked, &ossl_save, ossl_was); \
   } while (0)
#else                                   /* UNIX */
#define OSSL_CALL(ssl, call) do { (void)(ssl); call; } while (0)
#endif                                  /* UNIX */

int ossl_connect(SSL *ssl)
{
   int rc;
   OSSL_CALL(ssl, rc = SSL_connect(ssl));
   return rc;
}

int ossl_accept(SSL *ssl)
{
   int rc;
   OSSL_CALL(ssl, rc = SSL_accept(ssl));
   return rc;
}

int ossl_shutdown(SSL *ssl)
{
   int rc;
   OSSL_CALL(ssl, rc = SSL_shutdown(ssl));
   return rc;
}

int ossl_read(SSL *ssl, void *buf, int num)
{
   int rc;
   OSSL_CALL(ssl, rc = SSL_read(ssl, buf, num));
   return rc;
}

int ossl_write(SSL *ssl, const void *buf, int num)
{
   int rc;
   OSSL_CALL(ssl, rc = SSL_write(ssl, buf, num));
   return rc;
}

#ifndef OPENSSL_NO_DTLS

#define DTLS_COOKIE_SECRET_LEN 16

static unsigned char dtls_cookie_secret[DTLS_COOKIE_SECRET_LEN];
static int dtls_cookie_ready = 0;

extern int sock_udp_connect_saved(int fd);

static int dtls_generate_cookie(SSL *ssl, unsigned char *cookie,
                                unsigned int *cookie_len)
{
   (void)ssl;
   if (!dtls_cookie_ready) {
      if (RAND_bytes(dtls_cookie_secret, DTLS_COOKIE_SECRET_LEN) != 1)
         return 0;
      dtls_cookie_ready = 1;
      }
   memcpy(cookie, dtls_cookie_secret, DTLS_COOKIE_SECRET_LEN);
   *cookie_len = DTLS_COOKIE_SECRET_LEN;
   return 1;
}

static int dtls_verify_cookie(SSL *ssl, const unsigned char *cookie,
                              unsigned int cookie_len)
{
   (void)ssl;
   if (!dtls_cookie_ready || cookie_len != DTLS_COOKIE_SECRET_LEN)
      return 0;
   return CRYPTO_memcmp(cookie, dtls_cookie_secret, DTLS_COOKIE_SECRET_LEN) == 0;
}

static int dtls_want_retry(SSL *ssl, int rv)
{
   int err = SSL_get_error(ssl, rv);
   return err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE;
}

static int dtls_connect_peer(int fd, BIO_ADDR *peer, void *sa_out)
{
   int family = BIO_ADDR_family(peer);
   unsigned short port = BIO_ADDR_rawport(peer);
   size_t alen;

   if (family == AF_INET) {
      struct sockaddr_in *sin = (struct sockaddr_in *)sa_out;
      memset(sin, 0, sizeof(*sin));
      sin->sin_family = AF_INET;
      sin->sin_port = port;
      alen = sizeof(sin->sin_addr);
      if (!BIO_ADDR_rawaddress(peer, &sin->sin_addr, &alen))
         return -1;
      return connect(fd, (struct sockaddr *)sin, sizeof(*sin));
      }
#ifdef AF_INET6
   if (family == AF_INET6) {
      struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sa_out;
      memset(sin6, 0, sizeof(*sin6));
      sin6->sin6_family = AF_INET6;
      sin6->sin6_port = port;
      alen = sizeof(sin6->sin6_addr);
      if (!BIO_ADDR_rawaddress(peer, &sin6->sin6_addr, &alen))
         return -1;
      return connect(fd, (struct sockaddr *)sin6, sizeof(*sin6));
      }
#endif
   return -1;
}

int ssl_dtls_accept(SSL *ssl, int fd)
{
   BIO *bio;
   BIO_ADDR *client;
   SSL_CTX *ctx;
   struct timeval tv;
   struct sockaddr_storage peer_sa;
   int rv, tries;

   bio = BIO_new_dgram(fd, BIO_NOCLOSE);
   if (bio == NULL)
      return 0;
   /* Short recv timeout so we can re-enter DTLSv1_listen while the client starts. */
   tv.tv_sec = 1;
   tv.tv_usec = 0;
   BIO_ctrl(bio, BIO_CTRL_DGRAM_SET_RECV_TIMEOUT, 0, &tv);
   BIO_ctrl(bio, BIO_CTRL_DGRAM_SET_SEND_TIMEOUT, 0, &tv);
   SSL_set_bio(ssl, bio, bio);

   ctx = SSL_get_SSL_CTX(ssl);
   SSL_CTX_set_cookie_generate_cb(ctx, dtls_generate_cookie);
   SSL_CTX_set_cookie_verify_cb(ctx, dtls_verify_cookie);
   SSL_set_options(ssl, SSL_OP_COOKIE_EXCHANGE);

   client = BIO_ADDR_new();
   if (client == NULL)
      return 0;
   rv = 0;
   for (tries = 0; tries < 20; tries++) {
      rv = DTLSv1_listen(ssl, client);
      if (rv > 0)
         break;
      if (rv < 0 && !dtls_want_retry(ssl, rv)) {
         BIO_ADDR_free(client);
         return 0;
         }
      }
   if (rv <= 0) {
      BIO_ADDR_free(client);
      return 0;
      }
   if (dtls_connect_peer(fd, client, &peer_sa) < 0) {
      BIO_ADDR_free(client);
      return 0;
      }
   BIO_ADDR_free(client);
   BIO_ctrl(bio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, &peer_sa);
   rv = ossl_accept(ssl);
   while (rv <= 0 && dtls_want_retry(ssl, rv))
      rv = ossl_accept(ssl);
   return rv == 1;
}

int ssl_dtls_connect(SSL *ssl, int fd)
{
   BIO *bio;
   struct sockaddr_storage peer;
   socklen_t peerlen = sizeof(peer);
   struct timeval tv;
   int rv;

   /* Unicon UDP sockets are unconnected; connect to open()'s peer. */
   if (sock_udp_connect_saved(fd) < 0)
      return 0;
   if (getpeername(fd, (struct sockaddr *)&peer, &peerlen) < 0)
      return 0;
   bio = BIO_new_dgram(fd, BIO_NOCLOSE);
   if (bio == NULL)
      return 0;
   tv.tv_sec = 5;
   tv.tv_usec = 0;
   BIO_ctrl(bio, BIO_CTRL_DGRAM_SET_RECV_TIMEOUT, 0, &tv);
   BIO_ctrl(bio, BIO_CTRL_DGRAM_SET_SEND_TIMEOUT, 0, &tv);
   BIO_ctrl(bio, BIO_CTRL_DGRAM_SET_CONNECTED, 0, &peer);
   SSL_set_bio(ssl, bio, bio);
   rv = ossl_connect(ssl);
   while (rv <= 0 && dtls_want_retry(ssl, rv))
      rv = ossl_connect(ssl);
   return rv == 1;
}

#else /* OPENSSL_NO_DTLS */

int ssl_dtls_accept(SSL *ssl, int fd)
{
   (void)ssl;
   (void)fd;
   return 0;
}

int ssl_dtls_connect(SSL *ssl, int fd)
{
   (void)ssl;
   (void)fd;
   return 0;
}

#endif /* OPENSSL_NO_DTLS */

#endif /* HAVE_LIBSSL */
