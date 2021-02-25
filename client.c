#include "cfrp.h"
#include "lib.h"
#include "logger.h"

int main(int argc, char **argv) {
  log_level() = LOGGER_INFO;
  if (argc < 3) {
    log_error("parameter is not enough. %s [host] [port]", argv[0]);
    return 0;
  }
  cfrpc_t *frpc = make_cfrpc(argv[1], cfrp_atoi(argv[2]), NULL, argc, argv);
  if (!frpc) {
    log_error("make cfrp error");
    return 0;
  }
  cfrp_start(frpc);
  return 0;
}