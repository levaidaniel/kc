# 2.5.0
* OpenSSL-related fixes

  Support OpenSSL v3.
  OpenSSL 3 needs us to load the legacy OpenSSL provider to use the blowfish cipher.

  Renamed a couple of functions and we check for a return value of functions that had one recently implemented.
* Unify error messages in that they now appear on standard error output and they're prefixed with `ERROR:`..
* Increase maximum allowed password length to 1024 bytes.
* Implement OpenSSH agent support to acquire a database password from a signature based on the chosen SSH private key.

  `export`/`import`/`passwd` commands also support this.
* Support YubiKey challenge-response mode for database passwords.

  `export`/`import`/`passwd` commands also support this.  
* We won't strip the last new-line character when reading the password from a file.
* `passwd` command can now change encryption cipher and mode as well.
* The kc XML document type definition has changed so that now attributes created, modified and description are mandatory.
* `import` command now has a `-o` option to turn on legacy database parsing with the old kc DTD.

  One can salvage their legacy database(s) and fill in the missing attributes.
* `new` and `cnew` commands support spaces in key and keychain names (respectively) when specifying them on the command line.

  With the exception being that any trailing spaces from a keychain name will be stripped.
* `info` command shows how many lines there are in the value, and how many keys there are in the keychain.
* `c` command supports keychain names with spaces in them, with the exception being that any trailing spaces from a keychain name will be stripped.
* Removed support for the SHA-1 KDF.

  If you've been using the SHA-1 based KDF, before you upgrade, you need to change that to anything else with the 'passwd' or the 'export' command. Note that you would've had to explicitly specify this to use, as the default has always been SHA-2 512.
* Added '-R' option to specify the iterations/rounds to be used with the KDFs.
* Added support for the SHA-3 KDF (min. OpenSSL version is now 1.1.1).
* SHA-\* KDF default iterations changed to 100000 and bcrypt KDF default rounds changed to 36.

  Opening older datases where the defaults were smaller is possible with the '-R' option.
* Added support for aes256 ctr encryption cipher mode.
* Removed support for blowfish ecb encryption cipher mode.
* Added support for POSIX regex functions (it can be used instead of PCRE).
* Documentation and manual page fixes.
