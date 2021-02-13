#include <stdlib.h>

#include "cfrp.h"
#include "logger.h"
#include "list.h"

int main(int argc, char **argv)
{
    cfrps *frps = make_cfrps("127.0.0.1", 8080, NULL);
    if (!frps)
    {
        log_error("make cfrp error");
        exit(0);
    }
    cfrp_start(frps);
    
    return 0;
}