/* Application specific access functions for extended OD objects */


#include "testingVariables.h"
#include <string.h>
#include <stdlib.h>


#define SUBINDEX_I64 0x01
#define SUBINDEX_U64 0x02
#define SUBINDEX_R32 0x03
#define SUBINDEX_R64 0x04
#define SUBINDEX_AVERAGE 0x05
#define SUBINDEX_PARAMETER 0x09
#define SUBINDEX_DOMAIN 0x0A
#define SUBINDEX_DOMAIN_FILE_READ 0x0B
#define SUBINDEX_DOMAIN_FILE_WRITE 0x0C

#ifndef DOMAIN_LENGTH_INDICATE
#define DOMAIN_LENGTH_INDICATE 1
#endif


/*
 * Custom function for reading OD object _Testing variables_
 *
 * For more information see file CO_ODinterface.h, OD_IO_t.
 */
static OD_size_t OD_read_testVar(OD_stream_t *stream, uint8_t subIndex,
                                 void *buf, OD_size_t count,
                                 ODR_t *returnCode)
{
    if (stream == NULL || buf == NULL || returnCode == NULL) {
        if (returnCode != NULL) *returnCode = ODR_DEV_INCOMPAT;
        return 0;
    }

    /* Object was passed by OD_extensionIO_init, use correct type. */
    testingVariables_t *testVar = (testingVariables_t *)stream->object;
    *returnCode = ODR_OK;

    switch (subIndex) {
        case SUBINDEX_AVERAGE: {
            OD_size_t varSize = sizeof(float64_t);

            if (count >= varSize && stream->dataLength == varSize) {
                float64_t average = (float64_t)*testVar->i64;
                average += (float64_t)*testVar->u64;
                average += (float64_t)*testVar->r32;
                average += *testVar->r64;
                average /= 4;

                CO_LOCK_OD();
                memcpy(buf, &average, varSize);
                CO_UNLOCK_OD();
                return varSize;
            }
            else {
                *returnCode = ODR_DEV_INCOMPAT;
                return 0;
            }
        }

        case SUBINDEX_PARAMETER: {
            OD_size_t varSize = sizeof(testVar->parameterU16);

            if (count >= varSize && stream->dataLength == varSize) {
                CO_LOCK_OD();
                CO_setUint16(buf, ++testVar->parameterU16);
                CO_UNLOCK_OD();
                return varSize;
            }
            else {
                *returnCode = ODR_DEV_INCOMPAT;
                return 0;
            }
        }

        case SUBINDEX_DOMAIN: {
            /* Data is read from the file for this example. Data can be much
             * longer than count (available size of the buffer). So
             * OD_read_testVar() may be called multiple times. */
            if (stream->dataOffset == 0) {
                /* Data offset is 0, so this is the first call of this function
                 * in current SDO communication. Open the file now. But if it
                 * is already open, then previous SDO communication didn't
                 * finish correctly. So close the old file first. */
                if (testVar->domainReadFileStream != NULL)
                    fclose(testVar->domainReadFileStream);

                /* Get filename from auxiliary OD variable of type
                 * VISIBLE_STRING. This type of variable may have variable
                 * string length and always have null terminating character. */
                char *fileName = testVar->domainReadFileName;

                /* open the file and verify success */
                testVar->domainReadFileStream = fopen(fileName, "r");
                if (testVar->domainReadFileStream == NULL) {
                    *returnCode = ODR_NO_DATA;
                    return 0;
                }
#if DOMAIN_LENGTH_INDICATE
                /* Indicate dataLength */
                fseek(testVar->domainReadFileStream, 0L, SEEK_END);
                size_t dataLen = (size_t)ftell(testVar->domainReadFileStream);
                stream->dataLength = dataLen <= 0xFFFFFFFF
                                   ? dataLen : 0;
                rewind(testVar->domainReadFileStream);
#else
                /* It is not required to indicate data length in SDO transfer */
                stream->dataLength = 0;
#endif
            }

            /* fill the buffer */
            size_t len = fread(buf, 1, count, testVar->domainReadFileStream);

            /* determine, if file read finished or not */
            if (len != count || feof(testVar->domainReadFileStream)) {
                fclose(testVar->domainReadFileStream);
                testVar->domainReadFileStream = 0;
                *returnCode = ODR_OK;
                stream->dataOffset = 0;
            }
            else {
                *returnCode = ODR_PARTIAL;
                stream->dataOffset += len;
            }

            /* return number of bytes read */
            return len;
        }

        default: {
            return OD_readOriginal(stream, subIndex, buf, count, returnCode);
        }
    }
}


/*
 * Custom function for reading OD object _Testing variables_
 *
 * For more information see file CO_ODinterface.h, OD_IO_t.
 */
