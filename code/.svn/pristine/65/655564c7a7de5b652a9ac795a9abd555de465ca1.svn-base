#include "core_map.hh"
#include <stdlib.h>
#include <time.h>

// Ejercicio 4a plancha 4

CoreMap::CoreMap(){
    pageMap = new Bitmap(NUM_PHYS_PAGES);
    ownerAddSpaces = new AddressSpace *[NUM_PHYS_PAGES];
    virtualPageNum = new unsigned int [NUM_PHYS_PAGES];
    victimStack = new List<int>;

    // Ejercicio 5b plancha 4
    #ifdef PRPOLICY_LRU
      bitsReferencia = new unsigned [NUM_PHYS_PAGES];
    #endif

    #ifdef PRPOLICY_FIFO
        unsigned victimPointer = 0;
    #endif
}

CoreMap::~CoreMap(){
    delete pageMap;
    delete [] ownerAddSpaces;
    delete [] virtualPageNum;
    delete victimStack;
    #ifdef PRPOLICY_LRU
      delete bitsReferencia;
    #endif
}

int
CoreMap::BookPage(unsigned int vpn, AddressSpace *space){
    DEBUG('m',"FindPage start\n");
    int phyPage = pageMap->Find();

    // Ejercicio 5b plancha 4
    #ifdef PRPOLICY_LRU
        coreMap->UpdateReferenceBits(phyPage);
    #endif

    #ifdef SWAP
    if(phyPage == -1){
        phyPage = PickVictim();
        DEBUG('m',"Next victim: %d page\n", phyPage);
   
        int n = ownerAddSpaces[phyPage]->WriteToSwap(virtualPageNum[phyPage], phyPage * PAGE_SIZE);
        // if (n == NUM_PHYS_PAGES + 1)
        //   stats->numSwapWritten++;
        // else
        //   DEBUG('m',"La página no se pudo escribir en el swap\n");
    }
    #endif

  ownerAddSpaces[phyPage] = space;
  virtualPageNum[phyPage] = vpn;

  return phyPage;
}

void
CoreMap::ReleasePages(AddressSpace *addressSpace) {
    for (unsigned i = 0; i < NUM_PHYS_PAGES; i++) {
        if(ownerAddSpaces[i] == addressSpace) {
          pageMap->Clear(i);
          victimStack->Remove(i);
          ownerAddSpaces[i] = nullptr;
        }
    }
}

unsigned
CoreMap::CountClear() {
  return pageMap->CountClear();
}

// Ejercicio 4c plancha 4
int
CoreMap::PickVictim() {

// Ejercicio 5b plancha 4
#ifdef PRPOLICY_LRU
    unsigned maxIndex = -1;
    unsigned max = -1;

    for (unsigned i = 0; i < NUM_PHYS_PAGES; i++) {
        if (bitsReferencia[i] > max) {
            max = bitsReferencia[i];
            maxIndex = i;
        }
    }

    return maxIndex;

#elif PRPOLICY_FIFO
    unsigned victimIndex = victimPointer;
    victimPointer = (victimPointer + 1) % NUM_PHYS_PAGES;
    return victimIndex;
    
#else // Random
    srand(time(NULL));
    unsigned i = 0;
    for (; i < NUM_PHYS_PAGES && pageMap->Test(i); i++);

    int victimIndex = rand() % (i + 1);
    return victimIndex;
#endif
}

// Ejercicio 5b plancha 4
#ifdef PRPOLICY_LRU

void
CoreMap::UpdateReferenceBits(unsigned currentPhysPage) {
    for (unsigned i = 0; i < NUM_PHYS_PAGES; i++) {
        if (currentPhysPage == i)
            bitsReferencia[i] = 0;
        else
            bitsReferencia[i]++;
    }
}

#endif
