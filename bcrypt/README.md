bcrypt
======

All of this stuff was copied over from OpenBSD. Licenses, copyrights are stated
in the files themselves.

The files were only modified where it was necessary for compiling them cleanly
on Linux, to make the syncing with the source as easy as possible:

* The `bcrypt_pbkdf()` function's first parameter `pass` was changed to
  `unsigned char *`, so the use of `SHA512Update()` with the expected
  `u_int8_t` parameter won't emit a compiler warning.
* Removed OpenBSD specific function attributes (eg.: `__bounded__`).
* Removed the inclusion of some unused header files.

`src/lib/libutil/bcrypt_pbkdf.c` -> `bcrypt_pbkdf.c`
`src/sys/crypto/blf.h` -> `blf.h`
`src/sys/crypto/blf.c` -> `blf.c`
`src/sys/lib/libsa/explicit_bzero.c` -> `explicit_bzero.c`
`src/sys/crypto/sha2.h` -> `sha2.h`
`src/sys/crypto/sha2.c` -> `sha2.c`

2013.12.07.
