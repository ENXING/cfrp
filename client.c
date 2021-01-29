/*
 * @Author: your name
 * @Date: 1970-01-01 08:00:00
 * @LastEditTime: 2021-01-29 13:21:30
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /cfrpx/client.c
 */

#include "cfrp.h"

int main(int argc, char **argv)
{
    log_level() = WARNING;
    log_info("info", "info");
    log_debug("debug", "debug");
    log_warning("warning", "warning");
    log_error("error", "error");
    return 0;
}