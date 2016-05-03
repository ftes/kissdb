#include "sgx_lib_t_util.h"

#include "kissdb.h"
#include "kissdb_t.h"

// database "singleton" that is operated on by this enclave
KISSDB db;

int KISSDB_open_ecall(const char* path, int mode, unsigned long int hash_table_size, unsigned long int key_size, unsigned long int value_size) {
  return KISSDB_open(&db, path, mode, hash_table_size, key_size, value_size);
}

void KISSDB_close_ecall() {
  KISSDB_close(&db);
}

int KISSDB_get_ecall(const void* key, void* vbuf, unsigned long int key_size, unsigned long int value_size) {
  return KISSDB_get(&db, key, vbuf);
}

int KISSDB_put_ecall(const void* key, const void* value, unsigned long int key_size, unsigned long int value_size) {
  return KISSDB_put(&db, key, value);
}

int KISSDB_Iterator_next_ecall(KISSDB_Iterator *dbi, void* kbuf, void* vbuf, unsigned long int key_size, unsigned long int value_size) {
  int retval;
  dbi->db = &db;
  retval = KISSDB_Iterator_next(dbi, kbuf, vbuf);
  dbi->db = NULL;
  return retval;
}