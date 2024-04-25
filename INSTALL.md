# Install guide
## Prerequisites
- libbsd >= 0.2.0 (except on BSDs, of course)
- libxml >= 2.6
- libedit or libreadline
- openssl >= 1.1.1 (for Argon2id: openssl >= 3.2.0)

### Optional
- libpcre >= 8.12 <= 10.0 (i.e. not PCRE2)
- libscrypt >= 1.14
#### For YubiKey support
  - libykpers-1 from [_yubikey-personalization_](https://github.com/Yubico/yubikey-personalization/) >= 1.20.0
  - libyubikey from [_yubico-c_](https://github.com/Yubico/yubico-c/) >= 1.13
  - v2.2 or later firmware on the security key


## Compile

### BSD
    $ make <OPTIONS>  
    $ make install  

### Linux
    $ make -f Makefile.linux <OPTIONS>
    $ make -f Makefile.linux install

### Options
These are environment variables passed to _make_ (the value doesn't matter, only the definition).

  - `HAVE_PCRE=y`

    Enable Perl-compatible regular expression support while searching. Needs libpcre (see above).

    Note: POSIX regex is supported by default, without the need of the PCRE library and thus this option.
  - `HAVE_LIBSCRYPT=y`

    Turn on scrypt KDF support. Needs libscrypt (see above).
  - `HAVE_YUBIKEY=y`

    Turn on YubiKey challenge-response support. Needs libykpers-1 and libyubikey (see above).
  - `BUNDLED_BCRYPT=y`

    To use the packaged bcrypt implementation (that comes from OpenBSD). This is hardcoded in **`Makefile.linux`**, and on BSDs you should specify this everywhere except on OpenBSD.
  - `OS_OPENBSD=y`

    Only in **`Makefile`**, and currently this only makes the binary link against OpenBSD's libutil (to use the `bcrypt_pbkdf()` function).
  - `EDITLINE=y`

    Use libedit as the line editing function. This needs libedit.
  - `READLINE=y`

    Use readline as the line editing function. This needs libreadline.

    **`Makefile.linux`** uses Readline and **`Makefile`** uses Editline by default.

### Linux example
    $ make -f Makefile.linux HAVE_PCRE=1 HAVE_YUBIKEY=nice HAVE_LIBSCRYPT=yeah && make -f Makefile.linux install

### OpenBSD example
    $ make OS_OPENBSD=1 && make install

### FreeBSD example
    $ make HAVE_LIBSCRYPT=1 BUNDLED_BCRYPT=yesplease && make install
