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

#define CONNECTION_FAILURE_RET(x,y) x.Close();if(globalArgs.verbosity>0) fprintf(stderr,"[%p] %s\n",this,y); fflush(stderr); return
#define PRINT_ON_VERBOSE(x) if(globalArgs.verbosity>0) { fprintf(stdout,"[%p] %s\n",this,x); fflush(stdout); }
#define PRINT_ON_VERBOSE_2(x,y) if(globalArgs.verbosity>0) { fprintf(stdout,"[%p] %s : %s\n",this,x,y); fflush(stdout); }
#define PRINT_ON_VERBOSE_STRING_LIST(x,y) if(globalArgs.verbosity>0) { printf("[%p] %s: ",this,x);bool f2a=false;for (std::string oid : y){ if (f2a) { f2a = true; printf("%s",oid.c_str()); } else printf(",%s",oid.c_str());} printf("\n"); fflush(stdout);}


#endif /* VERBOSE_H_ */
