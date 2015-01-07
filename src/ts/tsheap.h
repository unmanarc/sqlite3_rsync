#ifndef TSHEAP_H
#define TSHEAP_H

#include <pthread.h>
#include <stdint.h>

#include <string>
#include <queue>
#include <vector>
#include "tsuint32.h"

using namespace std;
typedef uint32_t u_int32_t;

typedef queue<void *> ElementsQueue;

class TSHeap
{
public:
	TSHeap();
	TSHeap(TSUInt32 * _maxElements);

	u_int32_t GetElementsCount();

	bool AddElement(void *data);
	void * GetElement();

	~TSHeap();

private:
	// Thread based items:
	pthread_mutexattr_t l_mattr;
	pthread_mutex_t l_mp;
	pthread_cond_t notFull, notEmpty;

	ElementsQueue elementsHeap;
	TSUInt32 * maxElements, *maxElementsToDestroy;
};

#endif // TSHEAP_H
