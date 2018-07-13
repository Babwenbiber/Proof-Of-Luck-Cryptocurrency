/*
 * Copyright (C) 2011-2017 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef _APP_H_
#define _APP_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus
#include <string>
#endif


#include "sgx_error.h" //sgx_status_t
#include "sgx_eid.h" //sgx_enclave_id_t



#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

# define TOKEN_FILENAME   "enclave.token"
# define ENCLAVE_FILENAME "enclave.signed.so"

extern sgx_enclave_id_t global_eid; //global enclave id

typedef struct _sgx_errlist_t {
	sgx_status_t err;
    const char *msg;
    const char *sug; //Suggestion
} sgx_errlist_t;

/*Error code returned by sgx_create_enclave*/
static sgx_errlist_t sgx_errlist[] = {
    {
        SGX_ERROR_UNEXPECTED,
        "Unexpected error occurred.",
        NULL
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "Invalid parameter.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "Out of memory.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "Invalid enclave identification.",
        NULL
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "Invalid enclave signature.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "Out of EPC memory.",
        NULL
    },
    {
        SGX_ERROR_NO_DEVICE,
        "Invalid SGX device.",
        "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "Memory map conflicted.",
        NULL
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "Invalid enclave metadata.",
        NULL
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "SGX device was busy.",
        NULL
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "Enclave version was invalid.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "Enclave was not authorized.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "Can't open enclave file.",
        NULL
    },
    {
        SGX_ERROR_INVALID_STATE,
        "The API is invoked in incorrect order or state.",
        NULL
    },
    {
        SGX_ERROR_INVALID_FUNCTION,
        "The ECALL/OCALL function index is incorrect.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_TCS,
        "The enclave is out of TCS.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_CRASHED,
        "The enclave has crashed.",
        NULL
    },
    {
        SGX_ERROR_ECALL_NOT_ALLOWED,
        "ECALL is not allowed at this time. For examples:\nECALL is not public.\nECALL is blocked by the dynamic entry table.\nA nested ECALL is not allowed during global initialization.",
        NULL
    },
    {
        SGX_ERROR_OCALL_NOT_ALLOWED,
        "OCALL is not allowed during exception handling.",
        NULL
    },
    {
        SGX_ERROR_UNDEFINED_SYMBOL,
        "The enclave contains an import table.",
        NULL
    },
    {
        SGX_ERROR_NDEBUG_ENCLAVE,
        "The enclave is signed as product enclave and cannot be created as a debuggable enclave.",
        NULL    },
    {
        SGX_ERROR_MODE_INCOMPATIBLE,
        "The target enclave (32/64 bit or HS/Sim) mode is incompatible with the uRTS mode.",
        NULL
    },
    {
        SGX_ERROR_INVALID_MISC,
        "The MiscSelect/MiscMask settings are incorrect.",
        NULL
    },
    {
        SGX_ERROR_MAC_MISMATCH,
        "Indicates report verification error.",
        NULL
    },
    {
        SGX_ERROR_INVALID_CPUSVN,
        "The CPU SVN is beyond the CPU SVN value of the platform.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ISVSVN,
        "The ISV SVN is greater than the ISV SVN value of the enclave.",
        NULL
    },
    {
        SGX_ERROR_INVALID_KEYNAME,
        "Unsupported key name value.",
        NULL
    },
    {
        SGX_ERROR_SERVICE_UNAVAILABLE,
        "AE service did not respond or the requested service is not supported.",
        NULL
    },
    {
        SGX_ERROR_SERVICE_TIMEOUT,
        "The request to AE service timed out.",
        NULL
    },
    {
        SGX_ERROR_AE_INVALID_EPIDBLOB,
        "Indicates an Intel(R) EPID blob verification error.",
        NULL
    },
    {
        SGX_ERROR_EPID_MEMBER_REVOKED,
        "The Intel(R) EPID group membership has been revoked. The platform is not trusted. Updating the platform and retrying will not remedy the revocation.",
        NULL
    },
    {
        SGX_ERROR_UPDATE_NEEDED,
        "Intel(R) SGX needs to be updated.",
        NULL
    },
    {
        SGX_ERROR_NETWORK_FAILURE,
        "Network connecting or proxy setting issue is encountered.",
        NULL
    },
    {
        SGX_ERROR_AE_SESSION_INVALID,
        "The session is invalid or ended by server.",
        NULL
    },
    {
        SGX_ERROR_BUSY,
        "The requested service is temporarily not available.",
        NULL
    },
    {
        SGX_ERROR_MC_NOT_FOUND,
        "The Monotonic Counter does not exist or has been invalidated.",
        NULL
    },
    {
        SGX_ERROR_MC_NO_ACCESS_RIGHT,
        "The caller does not have the access right to the specified VMC.",
        NULL
    },
    {
        SGX_ERROR_MC_USED_UP,
        "No monotonic counter is available.",
        NULL
    },
    {
        SGX_ERROR_MC_OVER_QUOTA,
        "Monotonic counters reached quota limit.",
        NULL
    },
    {
        SGX_ERROR_KDF_MISMATCH,
        "Key derivation function does not match during key exchange.",
        NULL
    },
    {
        SGX_ERROR_UNRECOGNIZED_PLATFORM,
        "Intel(R) EPID Provisioning failed because the platform was not recognized by the back-end server.",
        NULL
    },
    {
        SGX_ERROR_FILE_BAD_STATUS,
        "The file is in a bad status, run sgx_clearerr to try and fix it.",
        NULL
    },
    {
        SGX_ERROR_FILE_NO_KEY_ID,
        "The Key ID field is all zeros, cannot re-generate the encryption key.",
        NULL
    },
    {
        SGX_ERROR_FILE_NAME_MISMATCH,
        "The current file name is different than the original file name (not allowed, substitution attack).",
        NULL
    },
    {
        SGX_ERROR_FILE_NOT_SGX_FILE,
        "The file is not an Intel SGX file.",
        NULL
    },
    {
        SGX_ERROR_FILE_CANT_OPEN_RECOVERY_FILE,
        "A recovery file cannot be opened, so the flush operation cannot continue (only used when no EXXX is returned).",
        NULL
    },
    {
        SGX_ERROR_FILE_CANT_WRITE_RECOVERY_FILE,
        "A recovery file cannot be written, so the flush operation cannot continue (only used when no EXXX is returned).",
        NULL
    },
    {
        SGX_ERROR_FILE_RECOVERY_NEEDED,
        "When opening the file, recovery is needed, but the recovery process failed.",
        NULL
    },
    {
        SGX_ERROR_FILE_FLUSH_FAILED,
        "fflush operation (to the disk) failed (only used when no EXXX is returned).",
        NULL
    },
    {
        SGX_ERROR_FILE_CLOSE_FAILED,
        "fclose operation (to the disk) failed (only used when no EXXX is returned).",
        NULL
    },
};

typedef struct fileData {
    uint32_t size;
    void *data;
} fileData;

typedef struct blockHeader {
		uint8_t prevBlock[40]; //hash size 20byte / 40 chars in hex
		uint8_t merkleTree[40];
		double luckyNumber;
} blockHeader;



#ifdef __cplusplus
int addProof(std::string merkle, std::string prev, double *luckyNumber, std::string *cert);
bool verifyProof(std::string merkle, std::string prevBlock, double luckyNumber, std::string certificate);
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif

#endif /* !_APP_H_ */
