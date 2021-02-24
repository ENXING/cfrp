#include <stdlib.h>

#include "cfrp.h"
#include "list.h"
#include "logger.h"

extern char **environ;

int main(int argc, char **argv) {

  log_level() = LOGGER_INFO;
  cfrp_mapping_t head;
  cfrp_mapping_t mp  = {.addr = "127.0.0.1", .port = 8083};
  cfrp_mapping_t mp2 = {.addr = "127.0.0.1", .port = 8084};
  INIT_LIST_HEAD(&head.list);
  list_add(&mp.list, &head.list);
  list_add(&mp2.list, &head.list);
  cfrps_t *frps = make_cfrps("127.0.0.1", 8080, &head, argc, argv);
  if (!frps) {
    log_error("make cfrp error");
    exit(0);
  }
  cfrp_start(frps);
  return 0;
}