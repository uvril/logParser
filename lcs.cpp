/*
 * lcs.cpp
 *
 *  Created on: Dec 12, 2015
 *      Author: mind
 */

#include "lcs.h"
//#include "utils.hpp"

NodeT* NodeT::findChild(char c) {
	int id = getId(c);
	if (m_children[id] != NULL)
		return m_children[id];
//	if (m_children.find(c) != m_children.end()) {
//		return m_children[c];
//	}
//	else
	return NULL;
}


indexTrie::indexTrie() {
	// TODO Auto-generated constructor stub
	m_root = new NodeT();
}

indexTrie::~indexTrie() {
	// TODO Auto-generated destructor stub
}

void indexTrie::addStr(string &s, int idInLCSMap) {
    NodeT * current = m_root;

    if ( s.size() == 0 ) {
    	current->m_marker = idInLCSMap; // an empty word
        return;
    }

    for ( uint i = 0; i < s.size(); i++ ) {
        NodeT* child = current->findChild(s[i]);
        if ( child != NULL ) {
            current = child;
        }
        else {
            NodeT* tmp = new NodeT();
            current->addChild(s[i], tmp);
            current = tmp;
        }
        if ( i == s.size() - 1 )
        	current->m_marker = idInLCSMap;
    }
}

int  indexTrie::getStrId(string &s) {
    NodeT* current = m_root;

    if ( current != NULL ) {
        for ( int i = 0; i < s.size(); i++ ) {
#ifdef PRINT_LOGTRIE
      cout<<"current: "<<s[i]<<endl;
      printf("dump current children:\n");
      for (auto it=current->m_children.begin(); it!=current->m_children.end(); it++)
    	  cout << it->first<<": "<<it->second<<endl;
#endif
            NodeT* tmp = current->findChild(s[i]);
            if ( tmp == NULL )
                return -1;
            current = tmp;
        }

        return current->m_marker;
    }

    return -1;
}


LCSParser::LCSParser() {
	m_nLines = 0;
	m_nLCSs = 0;
}

LCSParser::~LCSParser() {

}

bool LCSParser::getLCS(string &oriLog, int logType, int prec) {
	m_preCheckMethod = prec;
	clock_t prefixt=clock();
	if (m_nLines>0 && m_nLines  == 172) {
//		printf("debug");
//		splitLCSMap();
//		mergeLCSMap();
	}
	vector<string> logTokens;
	string logTimeStamp;
	if (!Utils::preProcessLog(oriLog, logTokens, logType, logTimeStamp))
		return false;
	m_logTokens.push_back(logTokens); // now just for split purpose
	m_logTimeStamp.push_back(logTimeStamp);
#ifdef PRINT_LCS
		printf("new log: \n");
		Utils::dumpVecStr(logTokens);
#endif
//	m_prefixTime += clock()-prefixt;

//	clock_t prefixt=clock();

// find in prefix tree
	if (m_preCheckMethod==prefix) {
		vector<string> matchedStr;
		m_nPrefix++;
		clock_t tmp = clock();
		int idInLCSMap = m_trie.fuzzySearchLCS(logTokens, matchedStr, m_nLines);
		m_prefixT += clock()-tmp;
		if (idInLCSMap>=0) { // found a match
			m_LCSMap[idInLCSMap].lineIds.push_back(m_nLines);
			m_LCSList[m_nLines] = idInLCSMap;
			m_nLines++;
			m_prefixTime += clock()-prefixt;
			return true;
		}

		// not found in prefix, try to find in preCheckLCS
/*	try no mn for ICDM16  */
		if (preCheckLCS(logTokens, m_nLines)>=0) { // found a match
			m_nLines++;
			m_prefixTime += clock()-prefixt;
			return true;
		}

		m_prefixTime += clock()-prefixt;
	}
	else if (m_preCheckMethod==loop) {
		// find in preCheckLCS
		if (preCheckLCS(logTokens, m_nLines)>=0) { // found a match
			m_nLines++;
			m_prefixTime += clock()-prefixt;
			return true;
		}
		m_prefixTime += clock()-prefixt;
	}
	else if (m_preCheckMethod==hashed) {
		// find in preHashCheckLCS
		if (preHashCheckLCS1(logTokens, m_nLines)>=0) { // found a match
			m_nLines++;
			m_prefixTime += clock()-prefixt;
			return true;
		}
		m_prefixTime += clock()-prefixt;
	}
	else if (m_preCheckMethod==invertedList) {
		// find in invertedList
		if (preInvertedList(logTokens, m_nLines)>=0) { // found a match
			m_nLines++;
			return true;
		}
	}
// no match in prefix tree, find using naive way
//	cout<<"not pre found: ";
//	Utils::dumpVecStr(logTokens);

	m_nAfterPrefix++;
	m_idsAfterPrefix.push_back(m_nLines);
	vector<LCSObject>::iterator canIt; // iterator for candidate LCS
	uint lenCanLCS = 0; // length of LCS compared with candidate LCS
	uint lcsLen = 100000; // just the biggest integer lcs can reach
	string newCanLCS; // new candidate LCS after compared with logStmt
//	vector<int **> ptsLens;
	int **ptCanLen=NULL;
	clock_t lcsLenTime = clock();
	int canCnt = 0;
	int cnt = 0;
	for (auto it = m_LCSMap.begin(); it != m_LCSMap.end(); it++) {
		vector<string> &lcsToken = it->lcsTokens;
#ifdef PRINT_LCS
		printf("this lcs: \n");
		Utils::dumpVecStr(lcsToken);
#endif

		int **lens;
		Utils::alloc2dIntArray(logTokens.size()+1, lcsToken.size()+1, lens);
		uint len = LCSLen(logTokens, lcsToken, lens);
//		if ( (len >= logTokens.size()/2 + 1) &&
//		cout << "total size: "<<logTokens.size()<<", ceil half"<< ceil((double)logTokens.size()/2) << endl;
		if ( (len >= (int)ceil((double)logTokens.size()/2)) &&
				(len>lenCanLCS || (len==lenCanLCS && lcsToken.size()<lcsLen) ) ) {
			lenCanLCS = len;
			lcsLen = lcsToken.size();
			canIt = it;
			canCnt = cnt;
			if (ptCanLen != NULL)
				Utils::dealloc2dIntArray(logTokens.size()+1, ptCanLen);
			ptCanLen = lens;
		}
		else {
			Utils::dealloc2dIntArray(logTokens.size()+1, lens);
		}
		cnt ++;
	}
	m_lcsLenTime += clock() - lcsLenTime;
	clock_t lcsTime = clock();
//	if ((lenCanLCS < logTokens.size()/2 + 1) || (lenCanLCS < canIt->lcsTokens.size()/2 +1)) {
	if ((lenCanLCS < (int)ceil((double)logTokens.size()/2)) ||
			(lenCanLCS < (int)ceil((double)(canIt->lcsTokens.size())/2))) {
		m_nNewLCS++;
		m_idsNewLCS.push_back(m_nLines);
		LCSObject newLCSObject;
		newLCSObject.lcsTokens = logTokens;
		newLCSObject.lineIds = vector<int>(1, m_nLines);
		newLCSObject.nStars = 0;
//		newLCSObject.paramPos = vector<int>();
		int idInLCS=-1;
		if (m_lcsAvailSlots.size()>0) {
			idInLCS=m_lcsAvailSlots.back();
			m_lcsAvailSlots.pop_back();
			m_LCSMap[idInLCS] = newLCSObject;
		}
		else {
			m_LCSMap.push_back(newLCSObject);
			idInLCS = m_LCSMap.size()-1;
		}
		if (m_preCheckMethod==prefix)	// new log as lcs, simply add into trie
			m_trie.addLCS(m_LCSMap[idInLCS].lcsTokens, idInLCS);
		else if (m_preCheckMethod==invertedList)
			addToInvertedList(m_LCSMap[idInLCS].lcsTokens, idInLCS);
		m_LCSList[m_nLines] = idInLCS;
	}
	else {
		// get LCS of logStmt and it->first
		vector<string> newLCS;
		vector<string> &tmpToken = canIt->lcsTokens;
		vector<int> paramPos;
		unordered_set<string> retParams;
		LCSOf(logTokens, tmpToken, newLCS, paramPos, retParams, ptCanLen);
		Utils::dealloc2dIntArray(logTokens.size()+1, ptCanLen);
#ifdef PRINT_LCS
		printf("candidate lcs: \n");
		Utils::dumpVecStr(canIt->lcsTokens);
		printf("candidate lcs param pos: \n");
//		Utils::dumpVecInt(canIt->paramPos);
		printf("new paramPos: \n");
		Utils::dumpVecInt(paramPos);
		printf("new LCS: \n");
		Utils::dumpVecStr(newLCS);
#endif
		adjustLCSMap(canIt, newLCS);
		m_LCSList[m_nLines] = canCnt;
	}
	m_lcsStrTime += clock()-lcsTime;
	m_nLines++;
	return true;
}

