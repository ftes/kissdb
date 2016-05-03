# About
This fork of [KissDB](https://github.com/adamierymenko/kissdb) provides a secure database using Intel SGX.


# Technical Details
The database executes entirely inside an SGX enclave. Data is not written into a `.db` file in the clear, but encrypted.

- one enclave corresponds to exactly one `KISSDB` instance
- `KISSDB_open` creates a new enclave and loads (if present) the encrypted header data from an exisiting `.db` file into secure memory
  - the encryption key is known only to the enclave (`EGETKEY`, sealed data)
- `KISSDB_put` and `_get` encrypt / decrypt data in the `.db` file
- `KISSDB_close` frees the memory and destroys the enclave
- the hash tables of `KISSDB` instances remain in enclave memory, the untrusted application only has the `enclave id`
- `KISSDB_Iterator` instances live in untrusted memory


# ToDo
- attest and establish secure channel / provision keys
- assure file integrity and freshness


# Design considerations
A better approach would be to minimize the size of the TCB, and execute only security critical functionality inside the enclave.
This reduces the risk of a bug compromising the security of the entire application.


# Implementation Problems
- hash tables store `key->offset` mapping of database entries
  - vanilla KissDB assumes it knows how large the elements are it writes into a `.db` file
  	- the `offset` for an item is calculated based on that assumption (e.g. offset for first item: header_size + hash_table_size + 1)
  - writing out encrypted versions of data (metadata and items) breaks that assumption
    - the offset are actually larger than assumed, because the encrypted data takes up more space