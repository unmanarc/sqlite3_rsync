#ifndef HELPERS_H
#define HELPERS_H

#include <stdint.h>
#include <string>

typedef uint64_t u_int64_t;

u_int64_t StringToUI64(const std::string &var);
std::string UI64ToString(const u_int64_t & var);
void find_and_replace(std::string &s, const std::string &search,
		const std::string &replace);

#endif // HELPERS_H