void LCSParser::adjustLCSMap(vector<LCSObject>::iterator &oldLCSIt,
		vector<string> &newLCSTokens /*, vector<int> &newParamPos*/) {
	vector<string> oldLCSTokens = oldLCSIt->lcsTokens;
//	Utils::getTokens(newLCS, newLCSTokens);
	if (oldLCSTokens.size()!=newLCSTokens.size() || !std::equal(oldLCSTokens.begin(), oldLCSTokens.end(), newLCSTokens.begin()))
		{m_nNewLCS++; m_idsNewLCS.push_back(m_nLines);}
	int nStars=0;
	for (int i=0; i<newLCSTokens.size(); i++) {
		if (newLCSTokens[i]=="*")
			nStars++;
	}
	oldLCSIt->nStars = nStars;
	oldLCSIt->lcsTokens = newLCSTokens;
	oldLCSIt->lineIds.push_back(m_nLines);	
	int newLCSId = oldLCSIt-m_LCSMap.begin();
	if (add2LCS(newLCSId)) { // because duplicate, newLCSId changed, no need to add newLCSTokens into logTrie for it already exists
		// modify trie todo: what's wierd is: put it here is too times slower than @line205, though only one line parsed wrong: line 2786561
	//	m_trie.deleteLCS(oldLCSIt->lcsTokens); // must delete here, before changing oldLCSIt->lcsTokens
		if (m_preCheckMethod==prefix)
			m_trie.setMarker(oldLCSTokens, oldLCSIt-m_LCSMap.begin(), newLCSId);
		else if (m_preCheckMethod==invertedList) {
			rmFromInvertedList(oldLCSTokens, oldLCSIt-m_LCSMap.begin());
			addToInvertedList(oldLCSTokens, newLCSId);
		}

	}
	else { // no duplicate, just add new lcs
		if (m_preCheckMethod==prefix)
			m_trie.addLCS(newLCSTokens, newLCSId); // must happen after add2LCS, will use the new lcs id to set m_marker even if duplicated lcs
		else if (m_preCheckMethod==invertedList) {
			rmFromInvertedList(oldLCSTokens, oldLCSIt-m_LCSMap.begin());
			addToInvertedList(newLCSTokens, newLCSId);
		}
	}
}

// check if the new LCS is in previous m_LCSMap, if so, merge
bool LCSParser::add2LCS(int &newLCSId) {
	// check if the new LCS is in previous m_LCSMap, if so, merge
	LCSObject &newLCS = m_LCSMap[newLCSId];
	for (uint l=0; l<m_LCSMap.size(); l++) {
		if (newLCSId==l) continue;
		LCSObject &oldLCS = m_LCSMap[l];
		if ((newLCS.lcsTokens.size()==oldLCS.lcsTokens.size()) &&
				(equal(newLCS.lcsTokens.begin(), newLCS.lcsTokens.end(),
						oldLCS.lcsTokens.begin()))) {
			newLCS.lineIds.insert(newLCS.lineIds.end(),
					oldLCS.lineIds.begin(), oldLCS.lineIds.end());
			//m_LCSMap.erase(m_LCSMap.begin()+l);
			m_LCSMap[newLCSId].lcsTokens=vector<string>(1, m_AVAILSTR);
			m_lcsAvailSlots.push_back(newLCSId);
			newLCSId=l; // remove newLCSId instead, so that we could change id of "oldLCSIt->lcsTokens" to l in logTrie. If rm l, we don't know which oldLCS points to l in logTrie.
#ifdef PRINT_LCSSLOTS
			printf("lcstk:\n");
			Utils::dumpVecStr(m_LCSMap[l].lcsTokens);
			printf("lineIds:\n");
			Utils::dumpVecInt(m_LCSMap[l].lineIds);
			printf("avail slots:\n");
			Utils::dumpVecInt(m_lcsAvailSlots);
#endif
			return true; // need to adjust trie outside
		}
	}
	return false; // no duplicate LCS
}

int LCSParser::preHashCheckLCS(vector<string> &logTk, int lineId) {
//	cout<<"in prehashcheck, lineId: "<<lineId<<endl;
	struct  _ids {
	    int idMap, idLCS, nstars;

	    _ids(int a = 0, int b = 0, int c=0)
	    : idMap(a), idLCS(b), nstars(c){}
	};
	typedef _ids ids;

	int tkSz=logTk.size();
	unordered_map<string, vector<ids>> tkInMap;
//	dumpLCSMapSum();
	for (int i=0; i<m_LCSMap.size(); i++) {
		//cout<<0<<endl;
		int ifirst=0;
		while (ifirst<m_LCSMap[i].lcsTokens.size() && m_LCSMap[i].lcsTokens[ifirst]=="*") 
			ifirst++;
		if (m_LCSMap[i].lcsTokens.size()>=tkSz/2 && ifirst<m_LCSMap[i].lcsTokens.size()) {
			if (tkInMap.find(m_LCSMap[i].lcsTokens[ifirst])==tkInMap.end())
				tkInMap[m_LCSMap[i].lcsTokens[ifirst]] = vector<ids>(1, ids(i, ifirst, ifirst));
			else
				tkInMap[m_LCSMap[i].lcsTokens[ifirst]].push_back(ids(i, ifirst, ifirst));
		}
	}
	int maxMapId=-1;
	int maxLCSLen=-1;
	for (int l=0; l<tkSz; l++) {
		//cout<<1<<endl;
		unordered_map<string, vector<ids>> toAdd;
/*		cout<<"l: "<<l<<" dumpTkInMap: sz: "<<tkInMap.size()<<endl;
		for (auto it=tkInMap.begin(); it!=tkInMap.end(); it++) {
			cout<<"key: "<<it->first<<endl;
			for (int j=0; j<it->second.size(); j++) {
				cout<<"idMap: "<<it->second[j].idMap<<", idLCS: "<<it->second[j].idLCS<<endl;
			}
		}
*/
		if (tkInMap.find(logTk[l]) != tkInMap.end()) {
			vector<ids> &tmpId = tkInMap[logTk[l]];
			for (auto it=tmpId.begin(); it!=tmpId.end(); ) {
				vector<string> &lcsTk = m_LCSMap[it->idMap].lcsTokens;
				it->idLCS++;
				while (it->idLCS<lcsTk.size() && lcsTk[it->idLCS]=="*") {
					it->idLCS++;
					it->nstars++;
				}
				if (it->idLCS<lcsTk.size()) {
					if (toAdd.find(lcsTk[it->idLCS]) != toAdd.end()) {
						toAdd[lcsTk[it->idLCS]].push_back(ids(it->idMap, it->idLCS, it->nstars));
					}
					else {
						toAdd[lcsTk[it->idLCS]] = vector<ids>(1, ids(it->idMap, it->idLCS, it->nstars));
					}

				}
				else { //equal
					int thisLCSLen = it->idLCS - it->nstars;
					if (maxLCSLen < thisLCSLen) {
						maxLCSLen = thisLCSLen;
						maxMapId = it->idMap;
					}
				}
				tmpId.erase(it);
			}
			if (tmpId.size()==0) {
				tkInMap.erase(logTk[l]);
			}
		}
		for (auto it=toAdd.begin(); it!=toAdd.end(); it++) {
			if (tkInMap.find(it->first) != tkInMap.end()) {
				tkInMap[it->first].insert(tkInMap[it->first].end(), it->second.begin(), it->second.end());
			}
			else
				tkInMap[it->first] = it->second;
		}
	}
	for (auto it=tkInMap.begin(); it!=tkInMap.end(); it++) {
		vector<ids> & id = it->second;
		for (int i=0; i<id.size(); i++) {
			while (id[i].idLCS < m_LCSMap[id[i].idMap].lcsTokens.size()
					&& m_LCSMap[id[i].idMap].lcsTokens[id[i].idLCS]=="*") {
				id[i].idLCS++;
				id[i].nstars++;
			}
			int thisLCSLen = id[i].idLCS-id[i].nstars;
			if (id[i].idLCS>=m_LCSMap[id[i].idMap].lcsTokens.size() && maxLCSLen<thisLCSLen) {
				maxLCSLen = thisLCSLen;
				maxMapId = id[i].idMap;
			}
		}
	}
	if ((maxLCSLen>=(tkSz/2+1)) || (tkSz%2==0 && maxLCSLen>=tkSz/2)) {
		m_LCSMap[maxMapId].lineIds.push_back(lineId);	
		m_LCSList[lineId] = maxMapId;
		return 1;
	}
	return -1;
}

