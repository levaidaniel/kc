# 2.5.0
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
* `info` command shows how many lines there are in the value, and how many keys there are in the keychain.
* `c` command supports keychain names with spaces in them.
* Removed support for the SHA-1 KDF.
* Added support for the SHA-3 KDF (min. OpenSSL version is 1.1.1).
* Added support for POSIX regex functions (it can be used instead of PCRE).
* Documentation and manual page fixes.
