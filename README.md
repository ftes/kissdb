# About
This fork of [KissDB](https://github.com/adamierymenko/kissdb) provides a secure database using Intel SGX.


# Technical Details
The database executes entirely inside an SGX enclave. Data is not written into a `.db` file in the clear, but encrypted.

- one enclave corresponds to exactly one `KISSDB` instance
- `KISSDB_open` creates a new enclave and loads (if present) the encrypted header data from an exisiting `.db` file into secure memory
  - the encryption key is known only to the enclave (`EGETKEY`, sealed data)
- `KISSDB_put` and `_get` encrypt / decrypt data in the `.db` file
- `KISSDB_close` frees the memory and destroys the enclave
- the in-memory arrays of hash tables of `KISSDB` instances remain in enclave memory, the untrusted application only has the `enclave id`
- `KISSDB_Iterator` instances live in untrusted memory
- non-sensitive data (header, hash tables) are stored in the `.db` file in the clear


# Architecture
<img alt="Architecture" src="https://lh3.googleusercontent.com/QGM6cn-fhWcvfSaecv-8e_oHiSQrh4grmdrMZamRMNK6KMASUFkZCEiLX9WXFZNhZpvxusL3mhe0TA=w925-h991-no" width="500px"/>


# ToDos
- djb2 is not a crypto hash function, thus the hash table leaks information about keys
  - use crypto hash function (no informaion about keys leaked, but still problematic if small or non-uniformly distributed key space)
  - compute hash of encrypted key
- attest and establish secure channel / provision keys -> currently the enclave hands out secret data to anybody
- assure file integrity and freshness


# Design considerations
A better approach would be to minimize the size of the TCB, and execute only security critical functionality inside the enclave.
This reduces the risk of a bug compromising the security of the entire application.


# Information leakage
- offset of the first inserted item is constant
  - if the key of the first inserted item is known (e.g. always the same test item), the encryption key could be derived (known plaintext attack)
- hashes of plaintext keys (in the sense of DB primary keys) are stored in the hash table
  - the key of an encrypted data item may be determined through the hash table if a non-cryptographic hash function is used, or if there is a small or non-uniformly distributed key space
  - this again means the encryption key could be derived (known plaintext attack)
  - *see ToDos -> use crypto-hash of encrypted key*


# Obstacles
- hashtables on disk are navigated via `fseek` operations, which assume knowledge of how large the structures written to disk are
  - writing out encrypted versions of metadata such as hashtables breaks this assumption
  - **solution:** metadata (header and hash tables) are written to `.db` in the clear


# Diff to original code base
```
kissdb/kissdb_t$
wget --no-check-certificate -O kissdb.c.orig http://raw.githubusercontent.com/adamierymenko/kissdb/master/kissdb.c; \
unix2dos kissdb.c.orig; \
diff kissdb.c kissdb.c.orig
```

Changes:
- use unencrypted writes/reads for metadata (header, hash tables)
- remove `KISSDB_Iterator_init`, as this is done in untrusted code

```
< #include "sgx_lib_t_stdio.h"
<
80d76
<     /* header data is not sensitive -> unencrypted */
84c80
<                       if (fwrite_unencrypted(tmp2,4,1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
---
>                       if (fwrite(tmp2,4,1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
86c82
<                       if (fwrite_unencrypted(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
---
>                       if (fwrite(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
88c84
<                       if (fwrite_unencrypted(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
---
>                       if (fwrite(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
90c86
<                       if (fwrite_unencrypted(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
---
>                       if (fwrite(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
98c94
<               if (fread_unencrypted(tmp2,4,1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
---
>               if (fread(tmp2,4,1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
103c99
<               if (fread_unencrypted(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
---
>               if (fread(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
109c105
<               if (fread_unencrypted(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
---
>               if (fread(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
115c111
<               if (fread_unencrypted(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
---
>               if (fread(&tmp,sizeof(uint64_t),1,db->f) != 1) { fclose(db->f); return KISSDB_ERROR_IO; }
135,136c131
<   /* hash tables are stored unencrypted */
<       while (fread_unencrypted(httmp,db->hash_table_size_bytes,1,db->f) == 1) {
---
>       while (fread(httmp,db->hash_table_size_bytes,1,db->f) == 1) {
263,264c258
<       /* hash tables are stored unencrypted */
<                       if (fwrite_unencrypted(&endoffset,sizeof(uint64_t),1,db->f) != 1)
---
>                       if (fwrite(&endoffset,sizeof(uint64_t),1,db->f) != 1)
292,293c286
<   /* hash tables are stored unencrypted */
<       if (fwrite_unencrypted(cur_hash_table,db->hash_table_size_bytes,1,db->f) != 1)
---
>       if (fwrite(cur_hash_table,db->hash_table_size_bytes,1,db->f) != 1)
304,305c297
<     /* hash tables are stored unencrypted */
<               if (fwrite_unencrypted(&endoffset,sizeof(uint64_t),1,db->f) != 1)
---
>               if (fwrite(&endoffset,sizeof(uint64_t),1,db->f) != 1)
316a309,315
> void KISSDB_Iterator_init(KISSDB *db,KISSDB_Iterator *dbi)
> {
>       dbi->db = db;
>       dbi->h_no = 0;
>       dbi->h_idx = 0;
> }
>
343c342,452
< }
\ No newline at end of file
---
> }
>
> #ifdef KISSDB_TEST
... test code ...
```


# Unencrypted vs. Encrypted `.db.` files

## File structure overview
<img alt="File Structure" src="https://lh3.googleusercontent.com/1rly1xCvtc9j2PbHEHMwQacwXNDDOZcSWR76vNomv4Cyk0ztj9ycfHJhdMXbVRjWlB9BK2BsQdqpnS49PsBvI8YX5E6jBTcqvIkaZIVGx-xcqBT58HoowKtpB5pyR6GCvtdifgJm9RRIa0ua2qzXB1xpCwxXbxtYDfwMeeYKeEGiAB7QAjjYffdE8bmozjytmus9GdD2hqAmjcuIN7cJK2m5wZ9XrGaWZQGcuzh_befNa6Jk6eYNbaZEE8bXhZajKGPRjSdAloxfG1RL9BIesPJOA9xQEWSSxSOraFNeCUlzfgqsvu-PE9bAjES4kr9hLHHX-L5B50R-bCSCQugIykXXTfPtJvtRNdOUueG4c01fAh2oKdoBgHFR0oM3TChZX3w6O5NkeWhuwMmHbf1pzs4cjijcCxmuTjAJrs0gSi9lgLNNiJM0BF-B6ytnKnRzuNtFyYlAkSDkar0x3HBe_ABub2sDJHIJo1ou9yiaxOGAOVM4q7D1BpQvT1EUqvh4d25Op96V7K1TwPoFjDW94TTC_ZUNxL3HmCbkgnRNzNnnm9Ako0puKhLBsMTfeDqRsT2jDA=w1051-h993-no" width="500px" />

## Unencrypted (800kB)
Remarks:

- set `SGX_INSECURE_IO_OPERATIONS` macro in `kissdb_t` project to observe unencrypted `.db.` file
- file encoding is *little endian*: least significant byte (1 byte = 2 hex digits) first, e.g. `00 04 00 .. 00` is `00 .. 00 04 00` in big endian, which is 1,024 decimal
- expected offset of first data item in file: `header_size (28byte) + first_hash_table_page_size (8200byte)`
  - `= 8228 decimal = 0x2024 hex (24 20 little endian)`
- expected offset of hash table entry for key `0`: `header_size(28byte) + bucket_index(djb2_hash(0) mod 1024) * bucket_size(8byte)`
  - `= 28 + 517 * 8 = 4164 decimal = 0x1044 hex`

```
Offset(h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F

00000000  4B 64 42 02 00 04 00 00 00 00 00 00 08 00 00 00  KdB.............  // header: KDB2, hash table size (0004...), key size (08...) = 8 byte
00000010  00 00 00 00 40 00 00 00 00 00 00 00 0C 14 05 00  ....@...........  // header: value size (40...) = 64 byte
00000020  00 00 00 00 5C 0A 07 00 00 00 00 00 4C 4C 00 00  ....\.......LL..  // BEGIN first hash table page (incl. `0C 14 05 00` in previous line)
00000030  00 00 00 00 CC 3F 01 00 00 00 00 00 4C F6 03 00  ....Ì?......Lö..
00000040  00 00 00 00 9C EC 05 00 00 00 00 00 EC E2 07 00  ....œì......ìâ..
...
00001030  00 00 00 00 7C 7B 06 00 00 00 00 00 DC 5E 00 00  ....|{......Ü^..
00001040  00 00 00 00 24 20 00 00 00 00 00 00 64 47 03 00  ....$ ......dG..  // hash table entry for item with key `0` is `24 20`, at offset `1044` (as expected)
00001050  00 00 00 00 BC 5D 05 00 00 00 00 00 0C 54 07 00  ....¼].......T..
...
00001FF0  00 00 00 00 7C 3B 04 00 00 00 00 00 CC 31 06 00  ....|;......Ì1..
00002000  00 00 00 00 E4 65 00 00 00 00 00 00 2C 27 00 00  ....äe......,'..
00002010  00 00 00 00 AC DD 02 00 00 00 00 00 DC 70 00 00  ....¬Ý......Üp..  // END first hash table page, last entry is offset of next hash table page `DC 70` -> `70 DC` (little endian)
00002020  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................  // BEGIN first entry at 2024: key (0), value ([0,0,0,0,0,0,0,0])
00002030  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................  // entry is 72byte (0x48 hex) long, next entry starts at `0x2024 + 0x48 = 0x206C`
00002040  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................  // 72 byte: 9 64-bit integers (1 key, 8 value) -> `9 * 64 / 8 = 72byte`
00002050  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
00002060  00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00  ................  // BEGIN second entry
00002070  00 00 00 00 01 00 00 00 00 00 00 00 01 00 00 00  ................
00002080  00 00 00 00 01 00 00 00 00 00 00 00 01 00 00 00  ................
00002090  00 00 00 00 01 00 00 00 00 00 00 00 01 00 00 00  ................
000020A0  00 00 00 00 01 00 00 00 00 00 00 00 01 00 00 00  ................ 
000020B0  00 00 00 00 02 00 00 00 00 00 00 00 02 00 00 00  ................  // BEGIN third entry
...
000070C0  00 00 00 00 1E 01 00 00 00 00 00 00 1E 01 00 00  ................
000070D0  00 00 00 00 1E 01 00 00 00 00 00 00 C4 64 05 00  ............Äd..  // BEGIN second hash table page at offset 70DC (as linked from first page)
000070E0  00 00 00 00 14 5B 07 00 00 00 00 00 0C BD 00 00  .....[.......½..  
000070F0  00 00 00 00 8C B0 01 00 00 00 00 00 04 47 04 00  ....Œ°.......G..
...
```

## Encrypted (12MB)
```
Offset(h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F

00000000  4B 64 42 02 00 04 00 00 00 00 00 00 08 00 00 00  KdB.............  // header: identical (unencrypted)
00000010  00 00 00 00 40 00 00 00 00 00 00 00 0C 14 05 00  ....@...........
00000020  00 00 00 00 5C 0A 07 00 00 00 00 00 4C 4C 00 00  ....\.......LL..  // BEGIN first hash table page (unencrypted)
00000030  00 00 00 00 EC D5 0E 00 00 00 00 00 2C 12 30 00  ....ìÕ......,.0.
00000040  00 00 00 00 3C 8E 50 00 00 00 00 00 4C 0A 71 00  ....<ŽP.....L.q.
...
00001030  00 00 00 00 9C CB 59 00 00 00 00 00 7C 2E 04 00  ....œËY.....|...
00001040  00 00 00 00 24 20 00 00 00 00 00 00 C4 B4 26 00  ....$ ......Ä´&.  // hash table entry for first inserted item has not changed (offset remains the same): `24 20`
00001050  00 00 00 00 DC 50 47 00 00 00 00 00 EC CC 67 00  ....ÜPG.....ìÌg.
...
00001FF0  00 00 00 00 9C 8B 34 00 00 00 00 00 AC 07 55 00  ....œ‹4.....¬.U
00002000  00 00 00 00 E4 A2 04 00 00 00 00 00 8C 94 00 00  ....ä¢......Œ”..
00002010  00 00 00 00 CC D0 21 00 00 00 00 00 7C 58 05 00  ....ÌÐ!.....|X..  // END first hash table page
00002020  00 00 00 00 04 00 02 00 00 00 00 00 48 20 F3 37  ............H ó7  // BEGIN first entry at 2024: key_name=4, key_policy=2, isv_svn=0, ...
00002030  6A E6 B2 F2 03 4D 3B 7A 4B 48 A7 78 CB FF FF FF  jæ²ò.M;zKH§xËÿÿÿ  // encrypted entry is 568byte (0x238 hex) long, next entry starts at `0x2024 + 0x238 = 0x225C`
00002040  FF FF FF FF 00 00 00 00 00 00 00 00 66 2F A7 72  ÿÿÿÿ........f/§r
00002050  54 63 FD 12 F2 5B 43 12 C0 60 F9 61 3E 45 9F 0F  Tcý.ò[C.À`ùa>EŸ.
...
00002240  00 00 00 00 B3 AD A1 4C 81 86 64 90 31 09 CA A7  ....³.¡L.†d.1.Ê§
00002250  B7 6C D4 7C 6E A0 D9 BA BB FF 46 B9 04 00 02 00  ·lÔ|n Ùº»ÿF¹....  // BEGIN second entry at 225C: same magic numbers (4, 2, ...)
00002260  00 00 00 00 48 20 F3 37 6A E6 B2 F2 03 4D 3B 7A  ....H ó7jæ²ò.M;z
00002270  4B 48 A7 78 CB FF FF FF FF FF FF FF 00 00 00 00  KH§xËÿÿÿÿÿÿÿ....
...
```