int LCSParser::rmFromInvertedList(vector<string> &lcsTk, int idMap) {
	// avoid hash, but to search more?
//	cout<<"dump il in rm: "<<endl;
	dumpInvertedList();
//	cout<<"idMap: "<<idMap<<"lcsTk: ";
//	Utils::dumpVecStr(lcsTk);
/*
	for (auto it=m_invertedList.begin(); it!=m_invertedList.end(); it++) {
//		cout<<"il key: "<< it->first<<" ";
//		cout<<"it->second.size(): "<<it->second.size()<<endl;
		if (it->second.size()>idMap)
			(it->second[idMap]).clear();
	}
*/

	for (int i=0; i<lcsTk.size(); i++) {
		if (lcsTk[i]=="*") continue;
		int strId = m_strIdTrie.getStrId(lcsTk[i]);
		m_invertedList[strId][idMap].clear();
	}

	return 0;
}

int LCSParser::addToInvertedList(vector<string> &lcsTk, int idMap) {
	if (m_nStars.size()<=idMap) {
		m_nStars.resize(idMap+1);
	}
	int nStar=0;
	for (int i=0; i<lcsTk.size(); i++) {
		if (lcsTk[i]=="*") {
			nStar++;
			continue;
		}
		int idLCS = i-nStar;
		int strId = m_strIdTrie.getStrId(lcsTk[i]);
		if (strId<0) {
			strId = m_globalStrIndex++;
			m_strIdTrie.addStr(lcsTk[i], strId);
			if (m_invertedList.size()<=strId)
				m_invertedList.resize(strId+1);
			m_invertedList[strId] = vector<set<int>>(idMap+1, set<int>());
		}
		else {
			if (m_invertedList[strId].size()<=idMap) {
				m_invertedList[strId].resize(idMap+1);
			}
		}
//		cout<<"checkstr: "<<lcsTk[i]<<", id: "<<strId<<endl;
		m_invertedList[strId][idMap].insert(idLCS);
	}
	m_nStars[idMap] = nStar;
	return 0;
}

void LCSParser::dumpInvertedList() {
#ifdef PRINT_LCS
	printf("dump invertedList\n");
	for (auto it=m_invertedList.begin(); it!=m_invertedList.end(); it++) {
		cout<< it->first<<" ";
		for (int i=0; i<it->second.size(); i++) {
			cout<<"map id: "<<i<<", lcsTk ids: (";
			for (auto itk=it->second[i].begin(); itk!=it->second[i].end(); itk++)
				cout<<*itk<<", ";
			cout<<") "<<endl;
		}
		cout<<endl;
	}
#endif

}

#define MO

int LCSParser::preInvertedList(vector<string> &logTk, int lineId) {
	clock_t timeBegin=clock();

#ifdef MO
	int ret = -1;
	int tkSz = logTk.size();
	int maxLen = -1;
	int maxId = -1;

	vector<int> strIds = vector<int> (logTk.size(), -1);
	for (int i=0; i<logTk.size(); i++) {
		strIds[i] = m_strIdTrie.getStrId(logTk[i]);
	}

	for (int im=0; im<m_LCSMap.size(); im++) {
		int idToFnd = 0;
		int nMatched = 0;
		if ((m_LCSMap[im].lcsTokens.size() - m_LCSMap[im].nStars > tkSz)
				|| (m_LCSMap[im].lcsTokens.size() < 1/2 * tkSz))
			continue;
		for (int il=0; il<tkSz; il++) {
			int strId = strIds[il]; //m_strIdTrie.getStrId(logTk[il]);
			if (strId<0	|| m_invertedList[strId].size()<=im)
				continue;
			if (m_invertedList[strId][im].find(idToFnd) != m_invertedList[strId][im].end()) {
				nMatched++;
				idToFnd++;
			}
//			for (auto rit=m_invertedList[strId][im].begin();
//					rit!=m_invertedList[strId][im].end(); rit++) {
//				if (*rit == idToFnd) {
//					nMatched++;
//					idToFnd++;
//					break;
//				}
//			}
		}
		if ((maxLen < nMatched) && (idToFnd==m_LCSMap[im].lcsTokens.size()-m_nStars[im])) {
			maxLen = nMatched;
			maxId = im;
		}
	}
	if (maxLen>0 && ((maxLen>=(tkSz/2+1)) || (tkSz%2==0 && maxLen>=tkSz/2))) {
		m_LCSMap[maxId].lineIds.push_back(lineId);
		m_LCSList[lineId] = maxId;
		ret = maxId;
	}

	m_ilQueryTime += clock() - timeBegin;
	return ret;
#endif

#ifndef MO
	int ret=-1;
	dumpInvertedList();
	int tkSz = logTk.size();
	vector<int> nMatched(m_LCSMap.size(), 0);
	vector<int> lastId(m_LCSMap.size(), -1);
	vector<int> pruneId(m_LCSMap.size(), 0);
	for (int i=0; i<m_LCSMap.size(); i++) {
		if ((m_LCSMap[i].lcsTokens.size() - m_LCSMap[i].nStars > tkSz)
				|| (m_LCSMap[i].lcsTokens.size() < 1/2 * tkSz))
			pruneId[i] = -1;
	}
	for (int il=0; il<tkSz; il++) { // use this as outer loop to reduce hashing on tk
		int strId = m_strIdTrie.getStrId(logTk[il]);
		if (strId<0)
			continue;
		vector<set<int>> &idList = m_invertedList[strId];
		for (int im=0; im<m_LCSMap.size(); im++) {
			if(pruneId[im]==-1 || idList.size()<=im || idList[im].size()==0)
				continue;
			int idToFnd = lastId[im]+1;
//			for (set<int>::reverse_iterator rit =idList[im].rbegin(); rit != idList[im].rend(); rit++) {
			if (idList[im].find(idToFnd) != idList[im].end()) {
				nMatched[im]++;
				lastId[im]++;
			}
//			for (auto rit=idList[im].begin(); rit!=idList[im].end(); rit++) {
//				if (*rit == idToFnd) {
//					nMatched[im]++;
//					lastId[im]++;
//					break;
//				}
////				else if (*rit >idToFnd) { // not a match
////					nMatched[im] = 0;
////					pruneId[im] = -1;
////					break;
////				}
//			}
			// all ids smaller than lastId[im]. OK, meaning skip in logTk, goto next
		}
	}
	int maxLen=-1;
	int maxId=-1;
	for (int i=0; i<nMatched.size(); i++) {
//		cout<<lastId[i]<<", "<<m_LCSMap[i].lcsTokens.size()-m_nStars[i]<<endl;
		if ((nMatched[i] > maxLen) && (lastId[i]+1==m_LCSMap[i].lcsTokens.size()-m_nStars[i])) {
			maxLen = nMatched[i];
			maxId = i;
		}
	}
	if (maxLen>0 && ((maxLen>=(tkSz/2+1)) || (tkSz%2==0 && maxLen>=tkSz/2))) {
		m_LCSMap[maxId].lineIds.push_back(lineId);
		m_LCSList[lineId] = maxId;
		ret = maxId;
	}
	m_ilQueryTime += clock() - timeBegin;
	return ret;
#endif
}


