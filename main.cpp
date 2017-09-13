/*
 * main.cpp
 *
 *  Created on: Dec 15, 2015
 *      Author: mind
 */
#include "lcs.h"
#include "Clustering.h"
#include "IPLoM.h"
#include "Test.h"

void run_lcs(vector<string> &oriLogs, int logType, int smOption, int prec) { // last option: precheck type
  // lcs parser
	LCSParser parser;
	parser.m_bSplit = (smOption & 0x01) ? true:false;
	parser.m_bMerge = (smOption & 0x02) ? true:false;
	parser.runLCS(oriLogs, logType, prec);
}

void run_clustering(vector<string> &oriLogs, int logType) {
	Clustering cl;
	cl.runClustering(oriLogs, logType);
}

void run_iplom(vector<string> &oriLogs, int logType) {
	IPLoM ip;
	ip.runIPLoM(oriLogs, logType);
}
enum {prefix=0, invertedList=1, loop=2, hashed=3};

// usage: ./logParser run_lcs_basic logFile logType
int main(int argc, char* argv[]) {
	string fileName = "data/LAUR060803MX20NODES0TO255EVENTS.csv";

	string type = "run_lcs_basic";

	if (argc>1 && (strcmp(argv[1],"-h")==0 || strcmp(argv[1],"--help")==0)) {
		cout<< "usage: ./logParser run_method logFileName(Path) logType lcsOptimization" << endl;
		cout<< "run_method: "<<endl;
		cout<<"\t run_lcs_basic, run_lcs_split, run_lcs_split_merge, run_clustering, run_iplom" << endl;
		cout<< "logFileName: "<<endl;
		cout<<"\t file/path of the log file" <<endl;
		cout<< "logType: "<<endl;
		cout<<"\t currently support: losAlmos, bgl, openstack, spark" << endl;
		cout<< "lcsOptimization (recommended: loop): "<<endl;
		cout<<"\t data structure to speed up lcs method, prefix, invertedList, loop, hash"<<endl;
		return -1;
	}

	if (argc>1) {
		type = argv[1];
	}
	if (argc>2) {
		fileName = argv[2];
	}
	int logType=0;
	if (argc>3) {
//		cout<<argv[3]<<endl;
		if (strcmp(argv[3],"losAlmos")==0)
			logType = 0;
		else if (strcmp(argv[3],"bgl")==0)
			logType = 1;
		else if (strcmp(argv[3], "openstack")==0)
			logType = 2;
		else if (strcmp(argv[3], "spark")==0)
			logType = 3;
	}
	int prec=-1;
	if (argc>4) {
		if (strcmp(argv[4], "prefix")==0)
			prec = prefix;
		else if (strcmp(argv[4], "invertedList")==0)
			prec = invertedList;
		else if (strcmp(argv[4], "loop")==0)
			prec = loop;
		else if (strcmp(argv[4], "hash")==0)
			prec = hashed;
	}

	cout<<"method: "<<type<<endl;
	cout<<"fileName: "<<fileName<<endl;
	cout<<"log type: "<<logType<<endl;
	vector<string> oriLogs;
	Utils::readFile(fileName, oriLogs);

	if (type=="run_lcs_basic")
		run_lcs(oriLogs, logType, 0, prec);
	if (type=="run_lcs_split")
		run_lcs(oriLogs, logType, 1, prec);
	if (type=="run_lcs_split_merge")
		run_lcs(oriLogs, logType, 3, prec);
	if (type=="run_clustering") {
		run_clustering(oriLogs, logType);
	}
	if (type=="run_iplom") {
		run_iplom(oriLogs,  logType);
	}

	Test tt;
	if (type=="test_lcs")
		tt.test_lcs2();
	if (type=="test_clustering")
		tt.test_clustering();
	if (type=="test_iplom")
		tt.test_iplom();
	if (type=="test_logTrie")
		tt.test_logTrie2();

}
