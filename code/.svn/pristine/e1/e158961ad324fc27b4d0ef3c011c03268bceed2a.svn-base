/// Routines for managing statistics about Nachos performance.
///
/// DO NOT CHANGE -- these stats are maintained by the machine emulation.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "statistics.hh"
#include "lib/utility.hh"

#include <stdio.h>


/// Initialize performance metrics to zero, at system startup.
Statistics::Statistics()
{
    totalTicks = idleTicks = systemTicks = userTicks = 0;
    numDiskReads = numDiskWrites = 0;
    numConsoleCharsRead = numConsoleCharsWritten = 0;
    numMemAccess = numPageFaults = numPacketsSent = numPacketsRecvd = 0;
#ifdef DFS_TICKS_FIX
    tickResets = 0;
#endif

}

/// Print performance metrics, when we have finished everything at system
/// shutdown.
void
Statistics::Print()
{
#ifdef DFS_TICKS_FIX
    if (tickResets != 0) {
        printf("WARNING: the tick counter was reset %lu times; the following"
               " statistics may be invalid.\n\n", tickResets);
    }
#endif
    printf("Ticks: total %lu, idle %lu, system %lu, user %lu\n",
           totalTicks, idleTicks, systemTicks, userTicks);
    printf("Disk I/O: reads %lu, writes %lu\n", numDiskReads, numDiskWrites);
    printf("Console I/O: reads %lu, writes %lu\n",
           numConsoleCharsRead, numConsoleCharsWritten);

    // Ejercicio 2 plancha 4
    // Nota: una lectura fallida a la TLB seguida de una lectura en memoria exitosa
    // se considera una única lectura fallida
    printf("VMEM\n");
    unsigned long hits = numMemAccess - numPageFaults;
    printf("NMemAccess: %lu, Hits: %lu, Misses: %lu, hit ratio: %.3lu\n",
        numMemAccess,
        hits,
        numPageFaults,
        (numMemAccess - 2*numPageFaults) / hits * 100);
        // Se eliminan las lecturas repetidas: un miss seguido de un hit
        // se contabiliza como un miss

    // Ejercicio 4e plancha 4#
    printf("Número de páginas escritas en swap%lu\nNúmero de páginas leídas de swap%lu\n",
        numSwapWritten, numSwapRead);

    printf("Network I/O: packets received %lu, sent %lu\n",
           numPacketsRecvd, numPacketsSent);
}