int seed=0;
unordered_map<string, int> ttime;
set<string> ttstrs;
// first hash string using murmurhash3 (hope to be faster)
int LCSParser::preHashCheckLCS1(vector<string> &logTk, int lineId) {

	// test speed
	for (int i=0; i<logTk.size(); i++) {
		if (logTk[i]=="*") continue;
		if (ttstrs.find(logTk[i]) != ttstrs.end()) {
			ttstrs.insert(logTk[i]);
		}
//		int id = distance(ttstrs.begin(), ttstrs.find(logTk[i]));

	//		string tmp=logTk[i];
//		int tmp=MurmurHash2(logTk[i].c_str(), logTk[i].size(), 0);
//		if (ttime.find(tmp)!=ttime.end()) ttime[tmp]++;
//		else ttime[tmp]=0;
	}
	return 1; // so it'll also return outside



//	cout<<"in prehashcheck, lineId: "<<lineId<<endl;
	struct  _ids {
	    int idMap, idLCS, nstars;

	    _ids(int a = 0, int b = 0, int c=0)
	    : idMap(a), idLCS(b), nstars(c){}
	};
	typedef _ids ids;

	int tkSz=logTk.size();
	unordered_map<unsigned int, vector<ids>> tkInMap;
//	dumpLCSMapSum();
	for (int i=0; i<m_LCSMap.size(); i++) {
		//cout<<0<<endl;
		int ifirst=0;
		while (ifirst<m_LCSMap[i].lcsTokens.size() && m_LCSMap[i].lcsTokens[ifirst]=="*")
			ifirst++;
		if (m_LCSMap[i].lcsTokens.size()-m_LCSMap[i].nStars <= tkSz && // prune
				m_LCSMap[i].lcsTokens.size()>=tkSz/2 && ifirst<m_LCSMap[i].lcsTokens.size()) {
			string str=m_LCSMap[i].lcsTokens[ifirst];
			unsigned int hashCode = MurmurHash2(str.c_str(), str.size(), seed);
			if (tkInMap.find(hashCode)==tkInMap.end())
				tkInMap[hashCode] = vector<ids>(1, ids(i, ifirst, ifirst));
			else
				tkInMap[hashCode].push_back(ids(i, ifirst, ifirst));
		}
	}
	int maxMapId=-1;
	int maxLCSLen=-1;
	for (int l=0; l<tkSz; l++) {
		//cout<<1<<endl;
		unordered_map<string, vector<ids>> toAdd;
/*		cout<<"l: "<<l<<" dumpTkInMap: sz: "<<tkInMap.size()<<endl;
		for (auto it=tkInMap.begin(); it!=tkInMap.end(); it++) {
			cout<<"key: "<<it->first<<endl;
			for (int j=0; j<it->second.size(); j++) {
				cout<<"idMap: "<<it->second[j].idMap<<", idLCS: "<<it->second[j].idLCS<<endl;
			}
		}
*/
		string str=logTk[l];
		unsigned int hashCode = MurmurHash2(str.c_str(), str.size(), seed);
		if (tkInMap.find(hashCode) != tkInMap.end()) {

			vector<ids> &tmpId = tkInMap[hashCode];
			for (auto it=tmpId.begin(); it!=tmpId.end(); ) {
				vector<string> &lcsTk = m_LCSMap[it->idMap].lcsTokens;
				it->idLCS++;
				while (it->idLCS<lcsTk.size() && lcsTk[it->idLCS]=="*") {
					it->idLCS++;
					it->nstars++;
				}
				if (it->idLCS<lcsTk.size()) {
					if (toAdd.find(lcsTk[it->idLCS]) != toAdd.end()) {
						toAdd[lcsTk[it->idLCS]].push_back(ids(it->idMap, it->idLCS, it->nstars));
					}
					else {
						toAdd[lcsTk[it->idLCS]] = vector<ids>(1, ids(it->idMap, it->idLCS, it->nstars));
					}

				}
				else { //equal
					int thisLCSLen = it->idLCS - it->nstars;
					if (maxLCSLen < thisLCSLen) {
						maxLCSLen = thisLCSLen;
						maxMapId = it->idMap;
					}
				}
				tmpId.erase(it);
			}
			if (tmpId.size()==0) {
				string str=logTk[l];
				unsigned int hashCode = MurmurHash2(str.c_str(), str.size(), seed);
				tkInMap.erase(hashCode);
			}
		}
		for (auto it=toAdd.begin(); it!=toAdd.end(); it++) {
			string str=it->first;
			unsigned int hashCode = MurmurHash2(str.c_str(), str.size(), seed);
			if (tkInMap.find(hashCode) != tkInMap.end()) {
				tkInMap[hashCode].insert(tkInMap[hashCode].end(), it->second.begin(), it->second.end());
			}
			else
				tkInMap[hashCode] = it->second;
		}
	}
	for (auto it=tkInMap.begin(); it!=tkInMap.end(); it++) {
		vector<ids> & id = it->second;
		for (int i=0; i<id.size(); i++) {
			while (id[i].idLCS < m_LCSMap[id[i].idMap].lcsTokens.size()
					&& m_LCSMap[id[i].idMap].lcsTokens[id[i].idLCS]=="*") {
				id[i].idLCS++;
				id[i].nstars++;
			}
			int thisLCSLen = id[i].idLCS-id[i].nstars;
			if (id[i].idLCS>=m_LCSMap[id[i].idMap].lcsTokens.size() && maxLCSLen<thisLCSLen) {
				maxLCSLen = thisLCSLen;
				maxMapId = id[i].idMap;
			}
		}
	}
	if ((maxLCSLen>=(tkSz/2+1)) || (tkSz%2==0 && maxLCSLen>=tkSz/2)) {
		m_LCSMap[maxMapId].lineIds.push_back(lineId);
		m_LCSList[lineId] = maxMapId;
		return 1;
	}
	return -1;
}

int LCSParser::preCheckLCS(vector<string> &logTk, int lineId) {
	m_nLoop++;
	int ret=-1;
	clock_t timeBegin = clock();
	int nCommonLCS[m_LCSMap.size()];
	int maxId=-1;
	int maxCommonLCS=-1;
	for (int m=0; m<m_LCSMap.size(); m++) {
		nCommonLCS[m]=0; // initialize
		int idLCS=0;
		int l=0;
		if (m_LCSMap[m].lcsTokens.size()-m_LCSMap[m].nStars > logTk.size()) // early pruning
			continue;
		for (; l<logTk.size() && idLCS<m_LCSMap[m].lcsTokens.size(); ) {
			if (nCommonLCS[m]+m_LCSMap[m].lcsTokens.size()-idLCS < logTk.size()/2) { // early pruning
				nCommonLCS[m]=-1;
				break;
			}
			if (logTk[l]==m_LCSMap[m].lcsTokens[idLCS]) {
				l++;
				idLCS++;
				nCommonLCS[m]++;
			}
			else if (m_LCSMap[m].lcsTokens[idLCS]=="*") {
				idLCS++;
			}
			else // not equal and not wild card in lcs, skip in log
				l++;
		}
		if (idLCS == m_LCSMap[m].lcsTokens.size())
			//nCommonLCS[m]=idLCS;
			;
		else if (l==logTk.size()) {
			if (m_LCSMap[m].lcsTokens[idLCS] != "*") {
				nCommonLCS[m]=-1;
			}
			else {
				while (idLCS<m_LCSMap[m].lcsTokens.size() && m_LCSMap[m].lcsTokens[idLCS] == "*")
					idLCS++;
				if (idLCS==m_LCSMap[m].lcsTokens.size())
					//nCommonLCS[m]=idLCS;
					;
				else
					nCommonLCS[m]=-1;
			}
		}
		else // -1
			continue;
		if (nCommonLCS[m]>maxCommonLCS) {
			maxCommonLCS = nCommonLCS[m];
			maxId = m;
		}
	}
	int tkSz=logTk.size();
//	cout<<maxCommonLCS<<" ? "<<1/2 * logTk.size() << " = "<< (maxCommonLCS >= (1/2 * logTk.size())) <<endl;
	//if (maxCommonLCS >= ceil((1/2 * (double)logTk.size()))) { // not correct
//	if (maxCommonLCS >= (int)ceil((double)logTk.size()/2)) { // two times slower than above one
	if ((maxCommonLCS>=(tkSz/2+1)) || (tkSz%2==0 && maxCommonLCS>=tkSz/2)) {
		m_LCSMap[maxId].lineIds.push_back(lineId);
		m_LCSList[lineId] = maxId;
		ret = 1;
	}
	m_loopQueryTime += clock() - timeBegin;
	return ret;
}


// right now only split according to first param pos since following ones may not be accurate
void LCSParser::splitLCSMap() { // todo: change it to split according to least unique tk pos
#ifdef PRINT_LCS
	printf("dump LCSMap summary before split\n");
	dumpLCSMapSum();
#endif
	vector<LCSObject> toAdd;
	for (auto it=m_LCSMap.begin(); it!=m_LCSMap.end(); ) {
#ifdef PRINT_LCS
		printf("current message type to check: \n");
		Utils::dumpVecStr(it->lcsTokens);
#endif
		unordered_map<string, LCSObject> tmpAdd;
/*
		if (splitBySz(it)) {
			// split

		}
		else {

		}
*/
		vector<int> paramPos;
		for (uint i=0; i<it->lcsTokens.size(); i++) {
			if (it->lcsTokens[i]=="*") paramPos.push_back(i);
		}
		if (paramPos.size()==0) { // continue if this one has no params
			it++;
			continue;
		}
		int minPos=-1, minTkCnt=INT_MAX;
//		int minNumPos=-1, minNumTkCnt=INT_MAX; // for all num positions
		bool all_nums, min_all_nums=true;
		for (uint pos=0; pos<paramPos.size(); pos++) {
			all_nums = true;
			int tkCnt = getTokenCntOriLogs(paramPos[pos], it->lineIds, all_nums);
			if (minTkCnt > tkCnt) {
				minTkCnt = tkCnt;
				minPos = paramPos[pos];
				min_all_nums = all_nums;
			}
		}
		assert(minPos>=0);
		if ((!min_all_nums && minTkCnt>m_splitThresCnt) ||
				(min_all_nums && minTkCnt > m_numberSplitThresCnt)) {
			it++;
			continue;
		}
		// else, split
		for (uint i=0; i<it->lineIds.size(); i++) {
			string newTk = "";
			if (minPos<m_logTokens[it->lineIds[i]].size())
				newTk = m_logTokens[it->lineIds[i]][minPos]; // todo: here's a tricky one, how do you tell the paramPos-th param is the same position in oriLog?
			if (tmpAdd.find(newTk)==tmpAdd.end()) {
				tmpAdd[newTk] = *it;
				tmpAdd[newTk].lcsTokens = m_logTokens[it->lineIds[i]];
				// for other paramPos except for minPos, replace with original words
				tmpAdd[newTk].lineIds = vector<int>(1, it->lineIds[i]);
			}
			else {
				tmpAdd[newTk].lineIds.push_back(it->lineIds[i]);
				// for other paramPos except for minPos, see if need to assign '*' still
				// could use LCS, for now just use per token checking
				for (uint j=0; j<tmpAdd[newTk].lcsTokens.size(); j++) {
					if (tmpAdd[newTk].lcsTokens[j]=="*" || j>=m_logTokens[it->lineIds[i]].size()
							|| tmpAdd[newTk].lcsTokens[j]==m_logTokens[it->lineIds[i]][j])
						continue;
					else
						tmpAdd[newTk].lcsTokens[j]='*';
				}
			}
		}

		for (auto &ta : tmpAdd) {
			toAdd.push_back(ta.second);
		}
#ifdef PRINT_LCS
		Utils::dumpVecStr(it->lcsTokens);
#endif
		if (m_preCheckMethod==prefix)
			m_trie.deleteLCS(it->lcsTokens);
		else if (m_preCheckMethod==invertedList)
			rmFromInvertedList(it->lcsTokens, it-m_LCSMap.begin());

		it->lcsTokens=vector<string>(1, m_AVAILSTR);
		m_lcsAvailSlots.push_back(it-m_LCSMap.begin());
//		m_LCSMap.erase(it);
	}
	for (auto &it: toAdd) {
		int idInLCS=-1;
		if (m_lcsAvailSlots.size()>0) {
			idInLCS=m_lcsAvailSlots.back();
			m_lcsAvailSlots.pop_back();
			m_LCSMap[idInLCS] = it;
		}
		else {
			m_LCSMap.push_back(it);
			idInLCS = m_LCSMap.size()-1;
		}
		if (m_preCheckMethod==prefix)
			m_trie.addLCS(it.lcsTokens, idInLCS);
		else if (m_preCheckMethod==invertedList)
			addToInvertedList(it.lcsTokens, idInLCS);

	}
#ifdef PRINT_LCS
	printf("dump LCSMap after split\n");
	dumpLCSMapSum();
#endif
}

