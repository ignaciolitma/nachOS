/// Copyright (c) 2019-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#include "transfer.hh"
#include "lib/utility.hh"
#include "threads/system.hh"

// MAX_PAGE_FAULTS está definido en machine

bool SafeReadMem(int userAddress, unsigned size, int* buffer) {
    if (machine->ReadMem(userAddress++, 1, buffer))
        return true;

    for (int i = 0; i < MAX_PAGE_FAULTS; i++) {
        if (!machine->ReadMem(userAddress++, 1, buffer))
            return true;
    }

    return false;
}

bool SafeWriteMem(int userAddress, unsigned size, int buffer) {
    if (machine->WriteMem(userAddress++, 1, buffer))
        return true;

    for (int i = 0; i < MAX_PAGE_FAULTS; i++) {
        if (!machine->WriteMem(userAddress++, 1, buffer))
            return true;
    }

    return false;
}

void ReadBufferFromUser(int userAddress, char *outBuffer, unsigned byteCount) {
    ASSERT(userAddress != 0);
    ASSERT(outBuffer != nullptr);
    ASSERT(byteCount != 0);

    int temp;
    for (unsigned count = 0; count < byteCount; count++) {
        // Ejercicio 1b - Plancha 4. Tambien se reemplazan los llamados en args.cc
        ASSERT(SafeReadMem(userAddress++, 1, &temp));
        *outBuffer = (unsigned char)temp;
    }
}

bool ReadStringFromUser(int userAddress, char *outString, unsigned maxByteCount) {
    ASSERT(userAddress != 0);
    ASSERT(outString != nullptr);
    ASSERT(maxByteCount != 0);

    unsigned count = 0;
    do
    {
        int temp;
        count++;
        int i;
        for(i = 0; (i<MAX_PAGE_FAULTS) &&  (!(SafeReadMem(userAddress++, 1, &temp))); i++);
        userAddress++;
        ASSERT(i < MAX_PAGE_FAULTS);
        *outString = (unsigned char) temp;
    } while (*outString++ != '\0' && count < maxByteCount);

    return *(outString - 1) == '\0';
}

void WriteBufferToUser(const char *buffer, int userAddress, unsigned byteCount) {
    ASSERT(userAddress != 0);
    ASSERT(buffer != nullptr);
    ASSERT(byteCount != 0);

    for (unsigned count = 0; count < byteCount; count++) {
        ASSERT(SafeWriteMem(userAddress++, 1, (int)*buffer));
    }
}

void WriteStringToUser(const char *string, int userAddress) {
    ASSERT(userAddress != 0);
    ASSERT(string != nullptr);

    do
    {
        ASSERT(SafeWriteMem(userAddress++, 1, (int)*string));
    } while (*string++ != '\0');
}
