#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

// Test name
char *testName;

/* Test output file */
#define TESTPF "test_pagefile.bin"

/* Prototypes for test functions */
static void testCreateOpenClose(void);
static void testSinglePageContent(void);
static void testReadWriteMultiplePages(void);
static void testAppendEmptyBlock(void);
static void testEnsureCapacity(void);

/* Main function running all tests */
int main(void) {
    testName = "";

    initStorageManager();

    testCreateOpenClose();
    testSinglePageContent();
    testReadWriteMultiplePages();  // Added test
    testAppendEmptyBlock();        // Added test
    testEnsureCapacity();          // Added test

    return 0;
}

/* Test create, open, and close page file */
void testCreateOpenClose(void) {
    SM_FileHandle fh;

    testName = "test create open and close methods";

    TEST_CHECK(createPageFile(TESTPF));
    TEST_CHECK(openPageFile(TESTPF, &fh));
    ASSERT_TRUE(strcmp(fh.fileName, TESTPF) == 0, "filename correct");
    ASSERT_TRUE(fh.totalNumPages == 1, "expect 1 page in new file");
    ASSERT_TRUE(fh.curPagePos == 0, "freshly opened file's page position should be 0");

    TEST_CHECK(closePageFile(&fh));
    TEST_CHECK(destroyPageFile(TESTPF));
    ASSERT_TRUE(openPageFile(TESTPF, &fh) != RC_OK, "opening non-existing file should return an error.");

    TEST_DONE();
}

/* Test single page content */
void testSinglePageContent(void) {
    SM_FileHandle fh;
    SM_PageHandle ph = (SM_PageHandle)malloc(PAGE_SIZE);

    testName = "test single page content";

    TEST_CHECK(createPageFile(TESTPF));
    TEST_CHECK(openPageFile(TESTPF, &fh));

    TEST_CHECK(readFirstBlock(&fh, ph));
    for (int i = 0; i < PAGE_SIZE; i++)
        ASSERT_TRUE(ph[i] == 0, "expected zero byte in first page of freshly initialized page");

    for (int i = 0; i < PAGE_SIZE; i++)
        ph[i] = (i % 10) + '0';
    TEST_CHECK(writeBlock(0, &fh, ph));

    TEST_CHECK(readFirstBlock(&fh, ph));
    for (int i = 0; i < PAGE_SIZE; i++)
        ASSERT_TRUE(ph[i] == (i % 10) + '0', "character in page read from disk is the one we expected.");

    TEST_CHECK(destroyPageFile(TESTPF));  
    free(ph);

    TEST_DONE();
}

void testReadWriteMultiplePages(void) {
    //This test function creates a file, ensures it has 3 pages, 
    //writes and then reads back data to verify content consistency across multiple pages.

    SM_FileHandle fh;
    SM_PageHandle ph = (SM_PageHandle)malloc(PAGE_SIZE);  

    testName = "test read and write multiple pages";

    TEST_CHECK(createPageFile(TESTPF));
    TEST_CHECK(openPageFile(TESTPF, &fh));

    // Assuming ensureCapacity is fixed and works as expected
    TEST_CHECK(ensureCapacity(3, &fh));

    for (int i = 0; i < 3; i++) {
        memset(ph, 0, PAGE_SIZE); // Clear page buffer
        for (int j = 0; j < PAGE_SIZE; j++) {
            ((unsigned char*)ph)[j] = (unsigned char)((i * j) % 256); // Cast to unsigned char
        }
        TEST_CHECK(writeBlock(i, &fh, ph));

        // Re-read the block to verify content
        memset(ph, 0, PAGE_SIZE); // Clear buffer before reading back
        TEST_CHECK(readBlock(i, &fh, ph));
        for (int j = 0; j < PAGE_SIZE; j++) {
            if (((unsigned char*)ph)[j] != (unsigned char)((i * j) % 256)) {  // Cast to unsigned char for comparison
                printf("Mismatch at page %d, byte %d: expected %u, got %u\n",
                       i, j, (unsigned char)((i * j) % 256), ((unsigned char*)ph)[j]);
                ASSERT_TRUE(0, "mismatch in page content immediately after write");
            }
        }
    }

    TEST_CHECK(closePageFile(&fh));
    TEST_CHECK(destroyPageFile(TESTPF));
    free(ph);

    TEST_DONE();
}





/* Test append empty block */
void testAppendEmptyBlock(void) {
    //This test function verifies appending an empty block to a file, 
    //checking the updated total pages, and ensuring the new block is empty.

    SM_FileHandle fh;
    SM_PageHandle ph = (SM_PageHandle )malloc(PAGE_SIZE);

    testName = " test append empty block";

    TEST_CHECK(createPageFile(TESTPF));
    TEST_CHECK(openPageFile(TESTPF, &fh));

    TEST_CHECK( appendEmptyBlock(&fh)) ;
    ASSERT_TRUE(fh.totalNumPages == 2, "total number of pages should be 2 after append");

    
    TEST_CHECK(readBlock(1, &fh, ph)) ;
    for (int i = 0; i < PAGE_SIZE; i++ )
        ASSERT_TRUE(ph[i] == 0, "expected empty byte in appended block");

    TEST_CHECK(closePageFile(&fh));
    TEST_CHECK(destroyPageFile(TESTPF));
    free(ph);

    TEST_DONE();
}

/* Test ensure capacity */
void testEnsureCapacity(void) {
    //This test verifies the ensureCapacity function by creating a new file,
    // expanding it to 5 pages, and confirming the updated page count.

    SM_FileHandle fh;

    testName = "test ensure capacity";

    
    TEST_CHECK(createPageFile(TESTPF));

    TEST_CHECK(openPageFile(TESTPF, &fh));

    ASSERT_TRUE(fh.totalNumPages == 1, "New file should start with 1 page");
    TEST_CHECK(ensureCapacity(5, &fh));

   

    ASSERT_TRUE(fh.totalNumPages == 5, "total number of pages should be 5 after ensuring capacity");

    
    TEST_CHECK(closePageFile(&fh));
    TEST_CHECK(destroyPageFile(TESTPF));

    TEST_DONE();
}