void LCSParser::mergeLCSMap() {
#ifdef PRINT_LCS
	printf("dump LCSMap before merge\n");
	dumpLCSMap();
#endif
	vector<vector<int>> gpbyTks;
	for (uint i=0; i<m_LCSMap.size(); i++) {
		uint sz=m_LCSMap[i].lcsTokens.size();
		if (sz<=m_minLCSSz) continue; // not merge lcs with min lcs sz already.
		if (sz>=gpbyTks.size())
			gpbyTks.resize(sz+1);
		gpbyTks[sz].push_back(i);
	}
#ifdef PRINT_LCS
	printf("dump gpbyTks\n");
	Utils::dumpVecVecInt(gpbyTks);
#endif
	for (uint i=0; i<gpbyTks.size(); i++) {
		if (gpbyTks[i].size()==0)
			continue;
		vector<vector<int>> gpbyPms(1, vector<int>(1, gpbyTks[i][0]));
		for (uint j=1; j<gpbyTks[i].size(); j++) {
			uint k=0;
			for (; k<gpbyPms.size(); k++) {
			}
			if (k==gpbyPms.size()) {
				gpbyPms.push_back(vector<int>(1, gpbyTks[i][j]));
			}
		}
#ifdef PRINT_LCS
	printf("gpbyTks id: %d, dump gpbyPms\n", i);
	Utils::dumpVecVecInt(gpbyPms);
#endif
		// for each partition in gpbyPms, check if the pos having most #_unique_tks is greater than m_mergeThres
		for (uint m=0; m<gpbyPms.size(); m++) {
			assert(gpbyPms[m].size() != 0);
			if (gpbyPms[m].size()<m_mergeThresCnt)
				continue;
			int tkCnt = m_LCSMap[gpbyPms[m][0]].lcsTokens.size();
			int maxTkPos = 0;
			int maxTkCnt = 0;
			for (int n=0; n<tkCnt; n++) {
				int thisTkCnt = getTokenCnt(n, gpbyPms[m]);
				if (maxTkCnt < thisTkCnt) {
					maxTkCnt = thisTkCnt;
					maxTkPos = n;
				}
			}
			if (maxTkCnt > m_mergeThresCnt) { // merge everthing in gpbyPms to gpbyPms[0]
				m_LCSMap[gpbyPms[m][0]].lcsTokens[maxTkPos] = "*";
				if ((maxTkPos>0 && m_LCSMap[gpbyPms[m][0]].lcsTokens[maxTkPos-1]=="*") ||
						(maxTkPos<tkCnt-1 && m_LCSMap[gpbyPms[m][0]].lcsTokens[maxTkPos+1]=="*")) {
					m_LCSMap[gpbyPms[m][0]].lcsTokens.erase(m_LCSMap[gpbyPms[m][0]].lcsTokens.begin()+maxTkPos);
				}
				for (uint l=1; l<gpbyPms[m].size(); l++) {
					m_LCSMap[gpbyPms[m][0]].lineIds.insert(m_LCSMap[gpbyPms[m][0]].lineIds.end(),
							m_LCSMap[gpbyPms[m][l]].lineIds.begin(), m_LCSMap[gpbyPms[m][l]].lineIds.end());
					m_LCSMap[gpbyPms[m][l]].lineIds.clear();
				}
				add2LCS(gpbyPms[m][0]);
			}
		}
	}
	rmEmptyMaps();
#ifdef PRINT_LCS
	printf("dump LCSMap after merge\n");
	dumpLCSMap();
#endif
}

int LCSParser::getHashCode(string &str) {
return 0;

}


// -1: no pos, >=0: pos for grouping
int LCSParser::getPosForMerge(vector<int> &mtGroup) {
	// check each pos in m_LCSMap[mtGroup[i]].lcsTokens
#ifdef PRINT_LCS
	printf("dump lcs tokens before getPosForMerge:\n");
	for (int i=0; i<mtGroup.size(); i++) {
		Utils::dumpVecStr(m_LCSMap[mtGroup[i]].lcsTokens);
	}
	printf("ids in lcsmap:\n");
	Utils::dumpVecInt(mtGroup);
#endif
	assert(mtGroup.size()>0);
	int nPos = m_LCSMap[mtGroup[0]].lcsTokens.size();
	int minTkCnt=INT_MAX; // not allowing *, # showing at current pos
	int minTkPos=-1;
	int minTkCntStar=INT_MAX; // allowing *, not allowing #
	int minTkPosStar=-1;
	for (int pos=0; pos<nPos; pos++) {
		bool hasNum=false;
		unordered_set<string> tks;
		int mtId=0;
		for (mtId=0; mtId<mtGroup.size(); mtId++) {
			int lcsId = mtGroup[mtId];
			string curTk = m_LCSMap[lcsId].lcsTokens[pos];
			if (Utils::is_number(curTk)) {
				hasNum=true;
			}
//			else if (curTk.find("*")>=0) {
//				tks.insert("*");
//			}
			else tks.insert(curTk);
		}
		if (mtId==mtGroup.size()) {
			int sz=tks.size();
			if (hasNum) sz++;
			if (sz==1) continue; // unique token, not split at this pos
			if (tks.find("*") == tks.end()) { // not having *
				if (minTkCnt > tks.size()) {
					minTkCnt = tks.size();
					minTkPos = pos;
				}
			}
			else {
				if ((hasNum && sz>2) || (!hasNum && sz>1)) // if only number and *, no. must have a tk other than "*" and "number".
					if (minTkCntStar > tks.size()){
						minTkCntStar = tks.size();
						minTkPosStar = pos;
					}
			}
		}
	}
	if (minTkPos>=0) return minTkPos;
	if (minTkPosStar>=0) return minTkPosStar;

	return -1;
}

