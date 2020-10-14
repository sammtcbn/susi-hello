#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "SusiIoT.h"

#ifdef __linux__
typedef int BOOL;
#endif

#ifdef WIN32
    #define DEF_SUSIIOT_LIB_NAME    "SusiIoT.dll"
#else
    #define DEF_SUSIIOT_LIB_NAME    "libSusiIoT.so"
#endif

#define SUSI_HELLO_VER	"1.0.1"


#ifdef WIN32
#define SUSI_IOT_API __stdcall
#else
#define SUSI_IOT_API
#endif


typedef SusiIoTStatus_t (SUSI_IOT_API *PSusiIoTInitialize)();
typedef SusiIoTStatus_t (SUSI_IOT_API *PSusiIoTUninitialize)();
typedef const char *    (SUSI_IOT_API *PSusiIoTGetPFCapabilityString)();
typedef SusiIoTStatus_t (SUSI_IOT_API *PSusiIoTMemFree)(void *address);

#ifdef WIN32
HINSTANCE hGetProcIDDLL;
#else
void* hGetProcIDDLL;
#endif


PSusiIoTInitialize            pSusiIoTInitialize = NULL;
PSusiIoTUninitialize          pSusiIoTUninitialize = NULL;
PSusiIoTGetPFCapabilityString pSusiIoTGetPFCapabilityString = NULL;
PSusiIoTMemFree               pSusiIoTMemFree = NULL;


#ifdef WIN32
HINSTANCE OpenLib (const char* path)
{
    return LoadLibrary (path);
}
#else
void* OpenLib (const char* path)
{
    return dlopen (path, RTLD_LAZY);
}
#endif


#ifdef WIN32
int CloseLib (HINSTANCE handle)
{
    BOOL fFreeResult;
    fFreeResult = FreeLibrary (handle);
    if(fFreeResult == FALSE)
        return -1;
    else
        return 0;
}
#else
int CloseLib (void *handle)
{
    return dlclose (handle);
}
#endif


void* GetLibFnAddress (void *handle, const char *name)
{
#ifdef WIN32
    return (void*) GetProcAddress( (HMODULE) handle, name );
#else
    return dlsym (handle, name);
#endif
}


void SusiIoTMemFree_show (void)
{
    if(pSusiIoTMemFree == NULL)
        fprintf (stderr, "pSusiIoTMemFree is NULL\n");
    else
        fprintf (stderr, "pSusiIoTMemFree is not NULL\n");
}


/* return 1 , init pass
   return 0 , init fail */
int susi_load (void)
{
    int ret=0;

// load library
    hGetProcIDDLL = OpenLib (DEF_SUSIIOT_LIB_NAME);

    if (!hGetProcIDDLL) {
        fprintf (stderr, "could not load the dynamic library\n");
        return ret;
    }

// resolve function address
    pSusiIoTInitialize = (PSusiIoTInitialize) GetLibFnAddress(hGetProcIDDLL, "SusiIoTInitialize");
    if (!pSusiIoTInitialize)
        goto susi_load_fail;

	pSusiIoTUninitialize = (PSusiIoTUninitialize) GetLibFnAddress(hGetProcIDDLL, "SusiIoTUninitialize");
    if (!pSusiIoTUninitialize)
        goto susi_load_fail;

	pSusiIoTGetPFCapabilityString = (PSusiIoTGetPFCapabilityString) GetLibFnAddress(hGetProcIDDLL, "SusiIoTGetPFCapabilityString");
    if (!pSusiIoTGetPFCapabilityString)
        goto susi_load_fail;

	pSusiIoTMemFree = (PSusiIoTMemFree) GetLibFnAddress(hGetProcIDDLL, "SusiIoTMemFree");
    if (!pSusiIoTMemFree)
        goto susi_load_fail;

    fprintf (stderr, "load SusiIoT\n");

    return 1;

susi_load_fail:
    fprintf (stderr, "could not locate the function\n");
    return ret;
}


int susi_unload (void)
{
    int bRet = -1;

    if(pSusiIoTUninitialize)
    {
        SusiIoTStatus_t ret = pSusiIoTUninitialize();
        if(ret == SUSIIOT_STATUS_SUCCESS)
        {
            bRet = 0;
        }
    }

    pSusiIoTInitialize = NULL;
    pSusiIoTUninitialize = NULL;
    pSusiIoTGetPFCapabilityString = NULL;
    pSusiIoTMemFree = NULL;

    if (hGetProcIDDLL)
    {
        CloseLib (hGetProcIDDLL);
        fprintf (stderr, "unload SusiIoT\n");
        bRet = 0;
    }
    return bRet;
}


char* susi_get_capability (void)
{
    static char * IotPFIDataJsonStr = NULL;
    SusiIoTStatus_t status;

// init
    status = pSusiIoTInitialize();
    if (status != SUSIIOT_STATUS_SUCCESS)
        return IotPFIDataJsonStr;

// get capability
    if(IotPFIDataJsonStr == NULL)
    {
        char * cpbStr = NULL;

        if(pSusiIoTGetPFCapabilityString != NULL)
        {
            cpbStr = (char *)pSusiIoTGetPFCapabilityString();

            //fprintf (stderr, "cpbStr = %s\n", cpbStr);

#ifdef WIN32
            IotPFIDataJsonStr = _strdup(cpbStr);
#else
            IotPFIDataJsonStr = strdup(cpbStr);
#endif

#if 1   // Susi Free Mem
            if(pSusiIoTMemFree != NULL)
            {
                pSusiIoTMemFree(cpbStr);
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
    pSusiIoTUninitialize();

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
