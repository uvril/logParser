/*
 * Clustering.cpp
 *
 *  Created on: Dec 16, 2015
 *      Author: mind
 */

#include "Clustering.h"
#include "lcs.h"
#include <ctime>

Clustering::Clustering() {
	// TODO Auto-generated constructor stub

}

Clustering::~Clustering() {
	// TODO Auto-generated destructor stub
}

void Clustering::initGroupFixedThres() {
	parClusters2();
}

void Clustering::initGroupAutoThres() {
	getDists();
	getClassThres();
	parClusters();
}

void Clustering::runClustering(vector<string> &oriLogs, int logType) {
//	vector<string> tmp;
//	string logStmt;
	vector<string> logTokens;
	for (uint i=0; i<oriLogs.size(); i++) {
		if (!Utils::preProcessLog(oriLogs[i], logTokens, logType))
			continue;
		Utils::getTokens(oriLogs[i], logTokens);
		m_rks.push_back(logTokens);
	}
	const clock_t begin_time = clock();

	initGroupFixedThres();
//	initGroupAutoThres();

	const clock_t mid_time = clock();
	std::cout << "edit distance time: " << float( mid_time - begin_time ) /  CLOCKS_PER_SEC <<endl;

#ifdef PRINT_CL
	cout << "Print initial cluster ids and keys: "<<endl;
	for (uint i=0; i<m_logIdKeyClusters.size(); i++) {
		cout << "Cluster "<<i<<" contains : "<<endl;
		dumpOneClusterIds(m_logIdKeyClusters[i]);
		cout << endl<<"***************************************"<< endl;
	}
#endif

	int iitt=0;
	bool bSplitting = true;
	while (bSplitting) {
		bSplitting = false;
		vector<vector<ikPair>> toAddGroups;
		int cl=0;
		for (auto it=m_logIdKeyClusters.begin(); it!=m_logIdKeyClusters.end(); ) {
#ifdef PRINT_CL_DEBUG_MISSING
			for (int i=0; i<it->size(); i++) {
				if ((*it)[i].id == 32579) {
					printf("found 32579 in iitt: %d, cl: %d\n", iitt, cl);
					dumpOneClusterIds(*it);
					break;
				}
			}
#endif
			cl++;
			if (it->size()==0 || (it->back()).id==-1) {
				m_logIdKeyClusters.erase(it);
				continue;
			}
			vector<vector<ikPair>> outGroups;
			splitGroups((*it), outGroups);
			if (outGroups.size()==0) {
#ifdef PRINT_CL_DEBUG_MISSING
				printf("outgroup sz is 0, now dump to add to final:\n");
		dumpOneClusterIds(*it);
#endif
				addToFinal(*it);
				m_logIdKeyClusters.erase(it);
			}
			else {
				bSplitting = true; // split in next round
				m_logIdKeyClusters.erase(it);
				for (auto iit=outGroups.begin(); iit!=outGroups.end(); iit++)
					toAddGroups.push_back(*iit);
			}
		}
		for (auto it=toAddGroups.begin(); it!=toAddGroups.end(); it++) {
			m_logIdKeyClusters.push_back(*it);
		}
		iitt++;
	}
	const clock_t end_time = clock();
	std::cout << "split time: " << float( end_time - mid_time ) /  CLOCKS_PER_SEC <<endl;
	dumpClusters();
}

void Clustering::dumpOneCluster(vector<ikPair> &clToDump) {
	for (uint j=0; j<clToDump.size(); j++) {
		cout << clToDump[j].id << ": ";
		Utils::dumpVecStr(clToDump[j].key);
		cout<<endl;
	}
}

void Clustering::dumpOneClusterIds(vector<ikPair> &clToDump) {
	for (uint j=0; j<clToDump.size(); j++) {
		cout << clToDump[j].id << " ";
	}
	cout<<endl;
}


void Clustering::dumpClusters() {
	printf("Final dump m_finalClusters:\n");
	int procLines=0;
	for (auto it=m_finalClusters.begin(); it!=m_finalClusters.end(); it++) {
		procLines += it->second.size();
		cout << "message: "<<it->first<<endl;

		cout << "size: "<<it->second.size()<<endl << "lineIds: ";
		for (uint i=0; i<it->second.size(); i++)
			cout<<it->second[i]<<" ";
		cout<<endl;
//		Utils::dumpVecInt(it->second);
//		cout << endl<<"***************************************"<< endl;
	}
	cout<<"number of lines processed: "<<procLines<<endl;
	cout<<"number of partitions generated: "<<m_finalClusters.size()<<endl;
	cout<<"total line ids in clusters: "<<m_rks.size()<<endl;
}

