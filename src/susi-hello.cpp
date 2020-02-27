#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "SusiIoT.h"

#ifdef WIN32
    #define DEF_SUSIIOT_LIB_NAME    "SusiIoT.dll"
#else
    #define DEF_SUSIIOT_LIB_NAME    "libSusiIoT.so"
#endif

#define SUSI_HELLO_VER	"1.0.0"

typedef int             (__stdcall *f_SusiIoTInitialize)();
typedef int             (__stdcall *f_SusiIoTUninitialize)();
typedef char*           (__stdcall *f_SusiIoTGetPFCapabilityString)();
typedef SusiIoTStatus_t (__stdcall *f_SusiIoTMemFree)(void *address);

HINSTANCE hGetProcIDDLL;

f_SusiIoTInitialize SusiIoTInitialize = NULL;
f_SusiIoTUninitialize SusiIoTUninitialize = NULL;
f_SusiIoTGetPFCapabilityString	SusiIoTGetPFCapabilityString = NULL;
f_SusiIoTMemFree SusiIoTMemFree = NULL;


void SusiIoTMemFree_show (void)
{
    if(SusiIoTMemFree == NULL)
        fprintf (stderr, "SusiIoTMemFree is NULL\n");
    else
        fprintf (stderr, "SusiIoTMemFree is not NULL\n");
}


/* return 1 , init pass
   return 0 , init fail */
int susi_load (void)
{
    int ret=0;

// load library
    hGetProcIDDLL = LoadLibrary(DEF_SUSIIOT_LIB_NAME);

    if (!hGetProcIDDLL) {
        fprintf (stderr, "could not load the dynamic library\n");
        return ret;
    }

// resolve function address
    SusiIoTInitialize = (f_SusiIoTInitialize)GetProcAddress(hGetProcIDDLL, "SusiIoTInitialize");
    if (!SusiIoTInitialize)
        goto susi_load_fail;

    SusiIoTUninitialize = (f_SusiIoTUninitialize)GetProcAddress(hGetProcIDDLL, "SusiIoTUninitialize");
    if (!SusiIoTUninitialize)
        goto susi_load_fail;

    SusiIoTGetPFCapabilityString = (f_SusiIoTGetPFCapabilityString)GetProcAddress(hGetProcIDDLL, "SusiIoTGetPFCapabilityString");
    if (!SusiIoTGetPFCapabilityString)
        goto susi_load_fail;

    SusiIoTMemFree = (f_SusiIoTMemFree)GetProcAddress(hGetProcIDDLL, "SusiIoTMemFree");
    if (!SusiIoTMemFree)
        goto susi_load_fail;

    //SusiIoTMemFree_show();		// for debug

    fprintf (stderr, "load SusiIoT\n");

    return 1;

susi_load_fail:
    fprintf (stderr, "could not locate the function\n");
    return ret;
}


void susi_unload (void)
{
    BOOL fFreeResult;

    SusiIoTInitialize = NULL;
    SusiIoTUninitialize = NULL;
    SusiIoTGetPFCapabilityString = NULL;
    SusiIoTMemFree = NULL;

    if (hGetProcIDDLL)
    {
        fFreeResult = FreeLibrary (hGetProcIDDLL);
        fprintf (stderr, "unload SusiIoT\n");
    }
}


char* susi_get_capability (void)
{
    static char * IotPFIDataJsonStr = NULL;
    SusiIoTStatus_t status;

// init
    status = SusiIoTInitialize();
    if (status != SUSIIOT_STATUS_SUCCESS)
        return IotPFIDataJsonStr;

// get capability
    if(IotPFIDataJsonStr == NULL)
    {
        char * cpbStr = NULL;

        if(SusiIoTGetPFCapabilityString != NULL)
        {
            cpbStr = (char *)SusiIoTGetPFCapabilityString();

            //fprintf (stderr, "cpbStr = %s\n", cpbStr);

#ifdef WIN32
            IotPFIDataJsonStr = _strdup(cpbStr);
#else
            IotPFIDataJsonStr = strdup(cpbStr);
#endif

#if 0	// for now, not to use SusiIoTMemFree, it may cause crash
            //SusiIoTMemFree_show();	// for debug

            if(SusiIoTMemFree != NULL)
            {
                SusiIoTMemFree(cpbStr);
            }
            else
            {
                if(cpbStr)
                    free(cpbStr);
            }
#endif

        }
    }

// uninit
    SusiIoTUninitialize();

    return IotPFIDataJsonStr;
}


int main (void)
{
    int susi_load_stat = 0;

    fprintf (stderr, "susi-hello %s\n", SUSI_HELLO_VER);

    susi_load_stat = susi_load();
    if (susi_load_stat == 1)
    {
        char *capability = NULL;
        capability = susi_get_capability();
        if (capability)
        {
            fprintf (stderr, "PFCapability =\n%s\n", capability);
            free (capability);
        }
    }

    susi_unload();

    return 0;
}
