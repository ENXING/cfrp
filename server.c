#include <stdlib.h>

#include "cfrp.h"
#include "logger.h"
#include "list.h"

int main(int argc, char **argv)
{
    log_level() = LOGGER_DEBUG;
    struct list_head head;
    mapping_t mp = {
        .addr = "127.0.0.1",
        .port = 8080};
    INIT_LIST_HEAD(&head);
    list_add(&mp.list, &head);
    cfrps *frps = make_cfrps("127.0.0.1", 8080, NULL);
    if (!frps)
    {
        log_error("make cfrp error");
        exit(0);
    }
    cfrp_start(frps);
    return 0;
}