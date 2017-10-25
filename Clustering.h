/*
 * Clustering.h
 *
 *  Created on: Dec 16, 2015
 *      Author: mind
 */

#ifndef CLUSTERING_H_
#define CLUSTERING_H_

#include "common.h"

/*
 * Questions:
 * Q1. x_i in weighted edit distance: "the index of the word that is operated by the i_th operation OA_i"
 * index of rk1 or rk2?
 * For now, consider both index in rk1 and rk2.
 * If only consider index in rk1: rk1: "", rk2: "File finished at time t1", indices list is empty;
 * If only consider index in rk2: indices list could be empty for the same reason above;
 * If consider both rk1 and rk2: case1: "File 1 finished." and "File 2 finished." index list "1, 1";
 * case2: "File finished." and "File 2 finished." index list "1". While the case1 seems more like a log key.
 * For now, consider both indices in rk1 and rk2.
 * Q2. How to decide m_weidistV?
 * Q3. How to get "common word sequence of the raw log keys" in "Group splitting"? For now, use LCS on each log.
 */

class Clustering {
	friend class Test;

public:
	Clustering();
	virtual ~Clustering();
	void runClustering(vector<string> &oriLogs, int logType);

private:
	typedef struct idKeyPair {
		idKeyPair(int oi, vector<string> &ok) {id=oi; key=ok;}
		int id;
		vector<string> key;
	} ikPair;
	double m_weidistV=10; // parameter to calc weighted edit distance
	double m_initClusterV=5; // auto-determined using all pair-dist and k-means
	double m_groupSplitV=4;
//	vector<double> m_pairDists;
	// initial clusters after weighted edit distance clustering
//	vector<vector<int>> m_logIdClusters;
//	vector<vector<vector<string>>> m_logKeyClusters;
	vector<vector<ikPair>> m_logIdKeyClusters;
	unordered_map<string, vector<int>> m_finalClusters;
	unordered_map<string, double> m_distMap;
	vector<vector<string>> m_rks; // raw log tokens splited from raw log lines
	vector<string> m_logTimeStamp;
	// WED(rk1, rk2) = \sum_{i=1}^{EO}{\frac{1}{1+e^{x_i-v}}},
	// EO is # of ops changing from rk1 to rk2.
	// x_i is the index of the word operated by the i_th operation.
	// v is a parameter, m_weidistV.
	double getWeiDist(vector<string> &rk1, vector<string> &rk2);
	// caculate edit distance of rk1 and rk2, return a list of indices in rk1 being edited (to become rk2).
	void calcEditDist(vector<string> &rk1, vector<string> &rk2, vector<int> &retIds);
	// called by calcEditDist, backtracking to reversely find edit indices in rk1
	void btEditIds(vector<string> &rk1, vector<string> &rk2, int** editDist,
			int i, int j, vector<int> &retIds);
	// get the threshold m_classThres by a one-dim kMeans algorithm
	void getClassThres();
	void parClusters();
	void parClusters2();
	void getDists();
	void initGroupFixedThres();
	void initGroupAutoThres();
	// recursively split until each group has distinct log keys
	void splitGroups(vector<ikPair> &inGroup, vector<vector<ikPair>> &ioGroups);
	void replaceParams(vector<ikPair> &inGroup, vector<vector<int>> &lcsPosInLogs, int startPos);
	double getEntropy(unordered_map<string, int> &tksThisPos);
	void addToFinal(vector<ikPair> &cluster);
	void dumpClusters();
	void dumpOneCluster(vector<ikPair> &);
	void dumpOneClusterIds(vector<ikPair> &clToDump);
};

#endif /* CLUSTERING_H_ */
