#include "tsuint32.h"

TSUInt32::TSUInt32()
{
    rwlock = PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP;
    x = 0;
    minval = 0;
    maxval = 0xFFFFFFFF;
}

void TSUInt32::operator =(const unsigned int _x)
{
    pthread_rwlock_wrlock(&rwlock);
    if ( _x>=minval && _x<=maxval ) x=_x;
    pthread_rwlock_unlock(&rwlock);
}

TSUInt32::operator unsigned int()
{
    unsigned int r;
    pthread_rwlock_rdlock(&rwlock);
    r=x;
    pthread_rwlock_unlock(&rwlock);
    return r;
}

void TSUInt32::setRange(const unsigned int min, const unsigned int max)
{
    pthread_rwlock_wrlock(&rwlock);

    minval = min;
    maxval = max;

    pthread_rwlock_unlock(&rwlock);
}

unsigned int TSUInt32::getMin()
{
    unsigned int r;
    pthread_rwlock_rdlock(&rwlock);
    r=minval;
    pthread_rwlock_unlock(&rwlock);
    return r;
}

unsigned int TSUInt32::getMax()
{
    unsigned int r;
    pthread_rwlock_rdlock(&rwlock);
    r=maxval;
    pthread_rwlock_unlock(&rwlock);
    return r;
}

unsigned int TSUInt32::operator ++()
{
    unsigned int r;
    pthread_rwlock_wrlock(&rwlock);
    if ((x+1)<=maxval) x++;
    else x=0;
    r=x;
    pthread_rwlock_unlock(&rwlock);
    return r;
}

void TSUInt32::operator --()
{
    pthread_rwlock_wrlock(&rwlock);
    if ( (x-1)>=minval ) x--;
    pthread_rwlock_unlock(&rwlock);
}

