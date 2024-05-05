#include "errors.h"
#include <stdio.h>
#include <stdlib.h>

void fatal_error_system (const char *mesg) 
{
    perror(mesg);
    exit(1);
}

void handle_error_system (int err, const char *mesg) 
{
    if (err == -1) {
        // perror(mesg);
        // exit(1);
        fatal_error_system(mesg);
    }
}