static OD_size_t OD_write_testVar(OD_stream_t *stream, uint8_t subIndex,
                                  const void *buf, OD_size_t count,
                                  ODR_t *returnCode)
{
    if (stream == NULL || buf == NULL || returnCode == NULL) {
        if (returnCode != NULL) *returnCode = ODR_DEV_INCOMPAT;
        return 0;
    }

    /* Object was passed by OD_extensionIO_init, use correct type. */
    testingVariables_t *testVar = (testingVariables_t *)stream->object;
    *returnCode = ODR_OK;

    switch (subIndex) {
        case SUBINDEX_PARAMETER: {
            CO_LOCK_OD();
            testVar->parameterU16 = CO_getUint16(buf);
            CO_UNLOCK_OD();
            /* write value to the original location in the Object Dictionary */
            return OD_writeOriginal(stream, subIndex, buf, count, returnCode);
        }

        case SUBINDEX_DOMAIN: {
            /* Data will be written to the file for this example. Data can be
             * much longer than count (current size of data in the buffer). So
             * OD_write_testVar() may be called multiple times. */
            if (stream->dataOffset == 0) {
                /* Data offset is 0, so this is the first call of this function
                 * in current SDO communication. Open the file now. Write data
                 * to temporary file first. When transfer will be finished and
                 * temporary file will be written successfully, the original
                 * file will be replaced by with newly transferred. But if
                 * temporary file is already open in this moment, then previous
                 * SDO communication didn't finish correctly. So close the old
                 * file first. */
                if (testVar->domainWriteFileStream != NULL)
                    fclose(testVar->domainWriteFileStream);

                /* Get filename from auxiliary OD variable of type
                 * VISIBLE_STRING. This type of variable may have variable
                 * string length and always have null terminating character. */
                char *fileNameOrig = testVar->domainWriteFileName;

                /* create temporary file and verify success */
                char *fileName = malloc(strlen(fileNameOrig) + 6);
                strcpy(fileName, fileNameOrig);
                strcat(fileName, ".tmp");
                testVar->domainWriteFileStream = fopen(fileName, "w");
                free(fileName);
                if (testVar->domainWriteFileStream == NULL) {
                    *returnCode = ODR_OUT_OF_MEM;
                    return 0;
                }
            }

            /* write the data and verify */
            size_t len = fwrite(buf, 1, count, testVar->domainWriteFileStream);
            if (testVar->domainWriteFileStream == NULL) {
                *returnCode = ODR_GENERAL;
                return 0;
            }

            /* indicate total length written */
            stream->dataOffset += len;

            /* determine, if file write finished or not
             * (dataLength may not yet be indicated) */
            if (stream->dataLength > 0
                && stream->dataOffset == stream->dataLength
            ) {
                fclose(testVar->domainWriteFileStream);
                testVar->domainWriteFileStream = 0;

                /* replace original file with just downloaded */
                char *fileNameOrig = testVar->domainWriteFileName;
                char *fileName = malloc(strlen(fileNameOrig) + 6);
                strcpy(fileName, fileNameOrig);
                strcat(fileName, ".tmp");
                if (rename(fileName, fileNameOrig) == 0)
                    *returnCode = ODR_OK;
                else
                    *returnCode = ODR_GENERAL;
                free(fileName);

                stream->dataOffset = 0;
            }
            else {
                *returnCode = ODR_PARTIAL;
            }

            /* return number of bytes written */
            return len;
        }

        default: {
            return OD_writeOriginal(stream, subIndex, buf, count, returnCode);
        }
    }
}


/******************************************************************************/
CO_ReturnError_t testingVariables_init(testingVariables_t *testVar,
                                       uint32_t *errInfo,
                                       OD_entry_t *OD_testVar)
{
    if (testVar == NULL || errInfo == NULL || OD_testVar == NULL)
        return CO_ERROR_ILLEGAL_ARGUMENT;

    CO_ReturnError_t err = CO_ERROR_NO;
    ODR_t ret, ret1, ret2, ret3, ret4;
    OD_size_t len1, len2, len3, len4;

    /* initialize object variables */
    memset(testVar, 0, sizeof(testingVariables_t));

    /* Initialize custom OD object "Testing variables" */
    testVar->OD_testVar_extension.object = testVar;
    testVar->OD_testVar_extension.read = OD_read_testVar;
    testVar->OD_testVar_extension.write = OD_write_testVar;
    ret = OD_extension_init(OD_testVar, &testVar->OD_testVar_extension);

    /* This is strict behavior and will exit the program on error. Error
     * checking on all OD functions can also be omitted. In that case program
     * will run, but specific OD entry may not be accessible. */
    if (ret != ODR_OK) {
        *errInfo = OD_getIndex(OD_testVar);
        return CO_ERROR_OD_PARAMETERS;
    }

    /* Get variables from Object dictionary, related to "Average" */
    ret1 = OD_getPtr(OD_testVar, SUBINDEX_I64, (void **)&testVar->i64, &len1);
    ret2 = OD_getPtr(OD_testVar, SUBINDEX_U64, (void **)&testVar->u64, &len2);
    ret3 = OD_getPtr(OD_testVar, SUBINDEX_R32, (void **)&testVar->r32, &len3);
    ret4 = OD_getPtr(OD_testVar, SUBINDEX_R64, (void **)&testVar->r64, &len4);

    if (ret1 != ODR_OK || len1 != sizeof(int64_t)
        || ret2 != ODR_OK || len2 != sizeof(uint64_t)
        || ret3 != ODR_OK || len3 != sizeof(float32_t)
        || ret4 != ODR_OK || len4 != sizeof(float64_t)
    ) {
        *errInfo = OD_getIndex(OD_testVar);
        return CO_ERROR_OD_PARAMETERS;
    }

    /* Get variable 'Parameter with default value' from Object dictionary */
    ret1 = OD_get_u16(OD_testVar, SUBINDEX_PARAMETER,
                      &testVar->parameterU16, true);

    if (ret1 != ODR_OK) {
        *errInfo = OD_getIndex(OD_testVar);
        return CO_ERROR_OD_PARAMETERS;
    }

    /* Get variables from Object dictionary, related to "Domain" */
    ret1 = OD_getPtr(OD_testVar, SUBINDEX_DOMAIN_FILE_READ,
                     (void **)&testVar->domainReadFileName, NULL);
    ret2 = OD_getPtr(OD_testVar, SUBINDEX_DOMAIN_FILE_WRITE,
                     (void **)&testVar->domainWriteFileName, NULL);

    if (ret1 != ODR_OK || ret2 != ODR_OK) {
        *errInfo = OD_getIndex(OD_testVar);
        return CO_ERROR_OD_PARAMETERS;
    }

    return err;
}
