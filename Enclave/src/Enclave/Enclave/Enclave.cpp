#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */
#include <string.h>
#include <sgx_trts.h>
#include <stdint.h>
#include <sgx_tae_service.h>
#include <sgx_tcrypto.h>
#include <sgx_tseal.h>
#include <sgx_utils.h>

#include "Enclave.h"
#include "Enclave_t.h"  /* print_string */

/*Returns a random number between 0 and 1 */
sgx_status_t ecall_get_proof(blockHeader *header, uint32_t headerSize, uint8_t *certificate) {

    uint32_t mcValue, mcValueOld;
    unsigned int waitingPeriod;
    double luckyNumber = 0;

    do {
        status = sgx_create_pse_session();
        if (status == SGX_ERROR_BUSY) {
            ocall_sleep(10); //if the service is busy we wait 10 seconds before retrying
        } else if (status != SGX_SUCCESS && status != SGX_ERROR_BUSY) {
            return status;
        }
    } while(status == SGX_ERROR_BUSY);

    if (loadData() != SGX_SUCCESS) {
        //create monotonic counter to prevent concurrency
        do {
            status = sgx_create_monotonic_counter(&keys.mc, &mcValue);
            if (status == SGX_ERROR_BUSY) {
                ocall_sleep(10); //if the service is busy we wait 10 seconds before retrying
            } else if (status != SGX_SUCCESS && status != SGX_ERROR_BUSY) {
                return status;
            }
        } while(status == SGX_ERROR_BUSY);
        /*
          Generates public and private Key for signing blocks.
          If the keys will be new generated and not known to the
          network the block wont be accepted. This supports preventing
          concurrency. The keys will be new generated if and only if
          the file with the keys does not exist.
        */
        status = createKeys();
        if (status != SGX_SUCCESS) {
            return status;
        }
        int ret;
        ocall_save_public_key(&ret, (uint8_t *) &keys.publicKey, sizeof(sgx_ec256_public_t));
        if (ret < 0) {
          return SGX_ERROR_UNEXPECTED;
        }
    }

    do {
        status = sgx_increment_monotonic_counter(&keys.mc, &mcValueOld);
        if (status == SGX_ERROR_BUSY) {
            ocall_sleep(10); //if the service is busy we wait 10 seconds before retrying
        } else if (status != SGX_SUCCESS && status != SGX_ERROR_BUSY) {
            return status;
        }
    } while(status == SGX_ERROR_BUSY);

    status = saveData();
    if (status != SGX_SUCCESS) {
      return status;
    }

    status = getLuckyNumber(&luckyNumber);
    if (status != SGX_SUCCESS) {
        return status;
    }
    ocall_print_string("(Enclave) Lucky number generated!\n");

    //calculate waiting period
    waitingPeriod = round(MAX_WAITING_TIME - luckyNumber * MAX_WAITING_TIME);

    //waiting a time linear to the lucky number before returning the value
    status = wait(waitingPeriod);
    if (status != SGX_SUCCESS) {
        return status;
    }
    ocall_print_string("(Enclave) Waiting period expired!\n");

    status = loadData();
    if (status != SGX_SUCCESS) {
      return status;
    }

    status =  sgx_read_monotonic_counter(&keys.mc, &mcValue);
    if (status != SGX_SUCCESS) {
    return status;
    }

    status = sgx_close_pse_session();
    if (status != SGX_SUCCESS) {
        return status;
    }

    //checks if another enclave is running
    if (mcValue != mcValueOld) {
        return SGX_ERROR_MC_NOT_FOUND;
    } else {

        header->luckyNumber = luckyNumber;

        sgx_sha256_hash_t *hash = (sgx_sha256_hash_t *) malloc(sizeof(sgx_sha256_hash_t));
        uint8_t *signature = (uint8_t *) malloc(sizeof(sgx_ec256_signature_t));

        status = sgx_sha256_msg((uint8_t *) header, headerSize, hash);
        if (status != SGX_SUCCESS) {
            return status;
        }

        status = sign((uint8_t *) hash, sizeof(sgx_sha256_hash_t), signature);
        if (status != SGX_SUCCESS) {
            return status;
        }

        memcpy(certificate, signature, sizeof(sgx_ec256_signature_t));

        /*sgx_report_data_t reportData = {0};
        memcpy(reportData.d, signature, sizeof(sgx_ec256_signature_t));
        sgx_report_t *report = (sgx_report_t *) malloc(sizeof(sgx_report_t));
        status = sgx_create_report(NULL, &reportData, report);
        if (status != SGX_SUCCESS) {
            return status;
        }*/

        free(hash);
        free(signature);

        return SGX_SUCCESS;
    }
}

/*Generates the lucky number*/
sgx_status_t getLuckyNumber(double *luckyNumber) {

    uint64_t randomNumber;

    status = sgx_read_rand((unsigned char*) &randomNumber, 8); //generates random positive 64bit integer
    if (status != SGX_SUCCESS) {
        return status;
    }
    *luckyNumber = (double) randomNumber/UINT64_MAX; //maps positive 64bit integer between 0 and 1
    return SGX_SUCCESS;
}