void Clustering::addToFinal(vector<ikPair> &cluster) {
	assert(cluster.size()>0);
	vector<int> ids;
	for (uint i=0; i<cluster.size(); i++) {
		ids.push_back(cluster[i].id);
	}
	// get key (common part of a cluster)
	string finalKey = "";
	LCSParser lcsParser;
	if (cluster.size()<1) return;
	vector<string> lcsSoFar = cluster[0].key;
	if (cluster.size()>2) {
		for (uint i=1; i<cluster.size(); i++) {
			int **lens;
			Utils::alloc2dIntArray(cluster[i].key.size()+1, lcsSoFar.size()+1, lens);
			uint len = lcsParser.LCSLen(cluster[i].key, lcsSoFar, lens);
			vector<int> paramPos;
			unordered_set<string> retParams;
			lcsParser.LCSOf(cluster[i].key, lcsSoFar, lcsSoFar, paramPos, retParams, lens);
			Utils::dealloc2dIntArray(cluster[i].key.size()+1, lens);
		}
	}
	for (int i=0; i<lcsSoFar.size(); i++) {
		finalKey += ' '+lcsSoFar[i];
	}
#ifdef PRINT_CL_DEBUG_MISSING
	cout<<"final key: "<<finalKey<<endl;
	Utils::dumpVecInt(ids);
#endif
	if (m_finalClusters.find(finalKey) != m_finalClusters.end()) {
		m_finalClusters[finalKey].insert(m_finalClusters[finalKey].begin(), ids.begin(), ids.end());
	}
	else {
		m_finalClusters[finalKey] = ids;
	}
}

