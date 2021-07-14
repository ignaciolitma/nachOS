/// Routines to manage address spaces (memory for executing user programs).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2021 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "address_space.hh"
#include "executable.hh"
#include "threads/system.hh"

#include <stdio.h>
#include <string.h>

#ifndef VMEM

static uint32_t VirtualPageIndex(uint32_t virtualAddress){
  return virtualAddress / PAGE_SIZE;
}

static uint32_t VirtualPageOffset(uint32_t virtualAddress){
  return virtualAddress % PAGE_SIZE;
}

static unsigned VirtualToPhysical(unsigned virtualAddr, TranslationEntry *pageTable) {
    unsigned virtualPageIndex = VirtualPageIndex(virtualAddr);
    unsigned virtualPageOffset = VirtualPageOffset(virtualAddr);
    unsigned frame = pageTable[virtualPageIndex].physicalPage;
    return frame * PAGE_SIZE + virtualPageOffset;
}

#endif

static uint32_t VirtualPageAddress(uint32_t vpn){
  return vpn * PAGE_SIZE;
}

AddressSpace::AddressSpace(OpenFile *executable_file, SpaceId spaceId)
{
    ASSERT(executable_file != nullptr);

    Executable exe (executable_file);
    ASSERT(exe.CheckMagic());

    unsigned size = exe.GetSize() + USER_STACK_SIZE;
      // We need to increase the size to leave room for the stack.
    numPages = DivRoundUp(size, PAGE_SIZE);
    size = numPages * PAGE_SIZE;

    // Ejercicio 3 plancha 4
    // Como indica el enunciado, suponemos que los programas
    // Entran en memoria
    #ifndef DEMAND_LOADING
        ASSERT(numPages <= pageMap->CountClear());
    #endif

    DEBUG('a', "Initializing address space, num pages %u, size %u\n",
          numPages, size);

    // First, set up the translation.
    pageTable = new TranslationEntry[numPages];
    for (unsigned i = 0; i < numPages; i++) {
        pageTable[i].virtualPage  = i;
#ifdef DEMAND_LOADING // Ejercicio 1 plancha 4
        pageTable[i].physicalPage = numPages; // Invalidada por demand loading
        pageTable[i].valid        = false;
#else
        int newPage = pageMap->Find();
        ASSERT(newPage != -1);
        pageTable[i].physicalPage = (unsigned int) newPage;
        pageTable[i].valid        = true;
#endif
        pageTable[i].use          = false;
        pageTable[i].dirty        = false;
        pageTable[i].readOnly     = false;
          // If the code segment was entirely on a separate page, we could
          // set its pages to be read-only.
    }

// Ejercicio 4b plancha 4
#ifdef SWAP
    const unsigned SWAP_FILENAME = MAX_SWAP_FILENAME + 6; // length(SWAP.\0)
    swapFileName = new char[SWAP_FILENAME];
    snprintf(swapFileName, SWAP_FILENAME, "SWAP.%i", spaceId);

    // size = exe->GetSize() + USER_STACK_SIZE
    fileSystem->Create(swapFileName, size);
    swapFile = fileSystem->Open(swapFileName);

#endif

#ifdef VMEM
    exec_file = executable_file;

#else    
    char *mainMemory = machine->GetMMU()->mainMemory;

    for (unsigned i = 0; i < numPages; i++){
        memset(mainMemory + i * PAGE_SIZE, 0, PAGE_SIZE);
    }

    // copy in the code and data segments into memory.
    uint32_t codeSize = exe.GetCodeSize();
    uint32_t initDataSize = exe.GetInitDataSize();

    if (codeSize > 0) {
        uint32_t virtualAddr = exe.GetCodeAddr();
        DEBUG('a', "Initializing code segment, at 0x%X, size %u\n",
              virtualAddr, codeSize);

        unsigned physicalAddr;
        for(unsigned codeByte = 0; codeByte < codeSize; codeByte++){
          physicalAddr = VirtualToPhysical(virtualAddr + codeByte, pageTable);
          exe.ReadCodeBlock(mainMemory + physicalAddr, 1, codeByte);
        }
    }

    if (initDataSize > 0) {
        uint32_t virtualAddr = exe.GetInitDataAddr();
        DEBUG('a', "Initializing data segment, at 0x%X, size %u\n",
              virtualAddr, initDataSize);
        exe.ReadDataBlock(&mainMemory[virtualAddr], initDataSize, 0);

        unsigned physicalAddr;
        for(unsigned codeByte = 0; codeByte < initDataSize; codeByte++){
          physicalAddr = VirtualToPhysical(virtualAddr + codeByte, pageTable);
          exe.ReadDataBlock(mainMemory + physicalAddr, 1, codeByte); //PAGE_SIZE en vez de 1?
        }
    }
#endif

}

/// Deallocate an address space.
AddressSpace::~AddressSpace()
{
    for (unsigned i = 0; i < numPages; i++)
        pageMap->Clear(pageTable[i].physicalPage);

    delete [] pageTable;

    #ifdef VMEM
        delete exec_file;
    #endif

    // Ejercicio 4b plancha 4
    #ifdef SWAP
        coreMap->ReleasePages(this);
        fileSystem->Remove(swapFileName);
        delete [] swapFileName;
        delete swapFile;
    #endif
}

