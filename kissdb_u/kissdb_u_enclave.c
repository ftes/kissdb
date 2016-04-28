#include <tchar.h>
#include <sgx_urts.h>
#include <direct.h>

#include "sgx_lib_u_util.h"
#include "kissdb_u.h"

#include "kissdb.h"

#define ENCLAVE_FILE _T("kissdb_t.signed.dll")

void test_enclave() {
  sgx_enclave_id_t eid;
  char currentDir[200];

  _getcwd(currentDir, sizeof(currentDir));
  puts(currentDir);
  launch_enclave(ENCLAVE_FILE, &eid);
  dummy_root(eid);
  destroy_enclave(eid);
  getchar();
}

void main() {
  test_enclave();
}