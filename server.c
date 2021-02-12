#include <stdlib.h>

#include "cfrp.h"

int main(int argc, char **argv)
{
    cfrps *frps = make_cfrps();
    if (!frps)
    {
        log_error("make cfrp error");
        exit(0);
    }
    
    cfrp_start(frps);
    return 0;
}