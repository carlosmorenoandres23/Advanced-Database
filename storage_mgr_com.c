#include "storage_mgr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void initStorageManager (void){
    printf("Storage Manager Initialized\n");

}

RC createPageFile(char *newFileName) {
    
    //This function creates a new file, initializes it with a blank page using dynamically allocated memory
    // and handles errors if file creation fails.
    FILE *filePointer;
    filePointer = fopen(newFileName, "w");
    if (filePointer == NULL) {

        return RC_FILE_NOT_FOUND;

    } else {
        char *emptyPage = (char *)malloc(PAGE_SIZE);
        memset(emptyPage, '\0', PAGE_SIZE);
        fwrite(emptyPage, PAGE_SIZE, 1, filePointer);
        fclose(filePointer);
        free(emptyPage);

        return RC_OK;
    }
}

RC openPageFile(char *newFileName, SM_FileHandle *fileHandle) {
    
    //This function opens an existing file, updates the file handle, calculates total pages,
    // calculates total pages, and returns success or file not found error.

    FILE *filePointer;
    filePointer = fopen(newFileName, "r+");
   
    if (filePointer == NULL) {    
        return RC_FILE_NOT_FOUND;
    
    } else {
        
        fileHandle->fileName = newFileName;
        fseek(filePointer, 0, SEEK_END);
        fileHandle->totalNumPages = ftell(filePointer) / PAGE_SIZE;
        fileHandle->curPagePos = 0;
        fileHandle->mgmtInfo = filePointer;

        return RC_OK;
    }
}

RC closePageFile(SM_FileHandle *fileHandle) {

    //This function closes a page file by accessing the file pointer from the file handle
    // and returns success or a file not found error.

    FILE *filePointer;
    filePointer = fileHandle->mgmtInfo;
    if (filePointer == NULL) {
       
        return RC_FILE_NOT_FOUND;
    } else {
        fclose(filePointer);
        
        return RC_OK;
    }
}

RC destroyPageFile(char *newFileName) {

    //This function tries to delete a specified page file
    // and returns success if successful, or a file not found error otherwise.



    if (remove(newFileName) == 0) {
        return RC_OK;
    } else {   
        return RC_FILE_NOT_FOUND;
    }
}

//READ BLOCKS

RC readBlock(int pageNumber, SM_FileHandle *fileHandle, SM_PageHandle pageData) {
    //This function reads a block from a file at a given page number, 
    //updates file handle's current position, and handles potential errors.

    FILE *filePointer;
 
    filePointer = fileHandle->mgmtInfo;
    if (filePointer == NULL) {
    
        return RC_FILE_NOT_FOUND;
    } else {
        if (pageNumber > fileHandle->totalNumPages) {
            
            return RC_READ_NON_EXISTING_PAGE;
        } else {
           
            fseek(filePointer, pageNumber * PAGE_SIZE, SEEK_SET);
            fread(pageData, PAGE_SIZE, 1, filePointer);
            fileHandle->curPagePos = pageNumber;
            
            return RC_OK;
        }
    }
}

int getBlockPos(SM_FileHandle *fileHandle) {
    
    // Return the current page position.

    return fileHandle->curPagePos;
}

RC readFirstBlock(SM_FileHandle *fileHandle, SM_PageHandle pageData) {
  
    // Reads the first block of a file, updates current page position in file handle, and handles file not found errors.
    FILE *filePointer;
    filePointer = fileHandle->mgmtInfo;
    if (filePointer == NULL) {
        
        return RC_FILE_NOT_FOUND;

    } else {
        fseek(filePointer, 0, SEEK_SET);
        fread(pageData, PAGE_SIZE, 1, filePointer);
        fileHandle->curPagePos = 0;
        
        return RC_OK;
    }
}

RC readPreviousBlock(SM_FileHandle *fileHandle, SM_PageHandle pageData) {
   //Reads the previous block in a file, updates the current position,
   // and manages errors for start of file or file not found.


    FILE *filePointer;
    filePointer = fileHandle->mgmtInfo;
    if (filePointer == NULL) {
       
        return RC_FILE_NOT_FOUND;
    } else {
        if (fileHandle->curPagePos == 0) {
            return RC_READ_NON_EXISTING_PAGE;
        } else {
            
            fseek(filePointer, (fileHandle->curPagePos - 1) * PAGE_SIZE, SEEK_SET);
            fread(pageData, PAGE_SIZE, 1, filePointer);
            fileHandle->curPagePos = fileHandle->curPagePos - 1;
           
           
            return RC_OK;
        }
    }
}

RC readCurrentBlock(SM_FileHandle *fileHandle, SM_PageHandle pageData) {
    
    //Reads the current block from a file based on the file handle's position and 
    //handles cases where the file is not found.
    
    
    FILE *filePointer;
    filePointer = fileHandle->mgmtInfo;
    if (filePointer == NULL) {
        return RC_FILE_NOT_FOUND;
    } else {
        
        fseek(filePointer, fileHandle->curPagePos * PAGE_SIZE, SEEK_SET);
        fread(pageData, PAGE_SIZE, 1, filePointer);
        return RC_OK;
    }
}

