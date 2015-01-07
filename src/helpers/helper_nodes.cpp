#include "helper_nodes.h"

#include "helper_string.h"

using namespace std;

std::list<u_int64_t> GetOIDNodesByUInt64(std::list<string> nodes)
{
	std::list<u_int64_t> ret;
	for (string oidnode : nodes)
	{
		ret.push_back(StringToUI64(oidnode));
	}
	return ret;
}

std::list<string> ExpandOIDNodes(std::list<string> nodes)
{
	std::list<string> ret;
	for (string oidnode : nodes)
	{
		if (oidnode.find("-") == string::npos)
		{
			// Does not exist. interpret as is.
			ret.push_back(oidnode);
		}
		else
		{
			std::size_t pos = oidnode.find("-");    // position of "live" in str

			string sStartNode = oidnode.substr(0, pos);
			string sEndNode = oidnode.substr(pos + 1);

			u_int64_t iStartNode = StringToUI64(sStartNode);
			u_int64_t iEndNode = StringToUI64(sEndNode);

			if (iEndNode > iStartNode)
			{
				for (u_int64_t iCurrentNode = iStartNode; iCurrentNode <= iEndNode; iCurrentNode++)
				{
					ret.push_back(UI64ToString(iCurrentNode));
				}
			}
		}
	}
	return ret;
}

std::list<string> CompressOIDNodes(std::list<string> nodes)
{
	std::list<string> ret;
	std::string cachedFirst;
	std::string cachedLast;

	for (string oidnode : nodes)
	{
		if (cachedFirst == "")
		{
			// First Node...
			cachedFirst = oidnode;
			cachedLast = oidnode;
		}
		else
		{
			// Next node...
			if (StringToUI64(cachedLast) == StringToUI64(oidnode) - 1)
			{
				// Just Increment..
				cachedLast = oidnode;
			}
			else
			{
				// Consolidate current node.
				if (cachedFirst == cachedLast)
					ret.push_back(cachedFirst);
				else
					ret.push_back(cachedFirst + "-" + cachedLast);

				// First Node...
				cachedFirst = oidnode;
				cachedLast = oidnode;
			}
		}
	}

	// Consolidate current node.
	if (cachedFirst != "")
	{
		if (cachedFirst == cachedLast)
			ret.push_back(cachedFirst);
		else
			ret.push_back(cachedFirst + "-" + cachedLast);
	}

	return ret;
}

std::list<string> CalcMissingNodes(std::list<string> remoteNodes, const std::list<string> &localNodes)
{
	for (const string & lNode : localNodes)
	{
		remoteNodes.remove(lNode);
	}
	return remoteNodes;
}
