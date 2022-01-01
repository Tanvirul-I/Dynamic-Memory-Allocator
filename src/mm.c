#include <string.h>
#include <stdio.h>
#include <unistd.h>

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

void createFreeList();
void validateHeap();

static void* freeList[13];

#define CHUNK_SIZE (1<<12)

extern void *bulk_alloc(size_t size);

extern void bulk_free(void *ptr, size_t size);

static inline __attribute__((unused)) int block_index(size_t x) {
    if (x <= 8) {
        return 5;
    } else {
        return 32 - __builtin_clz((unsigned int)x + 7);
    }
}

void *malloc(size_t size) {
    if(size <= 0) return NULL;
    
    int tailPosition = block_index(size);
    
    if(tailPosition > 12) {
        void* newPtr = bulk_alloc(size + 8);
        *(size_t*) newPtr = size + 8;
        newPtr = newPtr + sizeof(size_t);
        return newPtr;
    }
    
    if(freeList[tailPosition] == NULL) {
        createFreeList(size);
    }

    void *headPointer = freeList[tailPosition] + sizeof(size_t);
    freeList[tailPosition] = *(void**)(headPointer);
    return headPointer;
}

void *calloc(size_t nmemb, size_t size) {
    if(size <= 0) return NULL;
    
    int tailPosition = block_index(size * nmemb);
    
    if(tailPosition > 12) {
        void *ptr = bulk_alloc(nmemb * size + 8);
        *(size_t*) ptr = nmemb * size + 8;
        ptr = ptr + sizeof(size_t);
        memset(ptr, 0, nmemb * size);
        return ptr;
    }
    
    if(freeList[tailPosition] == NULL) {
        createFreeList(nmemb * size);
    }
    
    void *headPointer = freeList[tailPosition] + sizeof(size_t);
    freeList[tailPosition] = *(void**)(headPointer);
    memset(headPointer, 0, nmemb * size);
    return headPointer;
}

void *realloc(void *ptr, size_t size) {
    if(size <= 0) return NULL;
    
    if(!ptr) return malloc(size);
    
    if(*(size_t*)(ptr - sizeof(size_t)) - 8 > size) return ptr;
    else {
        size_t copyAmount = *(size_t*)(ptr - sizeof(size_t));
        int tailPosition = block_index(size);
        
        if(tailPosition > 12) {
            void *newPtr = bulk_alloc(size + 8);
            *(size_t*) newPtr = size + 8;
            newPtr = newPtr + sizeof(size_t);
            memcpy(newPtr,ptr,copyAmount);
            return newPtr;
        }
        
        if(freeList[tailPosition] == NULL) {
            createFreeList(size);
        }
        
        void *headPointer = freeList[tailPosition] + sizeof(size_t);
        freeList[tailPosition] = *(void**)(headPointer);
        memcpy(headPointer,ptr,*(size_t*)(ptr - sizeof(size_t)) - 8);
        free(ptr);
        return headPointer;
    }
}

void free(void *ptr) {
    if(!ptr) return;

    size_t blockSize = *(size_t*)(ptr - sizeof(size_t));
    int blockIndex = block_index(blockSize) - 1;
    
    if(blockIndex > 12) bulk_free(ptr - sizeof(size_t), blockSize);
    else {
        *(void**) ptr = freeList[blockIndex];
        freeList[blockIndex] = ptr - sizeof(size_t);
    }
}

void buildNode(int tailPosition, void *brkAddress, int endValue) {
    size_t sizeBytes = 1 << tailPosition;
    *(size_t*) brkAddress = sizeBytes;
    brkAddress = brkAddress + sizeof(sizeBytes);

    if(endValue == 0) *(void**) brkAddress = brkAddress + (sizeBytes - sizeof(sizeBytes));
    else *(void**) brkAddress = NULL;
}

void createFreeList(size_t size) {
    int tailPosition = block_index(size);
    void *brkAddress;
    if(!freeList[tailPosition]) {
        brkAddress= sbrk(CHUNK_SIZE);
    }

    if(!freeList[tailPosition]) {
        freeList[tailPosition] = brkAddress;
        for(int i = 0; i < CHUNK_SIZE / (1 << tailPosition); i++) {
            if(i != CHUNK_SIZE / (1 << tailPosition) - 1) buildNode(tailPosition, brkAddress, 0);
            else buildNode(tailPosition, brkAddress, 1);
            brkAddress = brkAddress + (1 << tailPosition);
        }
    }
}
