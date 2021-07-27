#include "channel.hh"
#include <string.h>
#include <malloc.h>

Channel::Channel(const char *debugName)
{
    name = new char [strlen(debugName) + 1];
    strcpy(name, debugName);
    buffer = nullptr;
    lock = new Lock(name);
    msgSent = new Condition(name, lock);
    msgReceived = new Condition(name, lock);
    done = new Condition(name, lock);
}

Channel::~Channel()
{
    delete lock;
    delete msgSent;
    delete msgReceived;
    delete done;
    delete [] name;
}

const char *
Channel::GetName() const
{
    return name;
}

void
Channel::Send(int message)
{   
    lock->Acquire();

    while (buffer != nullptr){
        DEBUG('c', "Esperando que se vacie el buffer\n");        
        done->Wait();
    }

    DEBUG('c', "Se vacio el buffer\n");  
    buffer = (int*)malloc(sizeof(int));
    *buffer = message;

    DEBUG('c', "Se envio un mensaje\n");        
    msgSent->Signal();

    DEBUG('c', "Esperando que le llegue el mensaje\n");        
    msgReceived->Wait();
    DEBUG('c', "Dou llego el mensaje\n");        

    lock->Release();
}
    
void
Channel::Receive(int *message)
{   
    lock->Acquire();

    while(buffer == nullptr)
    {
        DEBUG('c', "Esperando que haya algo en el buffer\n");        
        msgSent->Wait();
    }

    DEBUG('c', "FIUMBA se vacio el buffer\n");        
    DEBUG('c', "Llego el mensaje\n");        
    *message = *buffer;
    free(buffer);
    buffer = nullptr;

    DEBUG('c', "Avisamos que nos llego el mensaje\n");        
    msgReceived->Signal();
    done->Signal();

    lock->Release();
}
