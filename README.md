kc
==

A console based password storing application using an encrypted XML document as its database.
It has a clean and simple command line interface and works on \*BSD and Linux.

Features include:
* fixed string or regex based search
* OpenSSH agent support for protecting the database
* YubiKey challenge-response support for protecting the database
* encrypted or plain text database import/export
* multiple keychains per database
* copy passwords to various clipboards
* editline (libedit) and readline support