void LCSParser::mergeLCSMap2() {
	// idea: first put "similar" message types in one group, then merge message types within each group
	// steps: 1. split all message types using their lengths;
	// 2. further split according to ... least tk pos?
	// split until in each group, all pos have the same tk except:
	// 1) pos having (distinct tks > mergeThres)
	// 2) pos having * ? ...
	// merge each group
#ifdef PRINT_LCS
	printf("dump LCSMap before merge\n");
	dumpLCSMapSum();
#endif
	vector<vector<int>> mtGroups; // each group is the ids in m_LCSMap
	// split by sz
	unordered_map<int, vector<int>> tmpGroups;
	for (int i=0; i<m_LCSMap.size(); i++) {
		int curMtLen = m_LCSMap[i].lcsTokens.size();
		if (tmpGroups.find(curMtLen)!=tmpGroups.end()) {
			tmpGroups[curMtLen].push_back(i);
		}
		else
			tmpGroups[curMtLen] = vector<int>(1, i);
	}
#ifdef PRINT_LCS
	printf("dump tmpGroups after grouping with size\n");
	for (auto it=tmpGroups.begin(); it!=tmpGroups.end(); it++) {
		cout<<"size: "<< it->first<<endl;
		for (int j=0; j<it->second.size(); j++) {
			Utils::dumpVecStr(m_LCSMap[it->second[j]].lcsTokens);
		}
	}
	Utils::dumpMapIntVecInt(tmpGroups);
#endif
	for (auto it=tmpGroups.begin(); it!=tmpGroups.end(); it++) {
		mtGroups.push_back(it->second);
	}
	tmpGroups.clear();

#ifdef PRINT_LCS
	printf("dump mtGroups: \n");
	Utils::dumpVecVecInt(mtGroups);
#endif
	vector<vector<int>> finalMtGroups;
	char stop=0;
	while (!stop) {
		vector<vector<int>> newMtGroups;
		stop=1;
		for (auto it=mtGroups.begin(); it!=mtGroups.end(); it++) {
			if (it->size()<=1) {
				finalMtGroups.push_back(*it);
				continue;
			}
#ifdef PRINT_LCS
	printf("gpid: %d, mtgroup: \n", it-mtGroups.begin());
	Utils::dumpVecInt(*it);
#endif
			unordered_map<string, vector<int>> tmpGroups;
			int pos = getPosForMerge(*it);
			if (pos<0) {
				finalMtGroups.push_back(*it);
				continue;
			}
			stop=0;
			for (int mtId = 0; mtId < it->size(); mtId++) {
				string curTk = m_LCSMap[(*it)[mtId]].lcsTokens[pos];
				if (Utils::is_number(curTk)) curTk="NUM_MIND";
				if (tmpGroups.find(curTk) != tmpGroups.end())
					tmpGroups[curTk].push_back((*it)[mtId]);
				else
					tmpGroups[curTk]=vector<int>(1, (*it)[mtId]);
			}
#ifdef PRINT_LCS
	printf("dump tmpGroups after grouping\n");
	Utils::dumpMapStrVecInt(tmpGroups);
#endif
			for (auto it=tmpGroups.begin(); it!=tmpGroups.end(); it++) {
				newMtGroups.push_back(it->second);
			}
			tmpGroups.clear();
		}
		mtGroups.clear();
		mtGroups=newMtGroups;
	}
	assert(mtGroups.size()==0);
	mtGroups = finalMtGroups;
#ifdef PRINT_LCS
	printf("dump final mt group:\n");
	Utils::dumpVecVecInt(mtGroups);
#endif
	vector<int> toRmLCSIds; // id of lcs to rm in m_LCSMap
	// now merge each group
	for (int gpId=0; gpId<mtGroups.size(); gpId++) {
		if (mtGroups[gpId].size()==1) continue;
		toRmLCSIds.insert(toRmLCSIds.end(), mtGroups[gpId].begin(), mtGroups[gpId].end());
		LCSObject newLCSObj;
		for (int mtId=0; mtId<mtGroups[gpId].size(); mtId++) {
			int lcsId = mtGroups[gpId][mtId];
			newLCSObj.lineIds.insert(newLCSObj.lineIds.end(),
					m_LCSMap[lcsId].lineIds.begin(), m_LCSMap[lcsId].lineIds.end());
		}
		sort(newLCSObj.lineIds.begin(), newLCSObj.lineIds.end());
		assert(mtGroups[gpId].size()>0);
		int posCnt = m_LCSMap[mtGroups[gpId][0]].lcsTokens.size();
		for (int pos=0; pos < posCnt; pos++) {
			string tk = m_LCSMap[mtGroups[gpId][0]].lcsTokens[pos];

			for (int mtId=1; mtId<mtGroups[gpId].size(); mtId++) {
				int lcsId = mtGroups[gpId][mtId];
				if (tk != m_LCSMap[lcsId].lcsTokens[pos]) {
					tk="*";
					break;
				}
			}
			newLCSObj.lcsTokens.push_back(tk);
		}
		int idInLCS=-1;
		if (m_lcsAvailSlots.size()>0) {
			idInLCS=m_lcsAvailSlots.back();
			m_lcsAvailSlots.pop_back();
			m_LCSMap[idInLCS] = newLCSObj;
		}
		else {
			m_LCSMap.push_back(newLCSObj);
			idInLCS = m_LCSMap.size()-1;
		}
//		if (m_preCheckMethod==0)
//			m_trie.addLCS(newLCSObj.lcsTokens, idInLCS);
//		else
//			addToInvertedList(newLCSObj.lcsTokens, idInLCS);
	}
#ifdef PRINT_LCS
	Utils::dumpVecInt(toRmLCSIds);
#endif
	sort(toRmLCSIds.begin(), toRmLCSIds.end());
	for (int i=toRmLCSIds.size()-1; i>=0; i--) { // will iterator pos change during reverse-remove?
		m_LCSMap[toRmLCSIds[i]].lcsTokens=vector<string>(1, m_AVAILSTR);
		m_lcsAvailSlots.push_back(toRmLCSIds[i]);
		//		m_LCSMap.erase(m_LCSMap.begin()+toRmLCSIds[i]);
	}

#ifdef PRINT_LCS
	printf("dump LCSMap after merge\n");
	dumpLCSMapSum();
#endif
}

void LCSParser::rmEmptyMaps() {
	for (auto it=m_LCSMap.begin(); it!=m_LCSMap.end(); ) {
		if (it->lineIds.size()==0) {
			it->lcsTokens=vector<string>(1, m_AVAILSTR);
			m_lcsAvailSlots.push_back(it-m_LCSMap.begin());
#ifdef PRINT_LCSSLOTS
			printf("in rmEmptyMaps, lcstk:\n");
			Utils::dumpVecStr(m_LCSMap[it-m_LCSMap.begin()].lcsTokens);
			printf("in rmEmptyMaps, lineIds:\n");
			Utils::dumpVecInt(m_LCSMap[it-m_LCSMap.begin()].lineIds);
			printf("in rmEmptyMaps, avail slots:\n");
			Utils::dumpVecInt(m_lcsAvailSlots);
#endif

		}
//			m_LCSMap.erase(it);
		else
			it++;
	}
}

int LCSParser::getTokenCnt(int pos, vector<int> &logs) {
	unordered_set<string> uniTokens;
	for (auto &logId : logs) {
		vector<string> &log = m_LCSMap[logId].lcsTokens;
		if (pos<log.size())
			uniTokens.insert(log[pos]);
		else
			uniTokens.insert("");
	}
	return uniTokens.size();
}

int LCSParser::getTokenCntOriLogs(int pos, vector<int> &logs, bool &all_nums) {
	unordered_set<string> uniTokens;
	for (auto &logId : logs) {
		vector<string> &log = m_logTokens[logId];
//		Utils::dumpVecStr(log);
		if (pos<log.size()) {
			uniTokens.insert(log[pos]);
			if (all_nums && !Utils::is_number(log[pos])) {
				all_nums=false;
			}
		}
		else uniTokens.insert("");
	}
	return uniTokens.size();
}

void LCSParser::LCSOf(vector<string> &newLog, vector<string> &l2, vector<string> &retLCS,
		vector<int> &retDiffPos, unordered_set<string> &retParams, int** &ptCanLen) {
	vector<vector<string>> allLCSs;
	LCSStrs(newLog, l2, allLCSs, ptCanLen, retParams);
	vector<string> tbdRetLCS;
	vector<vector<string>>::iterator lgLCSIt=allLCSs.begin();
	if (allLCSs.size()>1) {
		int lgSz = allLCSs.begin()->size();
		for (auto it=allLCSs.begin()+1; it!=allLCSs.end(); it++) {
			if (lgSz<it->size()) {
				lgSz = it->size();
				lgLCSIt = it;
			}
			if (lgSz==it->size()) {
				cout<<"WARNING 3"<<endl;
#ifdef PRINT_LCSOF
	printf("dump all lcs:\n");
	Utils::dumpVecVecStr(allLCSs);
#endif
			}
		}
	}
//	for (vector<string> lcs: allLCSs) { // tbd: what to do if multiple lcs?
		vector<string> lcs = allLCSs.at(lgLCSIt-allLCSs.begin());
		retLCS = lcs;
#ifdef PRINT_LCSOF
		cout <<"lcs: ";
		Utils::dumpVecStr(lcs);
#endif
		vector<string> &lcsTokens=lcs;
//		Utils::getTokens(lcs, lcsTokens);
		for (uint i=0; i<lcsTokens.size(); i++) {
			if (lcsTokens[i]=="*") retDiffPos.push_back(i);
		}
//		break; // for now only deal with the first LCS.
//	}
#ifdef PRINT_LCSOF
	for (string param: retParams) {
		cout << "param: "<<param<<endl;
	}
#endif
}