RC readNextBlock(SM_FileHandle *fileHandle, SM_PageHandle pageData) {
    //Reads the next block in a file, updates the file handle's current position, and manages errors for end of file or file not found.
    //
   
   
    FILE *filePointer;
    
    filePointer = fileHandle->mgmtInfo;
    if (filePointer == NULL) {
        return RC_FILE_NOT_FOUND;
    } else {
       
        if (fileHandle->curPagePos == fileHandle->totalNumPages) {
            return RC_READ_NON_EXISTING_PAGE;
        } else {
            
            fseek(filePointer, (fileHandle->curPagePos + 1) * PAGE_SIZE, SEEK_SET);
            fread(pageData, PAGE_SIZE, 1, filePointer);
            fileHandle->curPagePos = fileHandle->curPagePos + 1;
          
            return RC_OK;
        }
    }
}

RC readLastBlock(SM_FileHandle *fileHandle, SM_PageHandle pageData) {
    
    //Reads the last block from a file, updates the file handle's current page position
    //, and handles cases where the file is not found.


    FILE *filePointer;
    filePointer = fileHandle->mgmtInfo;
    if ( filePointer == NULL) {
        return RC_FILE_NOT_FOUND;
    } else {
        fseek(filePointer, fileHandle->totalNumPages * PAGE_SIZE, SEEK_SET);
        fread(pageData, PAGE_SIZE, 1, filePointer  );
        fileHandle->curPagePos = fileHandle->totalNumPages;
      
      
        return RC_OK;
    }
}

//WRITE BLOCKS

RC writeBlock(int pageNumber, SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    //Writes a block to a specified page in a file, handles append and error scenarios,
    // and updates the file handle's current page position.
    
    if (fileHandle  == NULL ||  fileHandle->mgmtInfo == NULL ) {
        return RC_FILE_HANDLE_NOT_INIT;
    }

    FILE *filePointer = ( FILE *)fileHandle->mgmtInfo;

   
    if (pageNumber == fileHandle->totalNumPages) {
        appendEmptyBlock(fileHandle); 
    } else if (pageNumber > fileHandle->totalNumPages) {
        return RC_WRITE_FAILED;
    }

    fseek(filePointer, pageNumber * PAGE_SIZE, SEEK_SET);
    size_t written = fwrite(memPage, 1, PAGE_SIZE, filePointer);
    if (written < PAGE_SIZE) {
        return RC_WRITE_FAILED;
    }

    fileHandle->curPagePos = pageNumber;
    return RC_OK;
}


RC writeCurrentBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    //write the current bloxk using the function writeNlock

    if (fileHandle == NULL || fileHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }

   
    return writeBlock(fileHandle->curPagePos, fileHandle, memPage);
}


RC appendEmptyBlock(SM_FileHandle *fileHandle) {
    // Create a FILE pointer variable to represent the file using the functions previously defines
    //as menset, fwrtie and free. 
    FILE *filePointer;
    
    filePointer = fileHandle->mgmtInfo;
    
    if (filePointer == NULL ) {
        
        return RC_FILE_NOT_FOUND;
    } else {
        
        char *emptyPage = (char *)malloc(PAGE_SIZE);
        memset(emptyPage, '\0', PAGE_SIZE);

        fwrite(emptyPage, PAGE_SIZE, 1, filePointer);
        fileHandle->totalNumPages = fileHandle->totalNumPages + 1;

        fileHandle->curPagePos = fileHandle->totalNumPages;
    
        free( emptyPage);
        return RC_OK;
    }
}

RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
    
    //Expands a file to a specified number of pages, 
    //initializes new pages with zeros, and updates file handle's total page count.
    //we also take the advantaje of using previous defined functions
    
    if (fHandle == NULL || fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }

    FILE *filePointer = (FILE *)fHandle->mgmtInfo;
    int currentPageCount = fHandle->totalNumPages;

    if (numberOfPages <= currentPageCount) {
        return RC_OK; 
    }

    fseek(filePointer, 0, SEEK_END); 

    char *emptyPage = (char *)calloc(PAGE_SIZE, sizeof(char)); 
    if (emptyPage == NULL) {
        return RC_WRITE_FAILED;
    }

    for (int i = currentPageCount; i < numberOfPages; i++) {
        size_t written = fwrite(emptyPage, sizeof(char), PAGE_SIZE, filePointer);
        if (written < PAGE_SIZE) {
            free(emptyPage);
            return RC_WRITE_FAILED;
        }
        fHandle->totalNumPages++; 
    }

    free(emptyPage);
    return RC_OK;
}
