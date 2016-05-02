#ifndef KISSDB_ENCLAVE_H
#define KISSDB_ENCLAVE_H

#ifdef _EXPORT
// for some (weird) reason, the dllexport does not cause the function to be exported in the dll, and thus no .lib is generated
// therefor, dll_exports.def is used in addition (though it should be redundant)
#define DllExport   __declspec( dllexport )
#else
#define DllExport   __declspec( dllimport )
#endif

void DllExport test_enclave();

#endif