/*
 * iterPartition.cpp
 *
 *  Created on: Dec 15, 2015
 *      Author: mind
 */

#include "IPLoM.h"

IPLoM::IPLoM() {
	// TODO Auto-generated constructor stub
	init();
}

IPLoM::~IPLoM() {
	// TODO Auto-generated destructor stub
}

void IPLoM::init() {
	m_logTokens.clear();
	m_outlierPar.clear();
}

void IPLoM::runIPLoM(vector<string> &oriLogs, int logType) {
	clock_t beginT = clock();

	parByTokenCnt(oriLogs, logType);
//	printf("after par by tk cnt \n");
//	getFinalPars();
	clock_t par23beginT = clock();
	parByTokenPos();
//	printf("after par by tk pos \n");
//	getFinalPars();

	parByBijection();
//	printf("after par by bijection \n");
	getFinalPars();
	m_par23Time = clock()-par23beginT;
	m_totalTime += clock()-beginT;
	dumpFinalPars();
}

void IPLoM::parByTokenCnt(vector<string> &oriLogs, int logType) {
	int lineId = 0;
	for (uint i=0; i<oriLogs.size(); i++) {
		clock_t preT = clock();
		string &oriLog = oriLogs[i];
//		string logStmt;
		vector<string> logTokens;
		string logTimeStamp;
		if (!Utils::preProcessLog(oriLog, logTokens, logType, logTimeStamp))
			continue;
		m_oriLogs.push_back(logTokens);
		m_logTimeStamp.push_back(logTimeStamp);
		m_preTime += clock()-preT;
		int sz = logTokens.size();
		if (sz>=m_logTokens.size())
			m_logTokens.resize(sz+1);
		m_logTokens[sz].push_back(lineId);
		lineId++;
	}
	m_nLogs = m_oriLogs.size();
	filePrune(m_logTokens);
#ifdef PRINT
	printf("dump m_logTokens after step 1:\n");
	Utils::dumpVecVecInt(m_logTokens);
	printf("dump outlier partition after step 1:\n");
	Utils::dumpVecInt(m_outlierPar);
#endif
}

void IPLoM::rmEmptyPars(vector<vector<int>> &pars) {
	for (auto it=pars.begin(); it!=pars.end(); ) {
		if (it->size()==0) {
#ifdef PRINT
//			Utils::dumpVecVecStr(*it);
#endif
			pars.erase(it);
		}
		else
			it++;
	}
}

// only partition once, for now use position with least #unique_token_cnt>1
void IPLoM::parByTokenPos() {
	vector<vector<int>> C_out; // output partitions of this step
	for (auto it=m_logTokens.begin(); it!=m_logTokens.end();) {
		// get its par token pos
		if (it->size()==0) continue;
		int logId = (*it)[0];
		int sz = m_oriLogs[logId].size();
		int minTokenCnt = m_nLogs;
		int minTokenPos = 0;
		int tokenCnt = 0;
		for (int pos=0; pos<sz; pos++) {
			tokenCnt = getTokenCnt(pos, *it);
			if (tokenCnt>1 && minTokenCnt>tokenCnt) {
				minTokenCnt = tokenCnt;
				minTokenPos = pos;
			}
		}
		vector<vector<int>> tmpC;
		parTokenPos(*it, tmpC, minTokenPos);
		parPrune(tmpC);
		for (auto &par:tmpC)
			C_out.push_back(par);
		m_logTokens.erase(it);
	}
	parPrune(C_out);
	m_logTokens = C_out;
#ifdef PRINT
	printf("dump m_logTokens after step 2:\n");
	Utils::dumpVecVecInt(m_logTokens);
	printf("dump outlier partition after step 2:\n");
	Utils::dumpVecInt(m_outlierPar);
#endif
}