void Clustering::splitGroups(vector<ikPair> &inGroup, vector<vector<ikPair>> &outGroups) {
#ifdef PRINT_CL_SG
	printf("inGroup to split:\n");
//	Utils::dumpVecVecStr(inGroup);
#endif
	/*************for now get common log keys by LCS method******start********/
	LCSParser lcsParser;
	if (inGroup.size()<2) return;
	vector<string> lcsSoFar = inGroup[0].key;
	for (uint i=1; i<inGroup.size(); i++) {
		int **lens;
		Utils::alloc2dIntArray(inGroup[i].key.size()+1, lcsSoFar.size()+1, lens);
		uint len = lcsParser.LCSLen(inGroup[i].key, lcsSoFar, lens);
		if (len==0) { // no common, split to two groups and return
  			vector<ikPair> g1(inGroup.begin(), inGroup.begin()+i); // not include i
			vector<ikPair> g2(inGroup.begin()+i, inGroup.end());
			outGroups.push_back(g1);
			outGroups.push_back(g2);
			return;
		}
//		string newLCS;
		vector<int> paramPos;
		unordered_set<string> retParams;
		lcsParser.LCSOf(inGroup[i].key, lcsSoFar, lcsSoFar, paramPos, retParams, lens);
//		Utils::getTokens(newLCS, lcsSoFar);
		Utils::dealloc2dIntArray(inGroup[i].key.size()+1, lens);
	}
	Utils::rmSpaceAndStars(lcsSoFar);
#ifdef PRINT_CL_SG
	cout<<"lcs so far: ";
	Utils::dumpVecStr(lcsSoFar);
#endif
	if (lcsSoFar.size()==0) return; // cannot cluster these
	/*************for now get common log keys by LCS method******end********/
	// Now we have lcsSoFar, which splits each log in inGroup to lcsSoFar.size()+1 positions.
	vector<unordered_map<string, int>> tksEachPos(lcsSoFar.size()+1, unordered_map<string, int>()); // tokens at each position, <token: #>
	vector<vector<int>> lcsPosInLogs; // positions of each lcs element in each log stmt of inGroup.
	for (auto &rk : inGroup) { // rk type: vector<string>, one log stmt
		Utils::rmSpaceAndStars(rk.key);
#ifdef PRINT_CL_SG
		cout<<"this log: "<<endl;
		Utils::dumpVecStr(rk.key);
#endif
		vector<int> newlcsPos;
		uint pos = 0;
		string posVal = "";
		for (uint i=0; i<rk.key.size(); i++) {
			if (pos<lcsSoFar.size() && rk.key[i]==lcsSoFar[pos]) {
				newlcsPos.push_back(i);
				if (tksEachPos[pos].find(posVal)!=tksEachPos[pos].end())
					tksEachPos[pos][posVal]++;
				else
					tksEachPos[pos][posVal] = 1;
				pos++;
				posVal = "";
			}
			else {
				posVal += ' '+rk.key[i];
			}
		}
		if (tksEachPos[pos].find(posVal)!=tksEachPos[pos].end())
			tksEachPos[pos][posVal]++;
		else
			tksEachPos[pos][posVal] = 1;
		lcsPosInLogs.push_back(newlcsPos);
	}
#ifdef PRINT_CL_SG
	cout <<"dump tksEachPos: "<<endl;
	Utils::dumpVecMapStrInt(tksEachPos);
#endif
	// count distinct # of values in each position, select the minimum number to split;
	// if multiple minimum numbers, compute entropy and select the smaller pos to split;
	// if same entropy, select the most left pos to split.
	int smallest = m_groupSplitV;
	vector<int> smallestPos;
	for (uint i=0; i<tksEachPos.size(); i++) {
		int cnt = tksEachPos[i].size();
		if (cnt>=m_groupSplitV) {
			replaceParams(inGroup, lcsPosInLogs, i-1);
		}
		else if (smallest==cnt)
			smallestPos.push_back(i);
		else if (smallest>cnt && cnt>1) {
			smallest = cnt;
			smallestPos.clear();
			smallestPos.push_back(i);
		}
	}
	uint splitPos = 0;
	if (smallestPos.size()==0) {
		outGroups.clear();
		return;
	}
	else if (smallestPos.size()==1) splitPos = smallestPos[0];
	else {
		double smEn = numeric_limits<double>::max();
		for (uint i=0; i<smallestPos.size(); i++) {
			double en = getEntropy(tksEachPos[smallestPos[i]]);
			assert(en>0);
			if (smEn> en) { // no equal sign ensures if multiple pos has same entropy, choose the most left one
				smEn = en;
				splitPos = smallestPos[i];
			}
		}
	}
	// Now having splitPos, split according to this pos
	vector<vector<string>> toSplit;
	for (auto it=tksEachPos[splitPos].begin(); it!=tksEachPos[splitPos].end(); it++) {
		vector<string> newPs;
		Utils::getTokens(it->first, newPs);
		Utils::rmSpaceAndStars(newPs);
#ifdef PRINT_CL_SG
		printf("newPs:\n");
		Utils::dumpVecStr(newPs);
#endif
		toSplit.push_back(newPs);
	}
	for (uint i=0; i<toSplit.size(); i++) {
		vector<ikPair> newOut;
		outGroups.push_back(newOut);
	}
#ifdef PRINT_CL_SG
	printf("dump lcsPosInLogs\n");
	Utils::dumpVecVecInt(lcsPosInLogs);
#endif
	int start=-1, end=-1;
	for (uint i=0; i<inGroup.size(); i++) {
		start = splitPos==0? 0 : lcsPosInLogs[i][splitPos-1]+1;
		end = (splitPos==lcsSoFar.size())? inGroup[i].key.size() : lcsPosInLogs[i][splitPos];
		for (uint j=0; j<toSplit.size(); j++) {
#ifdef PRINT_CL_SG
			printf("inGroup[i]: \n");
			Utils::dumpVecStr(inGroup[i].key);
			printf("toSplit[j]: \n");
			Utils::dumpVecStr(toSplit[j]);
#endif
			if (end-start==toSplit[j].size() &&
					equal(inGroup[i].key.begin()+start, inGroup[i].key.begin()+end, toSplit[j].begin())) {
				outGroups[j].push_back(inGroup[i]);
				break;
			}
		}
	}
}

double Clustering::getEntropy(unordered_map<string, int> &tksThisPos) {
	double ret = 0;
	int total = 0;
	for (auto it=tksThisPos.begin(); it!=tksThisPos.end(); it++) {
		total += it->second;
	}
	for (auto it=tksThisPos.begin(); it!=tksThisPos.end(); it++) {
		double perc = (double)it->second/total;
		ret += -1 * perc * log (perc);
	}
	return ret;
}

