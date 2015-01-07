#ifndef TSUINT32_H
#define TSUINT32_H

#include <pthread.h>

class TSUInt32
{
public:
    TSUInt32();

    unsigned int operator++();
    void operator--();

    void operator=(const unsigned int _x);
    //operator int();
    operator unsigned int();
    void setRange(const unsigned int min,const unsigned int max);
    unsigned int getMin();
    unsigned int getMax();

private:
    unsigned int x,minval,maxval;
    pthread_rwlock_t rwlock;
};
#endif // TSUINT32_H