void LCSParser::_LCSStrs(vector<string> &l1, vector<string> &l2, int** &ptCanLen, int i, int j, // l2 is lcs token, try to save it
		unordered_set<string> &retParams, unordered_set<vector<string>, Utils::VectorHash> &retLCS) {
	//cout << "begin, i: "<< i << ", j: " << j << endl;
	int lenI = i+1, lenJ = j+1; // index of ptCanLen
//	unordered_set<string> ret;
	if (i==-1 || j==-1) {
		vector<string> tmp(0);
		if (l1.size()>0 && l2.size()>0 && l1[0]!=l2[0])
			tmp.push_back("*");
		retLCS.insert(tmp);
	}
	else if (l1[i]==l2[j]) {
		unordered_set<vector<string>, Utils::VectorHash> last;
		_LCSStrs(l1, l2, ptCanLen, i-1, j-1, retParams, last);
//printf("in equal situation, back\n");
		for (auto it=last.begin(); it!=last.end(); it++) {
#ifdef PRINT_LCSOF
            vector<string> tmp1(it->begin(), it->end());
            Utils::dumpVecStr(tmp1);
#endif
			vector<string> tmp = *it;
			tmp.push_back(l1[i]);
			retLCS.insert(tmp);
		}
	}
	else {
		if (i==0 && j==0) {
			vector<string> tmp(1, "*");
			retLCS.insert(tmp);
			return;
		}

		//		cout << ptCanLen[lenI][lenJ-1]<<", "<<ptCanLen[lenI-1][lenJ]<<endl;

		if (ptCanLen[lenI][lenJ-1] > ptCanLen[lenI-1][lenJ]) {
	//	if (ptCanLen[lenI][lenJ-1] >= ptCanLen[lenI-1][lenJ]) {
			unordered_set<vector<string>, Utils::VectorHash> last;
			_LCSStrs(l1, l2, ptCanLen, i, j-1, retParams, last);
			retParams.insert(l2[j]);
//printf("in first greater situation, back\n");
			for (auto it=last.begin(); it!=last.end(); it++) {
//				cout<< *it << endl;
#ifdef PRINT_LCSOF
				vector<string> tmp(it->begin(), it->end());
				Utils::dumpVecStr(tmp);
#endif
				if (it->size()!=0 && (*it).back() != "*") {
					vector<string> tmp = *it;
					tmp.push_back("*");
					retLCS.insert(tmp);
				}
				else retLCS.insert(*it);
			}
//			ret.insert(last.begin(), last.end());
		}
		else {		
//if (ptCanLen[lenI-1][lenJ] >= ptCanLen[lenI][lenJ-1]) {
			retParams.insert(l1[i]);
			unordered_set<vector<string>, Utils::VectorHash> last;
			_LCSStrs(l1, l2, ptCanLen, i-1, j, retParams, last);
//printf("in second situation, back\n");
			for (auto it=last.begin(); it!=last.end(); it++) {
//				cout<< *it << endl;
				if (it->size()!=0 && (*it).back() != "*")  {	// last character of the string (*it)
					vector<string> tmp = *it;
					tmp.push_back("*");
					retLCS.insert(tmp);
				}
				else retLCS.insert(*it);
			}
//			ret.insert(last.begin(), last.end());
		}
	}
//	return ret;
}

// todo: now, "a b c d", "e f c d" produce "c d", try to produce "* c d".
void LCSParser::LCSStrs(vector<string> &l1, vector<string> &l2,
		vector<vector<string>> &retLCS, int** &ptCanLen, unordered_set<string> &retParams) {
	unordered_set<vector<string>, Utils::VectorHash> ret;
#ifdef PRINT_LCSOF
	printf("now to get the lcs of these two: \n");
	printf("l1: ");
	Utils::dumpVecStr(l1);
	printf("l2: ");
	Utils::dumpVecStr(l2);
#endif
	_LCSStrs(l1, l2, ptCanLen, l1.size()-1, l2.size()-1, retParams, ret);
	for (auto it = ret.begin(); it!=ret.end(); it++) {
		retLCS.push_back(*it);
	}
}

bool LCSParser::fewCommon(vector<string> &l1, vector<string> &l2) {
//	vector<int> commons = vector<int>(m_LCSMap.size()*100, 0);
	int commons[m_LCSMap.size()*100];
	memset(commons, '\0', m_LCSMap.size()*100);
	for (int i=0; i<l1.size(); i++) {
		int id = m_strIdTrie.getStrId(l1[i]);
//		if (id >= commons.size())
//			commons.resize(id+1);
		commons[id] = 1;
	}

	int nCommon = 0;
	for (int i=0; i<l2.size(); i++) {
		int id = m_strIdTrie.getStrId(l2[i]);
		if (commons[id]>0)
			nCommon++;
	}
	bool ret = true;
	if (nCommon >= max((int)l1.size()/2, 1))
		ret = false;
	else if (nCommon >= max((int)l2.size()/2, 1))
		ret = false;
	return ret;



//
//	unordered_set<string> strs;
//	int common = 0;
//	for (auto &str:l1) strs.insert(str);
//	for (auto &str:l2)
//		if (strs.find(str) != strs.end()) common++;
////	return (common < l1.size()/2+1) || (common < l2.size()/2+1);
//	return (common < max((int)l1.size()/2, 1)) || (common < max((int)l2.size()/2, 1));

//	this actually slows down
/*
	typedef struct cnt_t {
		int nl1;
		int nl2;
	} cnt_t;
	unordered_map<string, cnt_t> tmp;
	for (auto &str1:l1) {
		if (tmp.find(str1) != tmp.end()) tmp[str1].nl1++;
		else {
			tmp[str1].nl1 = 1;
			tmp[str1].nl2 = 0;
		}
	}
	for (auto &str2:l2) {
		if (tmp.find(str2) != tmp.end()) tmp[str2].nl2++;
		else {
			tmp[str2].nl2 = 1;
			tmp[str2].nl1 = 0;
		}
	}
	int common = 0;
	for (auto &it : tmp) {
		common += it.second.nl1 < it.second.nl2 ? it.second.nl1 : it.second.nl2;
	}

	return (common < l1.size()/2+1) || (common < l2.size()/2+1);
*/
}

int LCSParser::LCSLen(vector<string> &l1, vector<string> &l2, int** &lens) {
	m_totalcnt++;
    if (l1.size()==0 || l2.size()==0) return 0;
    const clock_t time1 = clock();
    if ((l1.size() < l2.size()/2 +1) || (l2.size() < l1.size()/2+1)) {
        m_time1 += clock()-time1;
        m_time2 += clock()-time1;
    	return 0;
    }
/*
    if (fewCommon(l1, l2)) {
        m_time1 += clock()-time1;
        m_time2 += clock()-time1;
    	return 0;
    }
*/
    m_time1 += clock()-time1;
    m_partialcnt++;
    // init
    // lens[i][j]: longest LCS len for l1[:i+1] and l2[:j+1]
    // boundary conditions
    for (uint i=0; i<=l1.size(); i++) {
   		lens[i][0] = 0;
    }
    for (uint i=0; i<=l2.size(); i++) {
   		lens[0][i] = 0;
    }

    // dp
    for (uint i=1; i<=l1.size(); i++) {
    	for (uint j=1; j<=l2.size(); j++) {
            if (l1[i-1]==l2[j-1])
            	lens[i][j] = lens[i-1][j-1]+1;
            else
            	lens[i][j] = std::max(lens[i-1][j], lens[i][j-1]);
    	}
    }
#ifdef PRINT_LCSOF
    for (uint i=0; i<=l1.size(); i++)
    	for (uint j=0; j<=l2.size(); j++)
    		cout<<"i: "<<i<<" j: "<<j<<", "<<lens[i][j]<<endl;
#endif
    m_time2 += clock()-time1;
    return lens[l1.size()][l2.size()];
}


void LCSParser::dumpLCSMap() {
	JSONNode array(JSON_ARRAY);
	int cnt = 0;
	//cout << m_LCSMap.size() << endl;
	for (auto it = m_LCSMap.begin(); it != m_LCSMap.end(); it++) {
		JSONNode child(JSON_NODE);
		child.push_back(JSONNode("keyNo", cnt));
		child.push_back(JSONNode("lcsTokens", Utils::VectoString(it->lcsTokens)));
		array.push_back(child);
		cnt++;
	}
	ofstream fout("LCSmap.txt");
	fout << array.write_formatted() << std::endl;
	fout.close();
}