void Clustering::replaceParams(vector<ikPair> &inGroup, vector<vector<int>> &lcsPosInLogs,
		int startPos) {
	int endPos = startPos+1;
	for (uint j=0; j<inGroup.size(); j++) {
		vector<string> &rk = inGroup[j].key;
		vector<int> &pos = lcsPosInLogs[j];
//		Utils::dumpVecInt(pos);
		for (uint i=0; i<rk.size(); i++) {
			if ( (startPos<0 && i<(uint)pos[endPos]) ||
					((uint)endPos==lcsPosInLogs.size() && i>(uint)lcsPosInLogs[j][startPos]) ||
					(i>(uint)lcsPosInLogs[j][startPos] && i<(uint)lcsPosInLogs[j][endPos]) ) {
				rk[i] = '*';
			}
		}
		for (auto it=rk.begin(); it!=rk.end(); ) { // erase adjacent '*'
			if (it!=rk.begin() && *it=="*" && *(it-1)=="*") rk.erase(it);
			else it++;
		}
	}
}

void Clustering::getDists() {
	double newDist;
	for (uint i=0; i<m_rks.size()-1; i++) {
		for (uint j=i+1; j<m_rks.size(); j++) {
			newDist = getWeiDist(m_rks[i], m_rks[j]);
//			m_pairDists.push_back(newDist);
			string key = to_string(i)+'-'+to_string(j);
			m_distMap.insert({key, newDist});
		}
	}
#ifdef PRINT_CL
	cout <<"dump m_distMap: "<<endl;
	for (auto it=m_distMap.begin(); it!=m_distMap.end(); it++) {
		cout << it->first<<": "<<it->second<<endl;
	}
#endif
}

void Clustering::parClusters2() {
	for (uint i=0; i<m_rks.size(); i++) {
		uint ci=0;
		for (; ci<m_logIdKeyClusters.size(); ci++) {
			// compare with the first log in each cluster to see if they are within one cluster
			if (i==(uint)m_logIdKeyClusters[ci][0].id) continue;
			int j = m_logIdKeyClusters[ci][0].id;
			double dist = getWeiDist(m_rks[i], m_rks[j]); //m_distMap[key];
#ifdef PRINT_CL
//			cout <<"key: "<<key<<", dist: "<<dist<<endl;
#endif
			if (dist<=m_initClusterV) {
				m_logIdKeyClusters[ci].push_back(idKeyPair(i, m_rks[i]));
				break;
			}
		}
		if (ci==m_logIdKeyClusters.size()) { // not found, init a new cluster
			m_logIdKeyClusters.push_back(vector<idKeyPair>(1, idKeyPair(i, m_rks[i])));
		}
	}
}

void Clustering::parClusters() {
	for (uint i=0; i<m_rks.size(); i++) {
		uint ci=0;
		for (; ci<m_logIdKeyClusters.size(); ci++) {
			// compare with the first log in each cluster to see if they are within one cluster
			if (i==(uint)m_logIdKeyClusters[ci][0].id) continue;
			int mmin, mmax;
			if (i<(uint)m_logIdKeyClusters[ci][0].id) {
				mmin = i; mmax = m_logIdKeyClusters[ci][0].id;
			}
			else {
				mmax = i; mmin = m_logIdKeyClusters[ci][0].id;
			}
			string key = to_string(mmin)+'-'+to_string(mmax);
			double dist = m_distMap[key];
#ifdef PRINT_CL
			cout <<"key: "<<key<<", dist: "<<dist<<endl;
#endif
			if (dist<=m_initClusterV) {
				m_logIdKeyClusters[ci].push_back(idKeyPair(i, m_rks[i]));
				break;
			}
		}
		if (ci==m_logIdKeyClusters.size()) { // not found, init a new cluster
			m_logIdKeyClusters.push_back(vector<idKeyPair>(1, idKeyPair(i, m_rks[i])));
		}
	}
}

// get the threshold m_classThres by a one-dim kMeans algorithm
void Clustering::getClassThres() {
	unordered_map<string, int> clIds; // which cluster each distPair belongs to
	for (auto it=m_distMap.begin(); it!=m_distMap.end(); it++) {
		clIds[it->first] = 0;
	}
	double center1=INT_MAX, center2=INT_MIN;
	for (auto it=m_distMap.begin(); it!=m_distMap.end(); it++) {
		if (it->second<center1) center1 = it->second;
		if (it->second>center2) center2 = it->second;
	}
	double sum1=0, sum2=0;
	double cnt1=0, cnt2=0;
	bool bChanging = true;
	int newId = 0;
	double d1, d2;
	int iterCnt = 0;
	while (bChanging && iterCnt<20) {
		bChanging = false;
		iterCnt++;
		sum1=0; sum2=0;
		cnt1=0; cnt2=0;
		for (auto it=m_distMap.begin(); it!=m_distMap.end(); it++) { // cluster
#ifdef PRINT_CL
			cout <<it->first<<": "<<it->second<<endl;
#endif
			d1 = abs(it->second-center1);
			d2 = abs(center2-it->second);
			if (d1<=d2) {
				newId = 1;
				sum1 += it->second;
				cnt1++;
			}
			else {
				newId = 2;
				sum2 += it->second;
				cnt2++;
			}
			if (newId != clIds[it->first]) {
				clIds[it->first] = newId;
				bChanging = true;
			}
		}
		center1 = sum1/cnt1; // re-center
		center2 = sum2/cnt2;
	}
	m_initClusterV = 0;
	for (auto it=m_distMap.begin(); it!=m_distMap.end(); it++) {
#ifdef PRINT_CL
		cout <<it->first<<", cl id: "<<clIds[it->first]<<", dist: "<<it->second<<endl;
#endif
		if (clIds[it->first]==1 && m_initClusterV<it->second)
			m_initClusterV = it->second;
	}
	cout << "initial cluster threshold: "<<m_initClusterV<<endl;
}

