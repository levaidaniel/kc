keychain
========

A console based password storing application using an encrypted XML document as its database.

Features include (but not limited to):
* fixed string or regex based search
* encrypted or plain text database import/export
* fuzzy displaying of passwords if you suspect someone might watch your monitor
* copy passwords to various clipboards
* multiple keychains per database
* the usual editing functions
* compiles and works on \*BSD and Linux
* editline (libedit) and readline support
* clean and simple CLI with command and keychain completion
* OpenSSH agent support for protecting the database
* YubiKey challenge-response support for protecting the database
