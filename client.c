#include "cfrp.h"
#include "logger.h"

int main(int argc, char **argv)
{
    log_level() = LOGGER_DEBUG;
    cfrp_start(NULL);
    return 0;
}