void IPLoM::parTokenPos (vector<int> &par, vector<vector<int>> &tmpC, int tkPos) {
	unordered_map<string, vector<int>> tmpOut; // token_string : id in &par
	for (uint i=0; i<par.size(); i++) {
		vector<string> &log = m_oriLogs[par[i]];
		if (log.size()<=tkPos) continue;
//cout<<"tkPos: "<<tkPos<<", len(log): "<<log.size()<<endl;
//cout<<log[tkPos]<<endl;
		string &tokenStr = log[tkPos];
//cout<<tokenStr<<endl;
		if (tmpOut.size()>0 || tmpOut.find(tokenStr)!=tmpOut.end())
			tmpOut[tokenStr].push_back(i);
		else
			tmpOut[tokenStr] = vector<int>(1, i);
	}
	for (auto iit=tmpOut.begin(); iit!=tmpOut.end(); iit++) {
		vector<int> tmpPar;
		for (auto id : iit->second)
			tmpPar.push_back(par[id]);
		tmpC.push_back(tmpPar);
	}
}

void IPLoM::parByBijection() {
	vector<vector<int>> addedPars;
	for (auto it=m_logTokens.begin(); it!=m_logTokens.end();) {
		vector<int> &par = *it;
		vector<vector<int>> tmpC;
		int p1, p2;
		getP1andP2(par, p1, p2);
#ifdef PRINT
		printf("p1: %d, p2: %d\n", p1, p2);
#endif
		if (p1==-1 || p2==-1) {
			it++;
			continue; // not split this par
		}
		assert(p1 != p2);
		if (p1 > p2) {
			int tmp = p1;
			p1 = p2;
			p2 = tmp;
		}
		unordered_set<string> s1, s2;
		for (auto &logId : par) {
			vector<string> &log = m_oriLogs[logId];
			s1.insert(log[p1]);
			s2.insert(log[p2]);
		}
		// Q: from line 5-41. If mapping is M-1 or M-M, meaning multiple elements in S1,
		// so shouldn't be "for each element in S1" to determine the mapping type
		MapType mType = getMapType(s1, s2, par, p1, p2);
		int splitPos=-1;
		if (mType==OneOne)
			splitPos = p1;
		else if (mType==OneM) {
			int splitRank = getRankPos(s2, par, p2, OneM);
			splitPos = splitRank==1 ? p1 : p2;
		}
		else if (mType==MOne) {
			int splitRank = getRankPos(s1, par, p1, MOne);
			splitPos = splitRank==2 ? p2 : p1;
		}
		else { // MM (according to journal, all lines with meet M-M relationships are placed in one partition)
			if (1) { // par has gone through step 2
				it++;
				continue;
			}
		}
		// split according to splitPos
		assert(splitPos>=0);
		unordered_map<string, vector<int>> newPars; // token@splitPos : logs in current par having this token@splitPos
		for (auto &logId : par) {
			vector<string> &log = m_oriLogs[logId];
			if (newPars.find(log[splitPos]) != newPars.end()) {
				newPars[log[splitPos]].push_back(logId);
			}
			else
				newPars.insert({log[splitPos], vector<int>(1, logId)});
		}
		for (auto it=newPars.begin(); it!=newPars.end(); it++) {
			addedPars.push_back(it->second);
		}
		m_logTokens.erase(it); // having partitioned current par to multiple partitions, earse current one
	}
	parPrune(addedPars);
	filePrune(addedPars);
	m_logTokens.insert(m_logTokens.end(), addedPars.begin(), addedPars.end());
#ifdef PRINT
	printf("final partitions: \n");
	Utils::dumpVecVecInt(m_logTokens);
#endif
}

IPLoM::MapType
IPLoM::getMapType(unordered_set<string> &s1, unordered_set<string> &s2,
		vector<int> &par, int p1, int p2) {
	MapType retType;
	unordered_map<string, unordered_set<string>> s1M, s2M;
	for (auto &str : s1)
		s1M[str] = unordered_set<string>();
	for (auto &str : s2)
		s2M[str] = unordered_set<string>();
	for (auto &logId : par) {
		vector<string> &log = m_oriLogs[logId];
		s1M[log[p1]].insert(log[p2]);
		s2M[log[p2]].insert(log[p1]);
	}
#ifdef PRINT
	printf("dump s1M: \n");
	Utils::dumpMapStrSetStr(s1M);
	printf("dump s2M: \n");
	Utils::dumpMapStrSetStr(s2M);
#endif
	bool m1=false, m2=false;
	for (auto it=s1M.begin(); it!=s1M.end(); it++) {
		if (it->second.size()>1) {
			m2 = true;
			break;
		}
	}
	for (auto it=s2M.begin(); it!=s2M.end(); it++) {
		if (it->second.size()>1) {
			m1 = true;
			break;
		}
	}
	if (m1&&m2) return MM;
	else if (m2) return OneM;
	else if (m1) return MOne;
	else return OneOne;
	return retType;
}

