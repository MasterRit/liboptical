#include "list.h"
#include "sysdevice.h"

int main(int argc, char**argv)
{
    optcl_list *devices;
    RESULT error = optcl_device_enumerate(&devices);

    if (FAILED(error)) {
        return(error);
    }

    return(0);
}
