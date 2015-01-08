/*
 * verbose.h
 *
 *  Created on: Jan 7, 2015
 *      Author: user
 */

#ifndef VERBOSE_H_
#define VERBOSE_H_

#include "global_args.h"
extern globalArgs_t globalArgs;

#define CONNECTION_FAILURE_RET(x,y) x.Close();if(globalArgs.verbosity>0) fprintf(stderr,"[%p] %s",this,y); return
#define PRINT_ON_VERBOSE(x) if(globalArgs.verbosity>0) fprintf(stdout,"[%p] %s",this,x)
#define PRINT_ON_VERBOSE_2(x,y) if(globalArgs.verbosity>0) fprintf(stdout,"[%p] %s:%s",this,x,y)
#define PRINT_ON_VERBOSE_STRING_LIST(x,y) if(globalArgs.verbosity>0) { printf("[%p] %s: ",this,x);bool f2a=false;for (std::string oid : y){ if (f2a) { f2a = true; printf("%s",oid.c_str()); } else printf(",%s",oid.c_str());}}


#endif /* VERBOSE_H_ */
