/*
 * iterPartition.h
 *
 *  Created on: Dec 12, 2015
 *      Author: mind
 */

#ifndef LCSPARSER_H_
#define LCSPARSER_H_

#include "common.h"
#include <ctime>
#include "logTrie.h"
//#include "MurmurHash3.h"
//#include "indexTrie.h"

class NodeT {
public:
    NodeT() {
    	m_marker = -1;
    	for (int i=0; i<50; i++)
    		m_children[i]=NULL;
    }
    ~NodeT() {}
    NodeT* findChild(char c);
    int getId(char chd) {
    	int id=-1;
    	if (chd>='a' && chd<='z')
    		id = chd-'a';
    	else if (chd>='0' && chd<='9')
    		id = 'z'-'a'+1 + chd-'0';
    	else //if (chd=='(')
    		id = 'z'-'a' + '9'-'0'+2 +1;
/*
    	else if (chd==')')
    		id = 'z'-'a' + '9'-'0'+2 +2;
    	else if (chd==',')
    		id = 'z'-'a' + '9'-'0'+2 +3;
    	else if (chd==':')
    		id = 'z'-'a' + '9'-'0'+2 +3;
    	else if (chd=='-')
    		id = 'z'-'a' + '9'-'0'+2 +3;
    	else
    		id = 'z'-'a' + '9'-'0'+2 +4;
*/
    	return id;
    }
    void addChild(char chd, NodeT* chdPtr) {
    	int id = getId(chd);
    	m_children[id] = chdPtr;
    }
    int m_marker;
    NodeT* m_children[50];

//    void addChild(char chd, NodeT* chdPtr) { m_children[chd] = chdPtr; }
//    int m_marker;
//    unordered_map<char, NodeT*> m_children;
};

class indexTrie {
public:
	indexTrie();
	virtual ~indexTrie();
    void addStr(string &s, int index);
    int getStrId(string &s);

private:
    NodeT* m_root;
};

class LCSParser {
public:
	LCSParser();
	virtual ~LCSParser();
	// logs are supposed to feed one by one. For now read a log file once and store in a vector,
	// then select from this vector one by one. each vector element is a line in the log
//	void readFile(string fileName, vector<string> &retLogs);
	// main func to process a new log stmt and adjust the current m_LCSMap.
	// when each new logStmt comes, first remove all potential params in m_params;
	// then compare the result with all keys in m_LCSMap, find the one candidate key (LCS)
	// having longest LCS with logStmt,
	// If the candidate LCS is not "valid", then add logStmt to m_LCSMap as a new LCS.
	// Here "valid" means:
	// 1. The resulting LCS of logStmt and candidate LCS is at least half length of logStmt;
	// 2. The diff positions (including previously deleted param positions) in logStmt is
	// a superset of param positions for that candidate LCS.
	// If "valid", then:
	// 1. If number of diff positions is only a "SMALL PERCENT" of whole logStmt, then add it to m_params;
	// 2. replace candidate LCS with the new LCS and add this logStmt line id to the corresponding value of m_LCSMap;
	// 3. replace param positions with the new diff positions
	bool getLCS(string &logStmt, int logType, int prec);
	void dumpLCSMap();
	void dumpLCSMapSum();
	// func to compute length of LCS using dp.
	int LCSLen(vector<string> &l1, vector<string> &l2, int** &lens);
	// func to get the LCS of two input vector strings according to the dp results returned by LCSLen.
	void LCSOf(vector<string> &newLog, vector<string> &l2, vector<string> &retLCS,
			vector<int> &retDiffPos, unordered_set<string> &retParams, int** &ptCanLen);
	void runLCS(vector<string> &oriLogs, int logType, int prec);
	bool m_bSplit = false;
	bool m_bMerge = false;
	enum {prefix=0, invertedList=1, loop=2, hashed=3};
	int m_preCheckMethod=prefix; // 0: prefixTree, 1: invertedList, 2: loop

private:
	// mapped value of each key LCS in m_LCSMap
	// ensure each vector is sorted
	typedef struct LCSObject {
		vector<string> lcsTokens;
		vector<int> lineIds;
		int nStars;
//		vector<int> paramPos;
	} LCSObject;

