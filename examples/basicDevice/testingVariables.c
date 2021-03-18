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
static ODR_t OD_read_testVar(OD_stream_t *stream, void *buf,
                             OD_size_t count, OD_size_t *countRead)
{
    if (stream == NULL || buf == NULL || countRead == NULL) {
        return ODR_DEV_INCOMPAT;
    }

    /* Object was passed by OD_extensionIO_init, use correct type. */
    testingVariables_t *testVar = (testingVariables_t *)stream->object;

    switch (stream->subIndex) {
        case SUBINDEX_AVERAGE: {
            OD_size_t varSize = sizeof(float64_t);

            if (count < varSize || stream->dataLength != varSize) {
                return ODR_DEV_INCOMPAT;
            }

            float64_t average = (float64_t)*testVar->i64;
            average += (float64_t)*testVar->u64;
            average += (float64_t)*testVar->r32;
            average += *testVar->r64;
            average /= 4;

            memcpy(buf, &average, varSize);

            *countRead = varSize;
            return ODR_OK;
        }

        case SUBINDEX_PARAMETER: {
            OD_size_t varSize = sizeof(testVar->parameterU16);

            if (count < varSize || stream->dataLength != varSize) {
                return ODR_DEV_INCOMPAT;
            }

            CO_setUint16(buf, ++testVar->parameterU16);

            *countRead = varSize;
            return ODR_OK;
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
                    return ODR_NO_DATA;
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
            ODR_t returnCode;

            /* determine, if file read finished or not */
            if (len != count || feof(testVar->domainReadFileStream)) {
                fclose(testVar->domainReadFileStream);
                testVar->domainReadFileStream = 0;
                returnCode = ODR_OK;
                stream->dataOffset = 0;
            }
            else {
                returnCode = ODR_PARTIAL;
                stream->dataOffset += len;
            }

            /* indicate number of bytes read and return ODR_OK or ODR_PARTIAL */
            *countRead = len;
            return returnCode;
        }

        default: {
            return OD_readOriginal(stream, buf, count, countRead);
        }
    }
}


/*
 * Custom function for reading OD object _Testing variables_
 *
 * For more information see file CO_ODinterface.h, OD_IO_t.
 */
static ODR_t OD_write_testVar(OD_stream_t *stream, const void *buf,
                              OD_size_t count, OD_size_t *countWritten)
{
    if (stream == NULL || buf == NULL || countWritten == NULL) {
        return ODR_DEV_INCOMPAT;
    }

    /* Object was passed by OD_extensionIO_init, use correct type. */
    testingVariables_t *testVar = (testingVariables_t *)stream->object;

    switch (stream->subIndex) {
        case SUBINDEX_PARAMETER: {
            testVar->parameterU16 = CO_getUint16(buf);
            /* write value to the original location in the Object Dictionary */
            return OD_writeOriginal(stream, buf, count, countWritten);
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
                    return ODR_OUT_OF_MEM;
                }
            }

            /* write the data and verify */
            size_t len = fwrite(buf, 1, count, testVar->domainWriteFileStream);
            if (testVar->domainWriteFileStream == NULL) {
                return ODR_GENERAL;
            }

            /* indicate total length written */
            stream->dataOffset += len;

            /* determine, if file write finished or not
             * (dataLength may not yet be indicated) */
            ODR_t returnCode = ODR_OK;
            if (stream->dataLength > 0
                && stream->dataOffset == stream->dataLength
            ) {
                fclose(testVar->domainWriteFileStream);
                testVar->domainWriteFileStream = 0;
                stream->dataOffset = 0;

                /* replace original file with just downloaded */
                char *fileNameOrig = testVar->domainWriteFileName;
                char *fileName = malloc(strlen(fileNameOrig) + 6);
                strcpy(fileName, fileNameOrig);
                strcat(fileName, ".tmp");
                int err = rename(fileName, fileNameOrig);
                free(fileName);

                if (err != 0) {
                    return ODR_GENERAL;
                }
            }
            else {
                returnCode = ODR_PARTIAL;
            }

            /* indicate number of bytes written and return ODR_OK or
             * ODR_PARTIAL */
            *countWritten = len;
            return returnCode;
        }

        default: {
            return OD_writeOriginal(stream, buf, count, countWritten);
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
    ODR_t odRet;

    /* initialize object variables */
    memset(testVar, 0, sizeof(testingVariables_t));

    /* Initialize custom OD object "Testing variables" */
    testVar->OD_testVar_extension.object = testVar;
    testVar->OD_testVar_extension.read = OD_read_testVar;
    testVar->OD_testVar_extension.write = OD_write_testVar;
    odRet = OD_extension_init(OD_testVar, &testVar->OD_testVar_extension);

    /* This is strict behavior and will exit the program on error. Error
     * checking on all OD functions can also be omitted. In that case program
     * will run, but specific OD entry may not be accessible. */
    if (odRet != ODR_OK) {
        *errInfo = OD_getIndex(OD_testVar);
        return CO_ERROR_OD_PARAMETERS;
    }

    /* Get variables from Object dictionary, related to "Average" */
    testVar->i64 = OD_getPtr(OD_testVar, SUBINDEX_I64, sizeof(int64_t), NULL);
    testVar->u64 = OD_getPtr(OD_testVar, SUBINDEX_U64, sizeof(uint64_t), NULL);
    testVar->r32 = OD_getPtr(OD_testVar, SUBINDEX_R32, sizeof(float32_t), NULL);
    testVar->r64 = OD_getPtr(OD_testVar, SUBINDEX_R64, sizeof(float64_t), NULL);

    if (testVar->i64 == NULL || testVar->u64 == NULL
        || testVar->r32 == NULL || testVar->r64 == NULL
    ) {
        *errInfo = OD_getIndex(OD_testVar);
        return CO_ERROR_OD_PARAMETERS;
    }

    /* Get variable 'Parameter with default value' from Object dictionary */
    odRet = OD_get_u16(OD_testVar, SUBINDEX_PARAMETER,
                      &testVar->parameterU16, true);

    if (odRet != ODR_OK) {
        *errInfo = OD_getIndex(OD_testVar);
        return CO_ERROR_OD_PARAMETERS;
    }

    /* Get variables from Object dictionary, related to "Domain" */
    testVar->domainReadFileName = OD_getPtr(OD_testVar,
                                            SUBINDEX_DOMAIN_FILE_READ,
                                            0, NULL);
    testVar->domainWriteFileName = OD_getPtr(OD_testVar,
                                             SUBINDEX_DOMAIN_FILE_WRITE,
                                             0, NULL);

    if (testVar->domainReadFileName == NULL
        || testVar->domainWriteFileName == NULL
    ) {
        *errInfo = OD_getIndex(OD_testVar);
        return CO_ERROR_OD_PARAMETERS;
    }

    return err;
}
