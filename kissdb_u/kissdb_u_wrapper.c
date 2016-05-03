#include <tchar.h>

#include "kissdb.h"
#include "kissdb_u.h"
#include "sgx_lib_u_util.h"

#define ENCLAVE_FILE _T("kissdb_t.signed.dll")

int KISSDB_open(KISSDB *db,	const char *path,	int mode,	unsigned long hash_table_size, unsigned long key_size, unsigned long value_size) {
  int retval;
  
	db->key_size = key_size;
	db->value_size = value_size;

  launch_enclave(ENCLAVE_FILE, &(db->eid));
  KISSDB_open_ecall(db->eid, &retval, path, mode, hash_table_size, key_size, value_size);

  return retval;
}


void KISSDB_close(KISSDB *db) {
  // freeing memory and memestting as performed by this ecall is not really necessary, as we are destroying enclave anyway
  KISSDB_close_ecall(db->eid);

  destroy_enclave(db->eid);
  memset(db,0,sizeof(KISSDB));
}

int KISSDB_get(KISSDB *db, const void *key, void *vbuf) {
  int retval;
  KISSDB_get_ecall(db->eid, &retval, key, vbuf, db->key_size, db->value_size);
  return retval;
}

int KISSDB_put(KISSDB *db, const void *key, const void *value) {
  int retval;
  KISSDB_put_ecall(db->eid, &retval, key, value, db->key_size, db->value_size);
  return retval;
}

void KISSDB_Iterator_init(KISSDB *db,KISSDB_Iterator *dbi) {
  dbi->db = db;
	dbi->h_no = 0;
	dbi->h_idx = 0;
}

int KISSDB_Iterator_next(KISSDB_Iterator *dbi,void *kbuf,void *vbuf) {
  int retval;
  KISSDB *db = dbi->db;
  dbi->db = NULL; //don't pass DB into enclave (different struct definitions)
  KISSDB_Iterator_next_ecall(db->eid, &retval, dbi, kbuf, vbuf, db->key_size, db->value_size);
  dbi->db = db;
  return retval;
}