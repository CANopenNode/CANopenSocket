/** Application specific access functions for extended OD objects */


#ifndef TESTING_VARIABLES_H
#define TESTING_VARIABLES_H


#include "301/CO_ODinterface.h"
#include <stdio.h>


/**
 * testingVariables object.
 */
typedef struct {
    int64_t *i64; /**< Pointer to variable in object dictionary */
    uint64_t *u64; /**< Pointer to variable in object dictionary */
    float32_t *r32; /**< Pointer to variable in object dictionary */
    float64_t *r64; /**< Pointer to variable in object dictionary */
    /** Variable initialised from OD sub-entry 'Parameter with default value' */
    uint16_t parameterU16;
    /** domain data type test - stream for reading the file */
    FILE *domainReadFileStream;
    /** domain data type test - filename from object dictionary */
    char *domainReadFileName;
    /** domain data type test - stream for writing the file */
    FILE *domainWriteFileStream;
    /** domain data type test - filename from object dictionary */
    char *domainWriteFileName;
} testingVariables_t;


/**
 * Initialize testingVariables object.
 *
 * @param testVar This object will be initialized.
 * @param [out] errinfo Variable may indicate additional information for some
 * types of errors.
 * @param OD_testVar Object Dictionary entry for Testing variables.
 *
 * @return @ref CO_ReturnError_t CO_ERROR_NO in case of success.
 */
CO_ReturnError_t testingVariables_init(testingVariables_t *testVar,
                                       uint16_t *errinfo,
                                       const OD_entry_t *OD_testVar);

#endif /* TESTING_VARIABLES_H */