int IPLoM::getRankPos(unordered_set<string> &S, vector<int> &par, int tkPos, MapType mtype) {
	int splitRank = 0;
	int sCnt = 0;
	for (auto &pId : par) {
		vector<string> &p = m_oriLogs[pId];
		if (S.find(p[tkPos])!=S.end()) sCnt++;
	}
	int dist = (double)S.size()/sCnt;
	if (dist<=m_lowerBnd)	// M side is CONSTANT
		splitRank = mtype==OneM ? 2 : 1; // split at M side
	else if (dist>=m_upperBnd) // M side is VARIABLE
		splitRank = mtype==OneM ? 1 : 2; // split at One side
	else
		splitRank = mtype==OneM ? 1 : 2; // between CONSTANT and VARIABLE thresholds, split at One side
	return splitRank;
}

void IPLoM::parPrune(vector<vector<int>> &collection) {
	vector<int> tmpPar;
	int totalSz = 0;
	for (auto &c:collection)
		totalSz += c.size();
	for (auto it=collection.begin(); it!=collection.end(); ) {
		int parSz = it->size();
		if ((double)parSz/totalSz < m_psThres) {
			tmpPar.insert(tmpPar.end(), it->begin(), it->end());
			collection.erase(it);
		}
		else it++;
	}
	if (tmpPar.size()>0)
		collection.push_back(tmpPar);
	rmEmptyPars(collection);
}

// what about the partitions deleted? meaning there exists logs not in any partitions
void IPLoM::filePrune(vector<vector<int>> &collection) {

	int totalSz = 0;
	for (auto &c:collection)
		totalSz += c.size();
	for (auto it=collection.begin(); it!=collection.end(); ) {
		int parSz = it->size();
		if ((double)parSz/totalSz < m_fsThres) {
//		if (parSz <= m_absFsThres) {
		m_outlierPar.insert(m_outlierPar.end(), it->begin(), it->end());
			collection.erase(it);
		}
		else it++;
	}
	rmEmptyPars(collection);

}

int IPLoM::getTokenCnt(int pos, vector<int> &logs) {
	unordered_set<string> uniTokens;
	for (auto &logId : logs) {
		vector<string> &log = m_oriLogs[logId];
//		cout<<pos<<endl;
//		Utils::dumpVecStr(log);
		uniTokens.insert(log[pos]);
	}
	return uniTokens.size();
}

void IPLoM::getMapPos(vector<int> &par, int &p1, int &p2) {
//	assert(par.size()>0);
	uint tkCnt = m_oriLogs[par[0]].size();
	if (tkCnt==2) {
		p1=0; p2=1;
	}
	else {
		if (1) { // if P went through step 2...
			vector<int> tkCard;
			for (uint i=0; i<tkCnt; i++) {
				tkCard.push_back(getTokenCnt(i, par));
			}
#ifdef PRINT
			Utils::dumpVecInt(tkCard);
#endif
			unordered_map<int, vector<int>> cardPoss; // token_cardinality : list of pos having this cardinality
			for (uint i=0; i<tkCard.size(); i++) {
				if (cardPoss.find(tkCard[i])!=cardPoss.end())
					cardPoss[tkCard[i]].push_back(i);
				else
					cardPoss[tkCard[i]]=vector<int>(1, i);
			}
			uint first=0, second=0; // frequency of cardinality
			uint fcard=0, scard=0;  // token value (# of unique tokens in each pos) having the most and second frequency
			// loop two times to find the first and second most positions
#ifdef PRINT
			printf("dump card poss: \n");
			Utils::dumpMapIntVecInt(cardPoss);
#endif
			for (auto &cp : cardPoss) { // find most frequent
				if (cp.first==1) continue; // cardinality is one
				// if the frequency of current cardinality is bigger, or,
				// frequency is the same but token value is lower (fcard, as freq_card in the paper)
#ifdef PRINT
				printf("cp.first: %d; cp.second: ", cp.first);
				Utils::dumpVecInt(cp.second);
#endif
				if (cp.second.size()>first || (cp.second.size()==first && fcard>cp.first)) {
					first = cp.second.size();
					fcard = cp.first;
				}
			}
			for (auto &cp : cardPoss) { // find second most frequent
				if (cp.first==1) continue; // cardinality is one
				if ((cp.second.size()<=first && cp.first!=fcard) &&
						( cp.second.size()>second || (cp.second.size()==second && scard>cp.first))) {
					second = cp.second.size();
					scard = cp.first;
				}
			}
			assert(fcard != scard);
			int freq_card1, freq_card2;
			if (first==second && fcard>scard) { // second cond cannot happen according to "fcard>cp.first" @line256
				freq_card1 = scard;
				freq_card2 = fcard;
			}
			else { // first>second or (first==second && fcard<scard)
				freq_card1 = fcard;
				freq_card2 = scard;
			}
			// line 12-18
			if (cardPoss[freq_card1].size()>1) {
				p1 = cardPoss[freq_card1][0];
				p2 = cardPoss[freq_card1][1];
			}
			else {
				p1 = cardPoss[freq_card1][0];
				p2 = cardPoss[freq_card2][0];
			}
		}
		else { // p is from step 1
		}
	}
}

