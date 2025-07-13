#include "../include/shm_common.h"

int create_shared_memory(const char *name, size_t size) {
    // Create shared memory object
    int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open failed");
        return -1;
    }
    
    // Set the size of shared memory
    if (ftruncate(fd, size) == -1) {
        perror("ftruncate failed");
        close(fd);
        shm_unlink(name);
        return -1;
    }
    
    return fd;
}

void* map_shared_memory(int fd, size_t size) {
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap failed");
        return NULL;
    }
    return ptr;
}

int cleanup_shared_memory(const char *name) {
    return shm_unlink(name);
}

sem_t* create_semaphore(const char *name, int value) {
    sem_t *sem = sem_open(name, O_CREAT, 0666, value);
    if (sem == SEM_FAILED) {
        perror("sem_open failed");
        return NULL;
    }
    return sem;
}

int cleanup_semaphore(const char *name) {
    return sem_unlink(name);
}