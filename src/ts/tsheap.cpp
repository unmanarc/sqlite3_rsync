#include "tsheap.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

TSHeap::TSHeap()
{
    TSUInt32 * _maxElements = new TSUInt32;
    *_maxElements = 100000;

    pthread_mutexattr_init(&l_mattr);
    pthread_mutexattr_setpshared(&l_mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&l_mp, &l_mattr);

    maxElements = _maxElements;
    maxElementsToDestroy = _maxElements;

    pthread_cond_init (&notFull, NULL);
    pthread_cond_init (&notEmpty, NULL);
}

TSHeap::TSHeap(TSUInt32 * _maxElements)
{
    pthread_mutexattr_init(&l_mattr);
    pthread_mutexattr_setpshared(&l_mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&l_mp, &l_mattr);

    maxElements = _maxElements;
    maxElementsToDestroy = NULL;

    pthread_cond_init (&notFull, NULL);
    pthread_cond_init (&notEmpty, NULL);
}

TSHeap::~TSHeap()
{
    if (maxElementsToDestroy) delete maxElementsToDestroy;
}

void * TSHeap::GetElement( )
{
    void * rsp;
    pthread_mutex_lock(&l_mp);

    while (elementsHeap.empty())
    {
        pthread_cond_wait (&notEmpty, &l_mp);
    }

    rsp = elementsHeap.front();
    elementsHeap.pop();

    pthread_mutex_unlock(&l_mp);
    pthread_cond_signal (&notFull);
    return rsp;
}

bool TSHeap::AddElement( void * data )
{
    bool rsp = true;
    pthread_mutex_lock(&l_mp);

    while (elementsHeap.size() >= *maxElements )
    {
        pthread_cond_wait (&notFull, &l_mp);
    }
    elementsHeap.push(data);

    pthread_mutex_unlock(&l_mp);
    pthread_cond_signal (&notEmpty);

    return rsp;
}

u_int32_t TSHeap::GetElementsCount( )
{
    u_int32_t x;
    pthread_mutex_lock(&l_mp);
    x = elementsHeap.size();
    pthread_mutex_unlock(&l_mp);
    return x;
}