void IPLoM::getP1andP2(vector<int> &par, int &p1, int &p2) {
	assert(par.size() > 0);
#ifdef PRINT
	Utils::dumpVecInt(par);
#endif
	uint tkCnt = m_oriLogs[par[0]].size();
	if (tkCnt>2) {
		int cnt1 = getCnt1Tokens(par);
		if ((double)cnt1/tkCnt < m_cgThres) {
			getMapPos(par, p1, p2);
		}
		else {
			p1=-1; p2=-1;
		}
	}
	else if(tkCnt==2) {
		getMapPos(par, p1, p2);
	}
	else {
		p1=-1; p2=-1;
	}

	return; // add par to C_out, iter to nex par. p1, p2 should be -1 to indicate
}

bool IPLoM::isCnt1(int tki, vector<int> &par) {
	uint sz = par.size();
	uint tkCnt = m_oriLogs[par[0]].size();
	uint pi=1;
	for (; pi<sz; pi++) {
		int logId1 = par[pi], logId2 = par[pi-1];
		if (m_oriLogs[logId1][tki] != m_oriLogs[logId2][tki])
			return false;
	}
	return true;
}

int IPLoM::getCnt1Tokens(vector<int> &par) { // get # of token positions with only one unique value as cnt1
	uint tkCnt = m_oriLogs[par[0]].size();
	int cnt1 = 0;
	for (uint tki=0; tki<tkCnt; tki++) {
		if (isCnt1(tki, par))
			cnt1++;
	}
	return cnt1;
}

void IPLoM::getFinalPars() {
	m_logTokens.push_back(m_outlierPar);
	for (auto &par : m_logTokens) {
		if(par.size()==0) continue;
		uint tkCnt = m_oriLogs[par[0]].size();
		string log_key;
		for (uint i=0; i<tkCnt; i++) {
			if (isCnt1(i, par))
				log_key += " "+ m_oriLogs[par[0]][i];
			else
				log_key += " *";
		}
		m_finalPars.insert({log_key, par});
	}
}

void IPLoM::dumpFinalPars() {
	int total=0;
	printf("dump final partitions: \n");
	Utils::dumpMapStrVecInt(m_finalPars);
	printf("summary of final partitions:\n");
	for (auto it=m_finalPars.begin(); it!=m_finalPars.end(); it++) {
		total += it->second.size();
		cout<<it->first<<":  size:  "<<it->second.size()<<";  "<<endl;
	}
	printf("number of lines processed: %d \n", m_oriLogs.size());
	printf("number of partitions generated: %d\n", m_logTokens.size());
	printf("total lines in final pars: %d\n", total);
	cout<< "total time: "<<m_totalTime<<"; pre time: "<<m_preTime<<"; par23 time: "<<m_par23Time<<endl;
}