/*Calls a sleep function that sleeps for waitingPeriod seconds. If the time nonces are not the same
it starts from the beginning*/
sgx_status_t wait(unsigned int waitingPeriod) {

    sgx_time_t currentTime, oldTime;
    sgx_time_source_nonce_t timeSourceNonce = {0};
    sgx_time_source_nonce_t oldTimeSourceNonce = {0};

    status = sgx_get_trusted_time(&oldTime, &oldTimeSourceNonce);
    if (status != SGX_SUCCESS) {
        return status;
    }
    status = sgx_get_trusted_time(&currentTime, &timeSourceNonce);
    if (status != SGX_SUCCESS) {
        return status;
    }
    ocall_print_string("(Enclave) Starting waiting period!\n");
    do {
        //checks whether the trusted times are compareable, if not the waiting period starts again
        if (memcmp(&oldTimeSourceNonce, &timeSourceNonce, sizeof(sgx_time_source_nonce_t)) == 0) {
            //ocall sleep
            ocall_sleep(waitingPeriod);
            status = sgx_get_trusted_time(&currentTime, &timeSourceNonce);
            if (status != SGX_SUCCESS) {
                return status;
            }
        } else {
            return wait(waitingPeriod);
        }
    } while (currentTime - oldTime < waitingPeriod);

    /*
      Checks whether the trusted times are compareable, if not the waiting period starts again.
      We have to check it again, because after the waiting time expired the loop will be exited
      without checking the correctness of the trusted times.
    */
    if (memcmp(&oldTimeSourceNonce, &timeSourceNonce, sizeof(sgx_time_source_nonce_t)) != 0) {
        return wait(waitingPeriod);
    }

    return SGX_SUCCESS;
}

/*Rounds a double down if < 0.5 and up if >= 0.5 and returns the result*/
unsigned int round(double number) {

    unsigned int roundNumber;

    roundNumber = (unsigned int) number;
    if (number - roundNumber < 0.5) {
        return roundNumber;
    } else {
        return roundNumber + 1;
    }
}

/*Seals the data and saves the sealed data into a file*/
sgx_status_t saveData() {

    int ret;

    // Allocate space for sealing
    uint8_t dataSize = sizeof(secretData);
    uint32_t sealedDataSize = sgx_calc_sealed_data_size(0, dataSize);
    sgx_sealed_data_t *sealedData = (sgx_sealed_data_t*) malloc(sealedDataSize);

    // Seal the data
    status = sgx_seal_data(0, NULL, dataSize, (uint8_t *) &keys, sealedDataSize, sealedData);
    if (status != SGX_SUCCESS) {
        return status;
    }

    //Save data in file
    ocall_save_in_file(&ret, FILENAME, (uint8_t *) sealedData, sealedDataSize);
    if (ret < 0) {
      return SGX_ERROR_UNEXPECTED;
    }

    free(sealedData);

    return SGX_SUCCESS;
}

/*Loads the sealed data from file and unseals it*/
sgx_status_t loadData() {

    fileData *temp = (fileData *) malloc(sizeof(fileData));
    ocall_read_from_file(temp, FILENAME); //load sealed data from file
    if (temp->size == 0) {
        return SGX_ERROR_UNEXPECTED;
    }
    uint8_t *data = (uint8_t *) malloc(temp->size);
    uint8_t *sdata = (uint8_t *) malloc(temp->size);
    memcpy(sdata, temp->data, temp->size); //the data must be inside the enclave for unsealing

    status = sgx_unseal_data((sgx_sealed_data_t *) sdata, NULL, NULL, data, &temp->size);
    if (status != SGX_SUCCESS) {
        return status;
    }
    memcpy(&keys, data, sizeof(secretData));

    free(temp);
    free(data);
    free(sdata);

    return SGX_SUCCESS;
}

/*Creates a public key pair and saves it in the struct secretData keys*/
sgx_status_t createKeys() {

  sgx_ecc_state_handle_t eccHandle;

  status = sgx_ecc256_open_context(&eccHandle);
  if (status != SGX_SUCCESS) {
      return status;
  }
  status = sgx_ecc256_create_key_pair(&keys.privateKey, &keys.publicKey, eccHandle);
  if (status != SGX_SUCCESS) {
      return status;
  }
  status = sgx_ecc256_close_context(eccHandle);
  if (status != SGX_SUCCESS) {
      return status;
  }
  return SGX_SUCCESS;
}

/*Signs data*/
sgx_status_t sign(uint8_t *data, uint32_t dataSize, uint8_t *signature) {

  sgx_ecc_state_handle_t eccHandle;

  status = sgx_ecc256_open_context(&eccHandle);
  if (status != SGX_SUCCESS) {
      return status;
  }
  status = sgx_ecdsa_sign(data, dataSize, &keys.privateKey, (sgx_ec256_signature_t *) signature, eccHandle);
  if (status != SGX_SUCCESS) {
      return status;
  }
  status = sgx_ecc256_close_context(eccHandle);
  if (status != SGX_SUCCESS) {
      return status;
  }
  return SGX_SUCCESS;
}

/*Verifys data*/
sgx_status_t verify(uint8_t *data, uint32_t dataSize, uint8_t *signature, uint8_t *result) {

  sgx_ecc_state_handle_t eccHandle;

  status = sgx_ecc256_open_context(&eccHandle);
  if (status != SGX_SUCCESS) {
      return status;
  }
  status = sgx_ecdsa_verify(data, dataSize, &keys.publicKey, (sgx_ec256_signature_t *) signature, result, eccHandle);
  if (status != SGX_SUCCESS) {
      return status;
  }
  status = sgx_ecc256_close_context(eccHandle);
  if (status != SGX_SUCCESS) {
      return status;
  }
  if (*result == SGX_EC_INVALID_SIGNATURE) {
      return SGX_ERROR_UNEXPECTED;
  }
  return SGX_SUCCESS;
}
