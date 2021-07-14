#ifndef NACHOS_VMEM_COREMAP__HH
#define NACHOS_VMEM_COREMAP__HH

#include "lib/bitmap.hh"
#include "lib/list.hh"
#include "machine/mmu.hh"
#include "../userprog/address_space.hh"

class CoreMap{
    public:
        CoreMap();

        ~CoreMap();

        // Reserva una página física y retorna su índice
        // Si todas las páginas están asignadas,
        // La envia al area de intercambio
        int BookPage(unsigned int vpn, AddressSpace *space);

        void ReleasePages(AddressSpace *addressSpace);

        unsigned CountClear();

// Ejercicio 5b plancha 4
#ifdef PRPOLICY_LRU
    private:
        // bitsReferencia[i] representa la cantidad de páginas que se usaron
        // (distintas de la página i) desde la última vez que se usó i
        unsigned *bitsReferencia;
    public:
        // Actualiza los bits de referencia de todas las páginas
        // cada vez que se utiliza una
        void UpdateReferenceBits(unsigned currentPhysPage);
#endif

#ifdef PRPOLICY_FIFO
    private:
        unsigned victimPointer;

#endif
        
    private:
        int PickVictim();

        // Available physical pages
        Bitmap *pageMap;

        // Assigned physical pages
        AddressSpace **ownerAddSpaces;

        unsigned int *virtualPageNum;

        // Lista de páginas ordenadas por least recently used
        List<int> *victimStack;
};

#endif