	vector<int> m_lcsAvailSlots;
	const string m_AVAILSTR = "availableMIND"; // just make sure it's unique
	logTrie m_trie;
	// store possible parameters happened so far.
	unordered_set<string> m_params;
	// store all parsed LCSs happened so far. key "LCS" format: "file * completed at time *", use "*" as a wild card.
	vector<LCSObject> m_LCSMap;
	map<int, int> m_LCSList;
	vector<vector<string>> m_logTokens;
	vector<string> m_logTimeStamp;
	// number of lines processed so far.
	uint m_nLines;
	uint m_nLCSs;
	/// for debug purpose, to see the power of prefix tree
	uint m_nAfterPrefix=0;
	uint m_nNewLCS=0;
	vector<int> m_idsAfterPrefix;
	vector<int> m_idsNewLCS;
	///
	const int m_mergePoint = 1000; // merge every 1000 lines
	const int m_mergeThresCnt = 8;
	const int m_splitThresCnt = 9;
	const int m_numberSplitThresCnt = 1; // if all number, only split when #distincts <= m_numberSplitThresCnt
	const double m_mergeThres = 0.5;
	const int m_minLCSSz = 1;
	int m_totalcnt=0, m_partialcnt=0;
	clock_t m_time1=0, m_time2=0;
	clock_t m_lcsStrTime=0, m_lcsLenTime=0, m_totalTime=0, m_prefixTime=0;
	clock_t m_loopQueryTime=0, m_ilQueryTime=0, m_prefixT;
	int m_nLoop=0, m_nIL=0, m_nPrefix=0;

	int m_globalStrIndex = 0;
	indexTrie m_strIdTrie;

	// 1. group by token cnt and paramPos; 2. if one position has #_of_unique_tokens > m_mergeThres * #_LCSMap ?,
	void mergeLCSMap();
	int getPosForMerge(vector<int> &);
	void mergeLCSMap2();
	bool add2LCS(int &newLCSId);
	void splitLCSMap();
	bool splitBySz(vector<LCSObject>::iterator &it); // whether to split by size. heuristic?
	void rmEmptyMaps();
	int getTokenCnt(int pos, vector<int> &logs);
	int getTokenCntOriLogs(int pos, vector<int> &logs, bool &all_nums);

	vector<int> m_nStars;
	struct  _m_ids {
	    int idMap, idLCS;

	    _m_ids(int a = 0, int b = 0)
	    : idMap(a), idLCS(b){}
	};
	typedef _m_ids m_ids;
	vector<vector<set<int>>> m_invertedList;
//	unordered_map<string, unordered_map<int, set<int>>> m_invertedList;


	int preCheckLCS(vector<string> &logTk, int lineId);
	int rmFromInvertedList(vector<string> &lcsTk, int idMap);
	void dumpInvertedList(); // for debug
	int addToInvertedList(vector<string> &lcsTk, int idMap);
	int preInvertedList(vector<string> &logTk, int lineId);
	int preHashCheckLCS(vector<string> &logTk, int lineId);
	int preHashCheckLCS1(vector<string> &logTk, int lineId);
	unsigned int MurmurHash2 ( const void * key, int len, unsigned int seed );

	void LCSStrs(vector<string> &l1, vector<string> &l2,
			vector<vector<string>> &retLCS, int** &ptCanLen, unordered_set<string> &retParams);
	// recur helper of LCSStrs
	void _LCSStrs(vector<string> &l1, vector<string> &l2, int** &ptCanLen, int i, int j,
			unordered_set<string> &retParams, unordered_set<vector<string>, Utils::VectorHash> &retLCS);

	// func to get the LCS of two input vector strings according to the dp results returned by LCSLen.
//	void LCSOf(vector<string> &newLog, vector<string> &l2, vector<string> &retLCS,
//			vector<int> &retDiffPos, unordered_set<string> &retParams, int** &ptCanLen);
	void LCSStrs(vector<string> &l1, vector<string> &l2, vector<vector<string>> &retLCS);
//	void getTokens(const string &line, vector<string> &logTokens);
	void adjustLCSMap(vector<LCSObject>::iterator &oldLCSIt,
			vector<string> &newLCS /*, vector<int> &newParamPos*/);
	bool fewCommon(vector<string> &l1, vector<string> &l2);
	int getHashCode(string &str);
//	void rmSchemas(const string &line, string &retLine);
//	void getDiffPos(vector<string> Tokens, vector<string> retLCS, vector<int> &retPos);
//	void alloc2dIntArray(int m, int n, int** &array);
//	void dealloc2dIntArray(int m, int** &array);

};


// procedure:
// read file line by line
// maintain LCS-list
// when each line comes, compare each line with the LCS list:
//	1.


# endif /* LCSPARSER_H_ */