void LCSParser::dumpLCSMapSum() {
	printf("summary of final partitions:\n");
	m_nLCSs = 0;
	int total = 0;
	for (auto it = m_LCSMap.begin(); it != m_LCSMap.end(); it++) {
		m_nLCSs++;
		cout << "LCS: ";
		Utils::dumpVecStr(it->lcsTokens);
		total += it->lineIds.size();
		cout << "size: "<<it->lineIds.size() <<endl;
	}
	cout<<"not found lines in prefix tree: "<<m_nAfterPrefix<<endl;
	//Utils::dumpVecInt(m_idsAfterPrefix);
	cout<<"log as new LCS inserted: "<<m_nNewLCS<<endl;
	//Utils::dumpVecInt(m_idsNewLCS);
	cout<<"number of lines processed: "<<m_nLines<<endl;
	cout<<"number of LCSs generated: "<<m_nLCSs<<endl;
	cout<<"total line ids in lcs: "<<total<<endl;
	cout<<"totalCnt: "<<m_totalcnt<<"; partialCnt: "<<m_partialcnt<<endl;
	cout<<"precheckTime (prefix): "<<m_prefixT<<", cnt: "<<m_nPrefix<<endl;
	cout<<"precheckTime (inverted list): "<<m_ilQueryTime<<", cnt: "<<m_nIL<<endl;
	cout<<"precheckTime (loop): "<<m_loopQueryTime<<", cnt: "<<m_nLoop<<endl;
	cout << "CLOCKS_PER_SEC: "<<CLOCKS_PER_SEC<<endl;
	//	cout<<"total time in lcs len: "<<m_time2<<"; early ret time in lcs len: "<<m_time1<<endl;
//	cout<<"totalTime: "<<m_totalTime<<"; lcs str time: "<<m_lcsStrTime<<"; lcs len time: "<<m_lcsLenTime<<"; prefix time: "<<m_prefixTime<<endl;
}

void LCSParser::runLCS(vector<string> &oriLogs, int logType, int prec) {
	clock_t totalTime = clock();
	JSONNode array(JSON_ARRAY);
	for (uint i=0; i<oriLogs.size(); i++) {
#ifdef PRINT_LCS
		cout<<i<<": log tokens";
		cout<<i<<": "<<oriLogs[i]<<endl;
#endif
		if (!getLCS(oriLogs[i], logType, prec)) continue;
		JSONNode items(JSON_NODE);
		items.push_back(JSONNode("keyNo", m_LCSList[m_nLines-1]));
		items.push_back(JSONNode("timeStamp", m_logTimeStamp[m_nLines-1]));
		items.push_back(JSONNode("oriTokens", Utils::VectoString(m_logTokens[m_nLines-1])));
		array.push_back(items);
	}
	cout << array.write_formatted() << std::endl;
	m_totalTime += clock()-totalTime;
	if (m_bSplit)
		splitLCSMap();
	if (m_bMerge)
		mergeLCSMap2();
	// final clean up
	for (auto it=m_LCSMap.begin(); it!=m_LCSMap.end(); ) {
		if (it->lcsTokens.size()==1 && it->lcsTokens[0]==m_AVAILSTR) {
			m_LCSMap.erase(it);
		}
		else it++;
	}

	//printf("dump final lcs map:\n");
	dumpLCSMap();
	//dumpLCSMapSum();
}

/*
int main(int argc, char* argv[]) {
	string fileName = "mac_system_t1.log";
	if (argc>1) {
		fileName = argv[1];
	}
	cout<<"fileName: "<<fileName<<endl;

	LCSParser parser;
	vector<string> oriLogs;
	readFile(fileName, oriLogs);
	cout<<"after read file "<<fileName<<endl;
	for (uint i=0; i<oriLogs.size(); i++) {
		cout<<i<<": log tokens"<<endl;
		cout<<i<<": "<<oriLogs[i]<<endl;
		parser.getLCS(oriLogs[i]);
	}
	parser.dumpLCSMap();
}

void LCSParser::LCSOf(vector<string> &newLog, vector<string> &l2, string retLCS,
		vector<int> &retDiffPos, vector<string> &retParams) {
	vector<string> allLCSs;
	LCSStrs(newLog, l2, allLCSs);
	vector<string> tbdRetLCS;
	for (string lcs: allLCSs) {
		cout <<"lcs: "<<lcs<<endl;
		vector<string> lcsTokens;
		getTokens(lcs, lcsTokens);
		cout << lcsTokens.size()<<", "<<newLog.size()<<endl;
        if (lcsTokens.size() < newLog.size()/2 + 1)
        	continue;  // rule 1
        uint j1=0, j2=0;
        vector<int> diff1, diff2;
    	int pCnt1=0, pCnt2 = 0; // redundant param count, like count plus one because of "1 2" in above example.
        string newParam = ""; // could be multiple words
    	int pos = -1; // position of params, if newLog = "file 1 2 completed at time t1", then pos=(1, 5), params=("1 2", t1)
    	uint i=0;
    	for (; i<lcsTokens.size(); i++)  { // e.g.: LCS: 1 4 5 6; l1: 1 2 3 4 5 6; l2: 1 7 4 5 6. This loop with put (2,3,7) as possible params. todo: now you're grouping logs with length, but it's possible same param could have different length.
        	newParam = "";
        	pos = -1;
        	while (i+j1 < newLog.size() && lcsTokens[i] != newLog[i+j1]) {
                if (pos==-1) pos = i+j1-pCnt1;
                else {pCnt1++; newParam += " ";}
                newParam += newLog[i+j1];
                j1 += 1;
            }
        	if (pos>-1) {
        		diff1.push_back(pos);
                retParams.push_back(newParam);
        	}
        	newParam = ""; pos = -1;
            while (i+j2 < l2.size() && lcsTokens[i] != l2[i+j2]) {
//                    print "second while, i", i, "j1", j1, "j2", j2
            	if (pos==-1) pos = i+j1-pCnt2;
            	else {pCnt2++; newParam += " ";}
            	newParam += l2[i+j2];
                j2 ++;
            }
            if (pos>-1) {
                diff2.push_back(pos);
                retParams.push_back(newParam);
            }
        }
    	newParam = ""; pos = -1;
        if (i+j1<newLog.size()) {
        	cout <<"WARNING 1"<<endl;
        }
        if (i+j2<newLog.size()) {
        	cout <<"WARNING 2"<<endl;
        }
        std::sort(diff1.begin(), diff1.end());
        std::sort(diff2.begin(), diff2.end());
        if (diff1 == diff2) {
            tbdRetLCS.push_back(lcs);
//            if 'metadata' in thispa: print "thispa", thispa, "l1", l1, "l2", l2, "lcs", lcsTokens, '(len(l1)-1)/2', (len(l1)-1)/2, 'len(lcsTokens)', len(lcsTokens)
        }
	}
	if (tbdRetLCS.size()>1)
		cout<<"WARNING 3"<<endl;
	if (tbdRetLCS.size()==1)
		retLCS = tbdRetLCS[0];
	else
		retLCS = "";
}

void LCSParser::LCSStrs(vector<string> &l1, vector<string> &l2, vector<string> &retLCS) {
    if (l1.size()==0 || l2.size()==0) return;
    // init
    vector<string> tmp1;
    vector<vector<string>> tmp2(l2.size(), tmp1);
    vector<vector<vector<string>>> allLCSs(l1.size(), tmp2);

//    vector<vector<vector<string>>> allLCSs(l1.size(), vector<vector<string>>(l2.size(), vector<string>()));

    for (uint i=0; i<l1.size(); i++)
    	cout<<l1[i]<<endl;
    for (uint j=0; j<l2.size(); j++)
    	cout<<l2[j]<<endl;

    // boundary conditions
    if (l1[0]==l2[0])
    	allLCSs[0][0].push_back(l1[0]);
    for (uint i=1; i<l1.size(); i++) {
    	if (allLCSs[0][0].size()>0)
    		allLCSs[i][0]=allLCSs[0][0];
        if (l1[i]==l2[0])
        	allLCSs[i][0].push_back(l2[0]);
    }
    for (uint i=1; i<l2.size(); i++) {
    	if (allLCSs[0][0].size()>0)
    		allLCSs[0][i]=allLCSs[0][0];
        if (l1[0]==l2[i])
        	allLCSs[0][i].push_back(l1[0]);
    }
    // dp
    for (uint i=1; i<l1.size(); i++) {
    	for (uint j=1; j<l2.size(); j++) {
    		if (l1[i].compare(l2[j])==0) {
                if (allLCSs[i-1][j-1].size()==0)
                	allLCSs[i][j].push_back(l1[i]);
                else {
                    for (string str: allLCSs[i-1][j-1])
                    	allLCSs[i][j].push_back(str+" "+l1[i]);
                }
            }
            else{
                if (allLCSs[i-1][j].size()==0)
                	allLCSs[i][j] = allLCSs[i][j-1];
                else if (allLCSs[i][j-1].size()==0)
                	allLCSs[i][j] = allLCSs[i-1][j];
                else {
                    int lcslen1 = allLCSs[i-1][j].size();
					int lcslen2 = allLCSs[i][j-1].size();
					if (lcslen1 == lcslen2) {
						allLCSs[i][j] = allLCSs[i-1][j];
						if (allLCSs[i-1][j] != allLCSs[i][j-1])
							allLCSs[i][j].insert(allLCSs[i][j].end(), allLCSs[i][j-1].begin(), allLCSs[i][j-1].end());
					}
					else if (lcslen1 > lcslen2)
                    	allLCSs[i][j] = allLCSs[i-1][j];
                    else
                    	allLCSs[i][j] = allLCSs[i][j-1];
                }
            }
    	}
    }
    retLCS = allLCSs[l1.size()-1][l2.size()-1];
}
 * */
