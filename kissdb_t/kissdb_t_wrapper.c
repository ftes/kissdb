#include "sgx_lib_t_util.h"

#include "kissdb.h"
#include "kissdb_t.h"

// database "singleton" that is operated on by this enclave
KISSDB db;

int KISSDB_open_ecall(const char* path, int mode, unsigned long int hash_table_size, unsigned long int key_size, unsigned long int value_size) {
  KISSDB_HEADER_SIZE = get_sealed_data_size((sizeof(uint64_t) * 3) + 4);
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

int KISSDB_Iterator_next_ecall(unsigned long int hashtable_number, unsigned long int hashtable_index, void* kbuf, void* vbuf, unsigned long int key_size, unsigned long int value_size) {
  KISSDB_Iterator iter;
  iter.db = &db;
  iter.h_no = hashtable_number;
  iter.h_idx = hashtable_index;
  return KISSDB_Iterator_next(&iter, kbuf, vbuf);
}