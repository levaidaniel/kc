# 2.5.3
* Plug some memory leaks.
* `status` command prints YubiKey serial number(s).
* Documentation and manual page fixes.


# 2.5.2
* _SHA-3 512_ support on OpenBSD.
* New '-K' parameter to specify encryption key length.
* Documentation fixes and enhancements to address missing parameters and badly specified arguments (especially with '-Y').


# 2.5.1
* Fix an overflow in the parameter/argument parser on arm* architectures.
* Extend regression tests with checks.


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
* Removed support for the _SHA-1_ KDF.

  If you've been using the _SHA-1_ based KDF, before you upgrade, either `dump` your database XML or change that to anything else with the `passwd` or the `export` command. Note that you would've had to explicitly specify this to use, as the default has always been _SHA-2 512_.
* Added `-R` option to specify the iterations/rounds to be used with the KDFs.
* Added support for the SHA-3 KDF (min. OpenSSL version is now 1.1.1).
* SHA-\* KDF default iterations changed to 100000 and bcrypt KDF default rounds changed to 36.

  Opening older databases where the default was smaller is possible with the new `-R` option. You can use the `passwd` or the `export` command to change this permanently to the new defaults in the database.
* Added support for aes256 _ctr_ encryption cipher mode.
* Removed support for blowfish _ecb_ encryption cipher mode.

  If you've been using the _ecb_ mode for _blowfish_, before you upgrade, either `dump` your database XML or change that to anything else with the `passwd` or the `export` command. Note that you would've had to explicitly specify this to use, as the default has always been _cbc_.
* Added support for POSIX regex functions (it can be used instead of PCRE).
* Documentation and manual page fixes.

# Compatibility issues with older versions
If you find yourself unable to import/open your v2.4 database, keep in mind, that:

* There are new mandatory XML attributes for the database which `kc` will validate during opening. If your database lacks these attributes `kc` will complain and won't open it.

  In this case, try creating a new, empty database, and use the `import` command with the `-o` option which tells `kc` to use the old XML DTD (validation schema) while opening (i.e. importing) the database. This way you'll end up with your secrets in the new database, while having the new mandatory attributes reset.
* The iterations/rounds numbers have changed for the KDFs. If you created your database with v2.4, then it's possible you used the default settings which was _5000 iterations_ for SHA-* and _16 rounds_ for bcrypt.

  Try specifying the old number - depending on the KDF you're using - with the new `-R` option. Note that `-R` is supported as a `kc` startup option **and** as e.g. an option for the `import` command.

Example:

Running v2.5, creating a new then importing a v2.4 database that was using the default settings (sha512 KDF, aes256 cipher, cbc mode):

```
$ kc
Creating 'default.kcd'
Using 'default.kcd' database.
New password (empty to cancel):
New password again (empty to cancel):
Initializing...
Database file: default.kcd (default.kcd)
XML structure size: 148 bytes
Security key(s): no
Password: yes
SSH agent: no
Password function: sha512 (100000 iterations)
Encryption: aes256, cbc
Read-only: no
Modified: yes
<default% > import -o -R 5000 -k passwords.kcd
Reading database...
Opening 'passwords.kcd'
Password:
Decrypting...
Checking database...
Counting keys and keychains...
Import finished.
<passwords% >
```
