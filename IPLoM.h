/*
 * iterPartition.h
 *
 *  Created on: Dec 15, 2015
 *      Author: mind
 */

#ifndef IPLOM_H_
#define IPLOM_H_

#include "common.h"
#include <ctime>
/*
 * Q1: parByTokenPos(): what if multiple positions have the same # of unique tokens? For now, choose the most left pos.
 *
 */

class IPLoM {
public:
	IPLoM();
	virtual ~IPLoM();
	void runIPLoM(vector<string> &oriLogs, int logType);
	void init();

private:
	enum MapType {OneOne, OneM, MOne, MM};
	// algorithm parameters
	const int m_absFsThres=1;
	const double m_fsThres=0.01; //0.05; // 0.01; //
	const double m_psThres=0; //0.1;
	const double m_upperBnd=0.9;
	const double m_lowerBnd=0.1;
	const double m_cgThres=0.34;
	clock_t m_preTime=0, m_totalTime=0, m_par23Time=0;
	int m_nLogs; // # of logs to be processed

	vector<vector<string>> m_oriLogs; // store original logs
	vector<string> m_logTimeStamp;
	// results after step 1: group log tokens by token count (# of words in a log stmt)
	vector<vector<int>> m_logTokens; // clusters of log ids in m_oriLogs
	unordered_map<string, vector<int>> m_finalPars; // log_key : ids in m_oriLogs of this partition
	vector<int> m_outlierPar; // outlier partition, put in by filePrune func
	void parByTokenCnt(vector<string> &oriLogs, int logType); // step 1
	// par by token position with the least number of unique tokens.
	void parByTokenPos(); // algo 2 - step 2
	void parByBijection(); // algo 4 - step 3
	void parPrune(vector<vector<int>> &); // algo 3
	void filePrune(vector<vector<int>> &); // algo 1
	void getMapPos(vector<int> &par, int &p1, int &p2); // algo 7
	void getP1andP2(vector<int> &par, int &p1, int &p2); // algo 5
	int getRankPos(unordered_set<string> &S, vector<int> &par, int tkPos, MapType mtype); // algo 6
	// helper functions
	int getTokenCnt(int pos, vector<int> &logs);
	int getCnt1Tokens(vector<int> &par); // get # of token positions with only one unique value as cnt1
	MapType getMapType(unordered_set<string> &s1, unordered_set<string> &s2, vector<int> &par, int p1, int p2);
	void parTokenPos (vector<int> &par, vector<vector<int>> &tempC, int tkPos);
	void rmEmptyPars(vector<vector<int>> &pars);
	void getFinalPars(); // m_logTokens to m_finalPars
	void dumpFinalPars();
	bool isCnt1(int tki, vector<int> &par); // if position tki has only one unique token
};

#endif /* IPLOM_H_ */