double Clustering::getWeiDist(vector<string> &rk1, vector<string> &rk2) {
	double ret = 0.0;
	vector<int> wdIds;
	calcEditDist(rk1, rk2, wdIds);
	for (uint i=0; i<wdIds.size(); i++) {
		double tmp = 1 + exp(wdIds[i]-m_weidistV);
		ret += (1.0/tmp);
	}
	return ret;
}

void Clustering::calcEditDist(vector<string> &rk1, vector<string> &rk2, vector<int> &retIds) {
	uint sz1 = rk1.size(), sz2 = rk2.size();
	int **editDist;
	Utils::alloc2dIntArray(sz1, sz2, editDist);
	editDist[0][0] = 0;
	for (uint i=1; i<sz1; i++)
		editDist[i][0] = i;
	for (uint j=1; j<sz2; j++)
		editDist[0][j] = j;
	for (uint i=1; i<sz1; i++)
		for (uint j=1; j<sz2; j++) {
			int rep = editDist[i-1][j-1] + (rk1[i]==rk2[j]? 0:1);
			editDist[i][j] = min(rep, min(editDist[i-1][j]+1, editDist[i][j-1]+1));
		}
#ifdef PRINT_CL_ED
	cout << "print edit distances: "<<endl;
	for (uint i=0; i<sz1; i++)
		for (uint j=0; j<sz2; j++)
			cout << "i: "<<i << ", j: "<<j << ", dist: "<<editDist[i][j]<<endl;
#endif
	btEditIds(rk1, rk2, editDist, sz1-1, sz2-1, retIds);
	Utils::dealloc2dIntArray(sz1, editDist);
#ifdef PRINT_CL_ED
	cout << "print ret ids: "<<endl;
	for (uint i=0; i<retIds.size(); i++)
		cout << retIds[i]<<" ";
	cout<<endl;
#endif
}

void Clustering::btEditIds(vector<string> &rk1, vector<string> &rk2, int** editDist,
		int i, int j, vector<int> &retIds) {
	if (i==0 && j==0) {
		if (rk1[0]!=rk2[0]) {
			retIds.push_back(0);
			retIds.push_back(0);
		}
	}
	else if (i==0) {
		bool flag = false;
		for (int rr=0; rr<j; rr++) {
			if (rk1[0]!=rk2[rr]) retIds.push_back(rr);
			else if (!flag) flag = true;
			else retIds.push_back(rr);
		}
		if (!flag) retIds.push_back(0);
	}
	else if (j==0) {
		bool flag = false;
		for (int rr=0; rr<i; rr++) {
			if (rk1[rr]!=rk2[0]) retIds.push_back(rr);
			else if (!flag) flag = true;
			else retIds.push_back(rr);
		}
		if (!flag) retIds.push_back(0);
	}
	if (i==0 || j==0) return;
	int rep = editDist[i-1][j-1] + (rk1[i]==rk2[j]? 0:1);
	int left = editDist[i-1][j]+1;
	int right = editDist[i][j-1]+1;
	int last = min(rep, min(left, right));
	if (last==rep) {
		if (rk1[i]!=rk2[j]) {
			retIds.push_back(i);
			retIds.push_back(j);
		}
		btEditIds(rk1, rk2, editDist, i-1, j-1, retIds);
	}
	else if (last==left) {
		retIds.push_back(j);
		btEditIds(rk1, rk2, editDist, i-1, j, retIds);
	}
	else {
		retIds.push_back(i);
		btEditIds(rk1, rk2, editDist, i, j-1, retIds);
	}
}
