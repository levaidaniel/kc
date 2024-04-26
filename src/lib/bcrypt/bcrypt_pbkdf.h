#ifndef _BCRYPT_PBKDF_H
#define _BCRYPT_PBKDF_H

int	bcrypt_pbkdf(const char *, size_t, const uint8_t *, size_t,
    uint8_t *, size_t, unsigned int);

#endif