/// Set the initial values for the user-level register set.
///
/// We write these directly into the “machine” registers, so that we can
/// immediately jump to user code.  Note that these will be saved/restored
/// into the `currentThread->userRegisters` when this thread is context
/// switched out.
void
AddressSpace::InitRegisters()
{
    for (unsigned i = 0; i < NUM_TOTAL_REGS; i++) {
        machine->WriteRegister(i, 0);
    }

    // Initial program counter -- must be location of `Start`.
    machine->WriteRegister(PC_REG, 0);

    // Need to also tell MIPS where next instruction is, because of branch
    // delay possibility.
    machine->WriteRegister(NEXT_PC_REG, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we do not
    // accidentally reference off the end!
    machine->WriteRegister(STACK_REG, numPages * PAGE_SIZE - 16);
    DEBUG('a', "Initializing stack register to %u\n",
          numPages * PAGE_SIZE - 16);
}

/// On a context switch, save any machine state, specific to this address
/// space, that needs saving.
///
/// For now, nothing!
void
AddressSpace::SaveState()
{
    #ifdef VMEM // Ejercicio 1 (no indicado en el enunciado) - Plancha 4
    TranslationEntry *tlb = machine -> GetMMU() -> tlb;
    for(unsigned i = 0; i < TLB_SIZE; i++){
          if(tlb[i].valid){
              unsigned pageIndex = tlb[i].virtualPage;
              pageTable[pageIndex].use = tlb[i].use;
              pageTable[pageIndex].dirty = tlb[i].dirty;
          }
      }
    #endif
}

/// On a context switch, restore the machine state so that this address space
/// can run.
///
/// For now, tell the machine where to find the page table.
void
AddressSpace::RestoreState()
{
  #ifdef VMEM  // Ejercicio 1a - Plancha 4
  // Invalidar TLB
  for(unsigned int i = 0 ; i < TLB_SIZE ; i++){
    machine->GetMMU()->tlb[i].valid = false;
  }
  #else
  machine->GetMMU()->pageTable     = pageTable;
  machine->GetMMU()->pageTableSize = numPages;
  #endif
}

#ifdef DEMAND_LOADING

void
AddressSpace::ReadPage(char *mainMemory, uint32_t virtualAddr, int physPageAddr) {

    Executable exe (exec_file);
    ASSERT(exe.CheckMagic());

    uint32_t codeSize = exe.GetCodeSize();
    uint32_t initDataSize = exe.GetInitDataSize();
    uint32_t initCodeAddr = exe.GetCodeAddr();
    uint32_t initDataAddr = exe.GetInitDataAddr();

    if(virtualAddr <= initCodeAddr + codeSize) {
        // Está en el segmento de código
        exe.ReadCodeBlock(mainMemory + physPageAddr * PAGE_SIZE, PAGE_SIZE, virtualAddr);

    } else if (virtualAddr <= initDataAddr + initDataSize) {
        // Está en el segmento de datos
        exe.ReadDataBlock(mainMemory + physPageAddr * PAGE_SIZE, PAGE_SIZE, virtualAddr);
        
    } else {
        // Está en el segmento stack
        memset(mainMemory + physPageAddr * PAGE_SIZE, 0, PAGE_SIZE);
    }
}


// Ejercicio 3 plancha 4
// Ejercicio 4 plancha 4
TranslationEntry
AddressSpace::LoadPage(int vpn) {

    uint32_t virtualAddr = vpn * PAGE_SIZE;
    char *mainMemory = machine->GetMMU()->mainMemory;

    int newPage = coreMap->BookPage(vpn, this);
    int physPageAddr = newPage * PAGE_SIZE;

    if (pageTable[vpn].physicalPage == numPages || pageTable[vpn].physicalPage != numPages + 1) {
        // Está invalidada por política de demand loading o
        // No está en el área de intercambio
        pageTable[vpn].physicalPage = newPage;
        ReadPage(mainMemory, virtualAddr, physPageAddr);
    } else {
        // Está en el área de intercambio
        pageTable[vpn].physicalPage = newPage;
        ReadFromSwap(vpn, physPageAddr);
        stats->numSwapRead++;
    }

    pageTable[vpn].virtualPage = vpn;
    pageTable[vpn].physicalPage = newPage;
    pageTable[vpn].valid = true;
    pageTable[vpn].readOnly = false;
    pageTable[vpn].use = false;
    pageTable[vpn].dirty = false;
    
    return pageTable[vpn];
}

#endif

#ifdef SWAP

// Ejercicio 4d plancha 4

unsigned
AddressSpace::WriteToSwap(int vpn, uint32_t physAddr) {
    char *mainMemory = machine->GetMMU()->mainMemory;
    int vAddr = VirtualPageAddress(vpn);
    pageTable[vpn].valid = false;

    unsigned resultado;
    if(pageTable[vpn].dirty == false) {
        resultado = numPages + 1; // Retorno especial - No es necesario escribirla en swap
    } else {
        resultado = swapFile->WriteAt(mainMemory + physAddr, PAGE_SIZE, vAddr);
    }

    // Remover página de memoria
    memset(mainMemory + physAddr, 0, PAGE_SIZE);

    // Invalidar entrada en la TLB
    for(unsigned i=0; i < TLB_SIZE; i++) {
        if(machine->GetMMU()->tlb[i].virtualPage == (unsigned) vpn)
            machine->GetMMU()->tlb[i].valid = false;
    }

    return resultado;
}

int
AddressSpace::ReadFromSwap(int vpn, uint32_t physAddr) {
    int vAddr = VirtualPageAddress(vpn);
    char *mainMemory = machine->GetMMU()->mainMemory;
    return swapFile->ReadAt(mainMemory + physAddr, PAGE_SIZE, vAddr);
}

#endif
