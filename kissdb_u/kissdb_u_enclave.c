#include <tchar.h>
#include <sgx_urts.h>
#include <direct.h>
#include <windows.h>

#include "sgx_lib_u_util.h"
#include "kissdb_u.h"

#include "kissdb.h"

#define ENCLAVE_FILE _T("kissdb_t.signed.dll")

void test_enclave() {
  sgx_enclave_id_t eid;
  launch_enclave(ENCLAVE_FILE, &eid);
  dummy_root(eid);
  destroy_enclave(eid);
  getchar();
}