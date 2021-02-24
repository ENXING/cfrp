#include "cshm.h"
#include "lib.h"
#include "logger.h"
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct {
  int shm_id;
  size_t total;
  size_t use;
  void *data_ptr;
} cfrp_shm_entry;

struct cfrp_shm *make_cshm(size_t size) {
  cfrp_shm_t *shm       = (cfrp_shm_t *)cfrp_malloc(sizeof(cfrp_shm_t));
  cfrp_shm_entry *entry = (cfrp_shm_entry *)cfrp_malloc(sizeof(cfrp_shm_entry));
  __non_null__(shm, NULL);
  __non_null__(entry, NULL);
  int shm_id     = 0;
  void *data_ptr = (void *)-1;
  if ((shm_id = shmget(IPC_PRIVATE, size, IPC_CREAT | SHM_EXEC | 0777)) < 0 || (data_ptr = shmat(shm_id, NULL, 0)) == (void *)-1) {
    shm_id != -1 && shmctl(shm_id, IPC_RMID, NULL);
    cfrp_free(entry);
    cfrp_free(shm);
    return NULL;
  }
  cfrp_memzero(data_ptr, size);
  entry->shm_id   = shm_id;
  entry->data_ptr = data_ptr;
  entry->total    = size;
  entry->use      = 0;
  shm->entry      = entry;
  shm->size       = entry->total;
  log_debug("share memory size: %ld", size);
  return shm;
}

void *cshm_alloc(struct cfrp_shm *shm, size_t size) {
  __non_null__(shm, NULL);
  __non_null__(shm->entry, NULL);
  cfrp_shm_entry *entry = (cfrp_shm_entry *)shm->entry;
  if (entry->total < entry->use + size) {
    log_error("out of shared memory");
    return NULL;
  }
  void *ptr = entry->data_ptr + entry->use;
  entry->use += size;
  return ptr;
}

void cshm_release(struct cfrp_shm *shm) {
  __non_null__(shm, ;);
  __non_null__(shm->entry, ;);
  cfrp_shm_entry *entry = (cfrp_shm_entry *)shm->entry;
  shmctl(entry->shm_id, IPC_RMID, NULL);
  cfrp_free(shm->entry);
  cfrp_free(shm);
}