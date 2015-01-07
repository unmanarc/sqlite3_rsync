#ifndef HELPER_NODES_H
#define HELPER_NODES_H

#include <string>
#include <list>
#include <stdint.h>

typedef uint64_t u_int64_t;

std::list<u_int64_t> GetOIDNodesByUInt64(std::list<std::string> nodes);

std::list<std::string> ExpandOIDNodes(std::list<std::string> nodes);
std::list<std::string> CompressOIDNodes(std::list<std::string> nodes);

std::list<std::string> CalcMissingNodes(std::list<std::string> remoteNodes, const std::list<std::string> & localNodes);

#endif // HELPER_NODES_H
