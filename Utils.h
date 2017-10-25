/*
 * utils.h
 *
 *  Created on: Dec 15, 2015
 *      Author: mind
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <boost/algorithm/string.hpp>
#include <boost/functional/hash.hpp>
//#include <boost/filesystem.hpp>
#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace std;


class Utils {
public:
	Utils();
	virtual ~Utils();
//	static const uint nIGNORED = 6;

	struct VectorHash {
	    size_t operator()(const std::vector<string>& v) const {
	        unsigned long seed = 0;
	        for (auto &ele : v)
	        	boost::hash_combine(seed, ele);
	        return seed;


/* an alternative version
	    	std::hash<string> hasher;
	        size_t seed = 0;
	        for (int i : v) {
	            seed ^= hasher(i) + 0x9e3779b9 + (seed<<6) + (seed>>2);
	        }
	        return seed;
*/
	    }
	};


	static int m_logType; // 0 for losAlmos and 1 for BGL

	static void readFile(string &fileName, vector<string> &retLogs);

	static void getTokens(const string &line, vector<string> &logTokens);
	static void tokensToStr(const vector<string> &logTokens, string &line);

	static void rmSchemas(const string &line, string &retLine, int nIgnored);

	static void preSysLogs(const string &line, vector<string> &logTokens);

	static bool preHPCLogs(string &line, vector<string> &logTokens, int nIgnored, string &logTimeStamp);

	static bool preSparkLogs(string &line, vector<string> &logTokens, int nIGNORED);

	static bool preBglLogs(string &line, vector<string> &logTokens, int nIgnored, string &logTimeStamp);

	static bool preOpenstackLogs(string &line, vector<string> &logTokens, int nIgnored, string &logTimeStamp);

	static bool preProcessLog(string &line, vector<string> &logTokens, int logType, string &logTimeStamp);

	static void alloc2dIntArray(int m, int n, int** &array);

	static void dealloc2dIntArray(int m, int** &array);

	static int getHashValOfVecStr(vector<string> &inVec);

	static void dumpVecStr(vector<string> &inVec);
	static string VectoString(vector<string> &inVec);
	static void dumpVecInt(vector<int> &inVec);
	static void dumpSetInt(set<int> &inVec);

/*	template <typename T, typename A>
	static void dumpVec(vector<T, A> &inVec);*/

	static void dumpVecVecInt(vector<vector<int>> &inGroup);

	static void dumpVecVecStr(vector<vector<string>> &inGroup);

	static void dumpVecVecVecStr(vector<vector<vector<string>>> &inGroups);

	static void dumpVecMapStrInt(vector<unordered_map<string, int>> &inVec);

	static void rmSpaceAndStars(vector<string> &inVec);

	static void dumpMapIntVecInt(unordered_map<int, vector<int>> &inMap);
	static void dumpMapStrSetStr(unordered_map<string, unordered_set<string>> &inMap);

	static void dumpMapStrVecInt(unordered_map<string, vector<int>> &inMap);
	static void dumpSetVecStr(unordered_set<vector<string>, Utils::VectorHash> &);

	static void dumpMapIntSetStr(unordered_map<int, unordered_set<string>> &inMap);

	static bool equalMapKeys(unordered_map<int, unordered_set<string>> &map1,
			unordered_map<int, unordered_set<string>> &map2);

	static bool is_number(const std::string& s);

};

#endif /* UTILS_H_ */
