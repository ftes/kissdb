#include <stdlib.h>
#include <string.h>

#include "sgx_tcrypto.h"
#include "sgx_trts.h"
#include "sgx_utils.h"

#include "sgx_lib_t_debug.h"

#include "kissdb_t.h"

#define BUF_SIZE 10
#define CTR_SIZE 128

void manual_encryption() {
  char plaintext[BUF_SIZE] = "encrypt";
  char encrypted[BUF_SIZE];
  char decrypted[BUF_SIZE];
  sgx_aes_ctr_128bit_key_t key;
  sgx_key_request_t request;
  sgx_report_t report;
  sgx_key_id_t key_id = {49034,92,165414,1654,23,7897652}; // should be random
  sgx_attributes_t attributes;
  uint8_t ctr[CTR_SIZE] = {0}; //ToDo is this the correct counter size?


  // create report to get cpu_svn
  sgx_create_report(NULL, NULL, &report);

  attributes.flags = SGX_FLAGS_INITTED | SGX_FLAGS_DEBUG;

  request.key_name = SGX_KEYSELECT_SEAL;
  request.key_policy = SGX_KEYPOLICY_MRENCLAVE; //only this enclave can rederive the key
  request.cpu_svn = report.body.cpu_svn;
  request.isv_svn = report.body.isv_svn;
  request.key_id = key_id;
  request.attribute_mask = attributes;

  sgx_get_key(&request, &key);

  sgx_aes_ctr_encrypt(&key, (uint8_t*) plaintext, BUF_SIZE, ctr, 1, (uint8_t*) encrypted);
  memset(ctr, 0, CTR_SIZE);
  sgx_aes_ctr_decrypt(&key, (uint8_t*) encrypted, BUF_SIZE, ctr, 1, (uint8_t*) decrypted);
  
  printf("Plaintext: %.10s\n", plaintext);
  printf("Encrypted: %.10s\n", encrypted);
  printf("Decrypted: %.10s\n", decrypted);
}