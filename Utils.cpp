/*
 * utils.cpp
 *
 *  Created on: Dec 15, 2015
 *      Author: mind
 */

#include "Utils.h"

Utils::Utils() {
	// TODO Auto-generated constructor stub

}

Utils::~Utils() {
	// TODO Auto-generated destructor stub
}

void Utils::readFile(string &fileName, vector<string> &retLogs) {
	namespace fs = boost::filesystem;
	if (!fs::exists(fileName)) {
		cout << "file "+fileName+" does not exist"<<endl;
		return;
	}
	string line;
	fs::path filePath(fileName);

	string tmpFN = "allLogsInside";

	if (fs::is_directory(filePath)) {
		tmpFN = fileName+"/"+tmpFN;
		string sysCmd = "rm "+tmpFN;
		system(sysCmd.c_str());
	    fs::directory_iterator end_iter;
	    for (fs::directory_iterator dir_itr(filePath); dir_itr != end_iter; ++dir_itr)
	    {
	      try
	      {
	    	  if (fs::is_regular_file(dir_itr->status()))
	    	  {
//	    		  std::cout << dir_itr->path().filename() << "\n";
	    		  string logFile = fileName+"/"+dir_itr->path().filename().string();
	    		  sysCmd = "echo \'fileName: "+logFile + "\' >> "+tmpFN;
	    		  system(sysCmd.c_str());
	    		  sysCmd = "cat "+logFile + " >> "+tmpFN;
	    		  system(sysCmd.c_str());
//	    		  fileNames.push_back(fileName+"/"+dir_itr->path().filename().string());
	    	  }
	      }
	      catch (const std::exception & ex)
	      {
	        std::cout << fileName+"/" + dir_itr->path().filename().string() << " " << ex.what() << std::endl;
	      }
	    }
	    fileName = tmpFN;
	}
	//cout<<"final log file name: "<<fileName<<endl;
	ifstream logFile(fileName);
	if (logFile.is_open()) {
		while (getline(logFile, line)) {
			// process line
//			string retLine;
//			Utils::rmSchemas(line, retLine);
//			cout<< retLine << endl;
			retLogs.push_back(line);
		}
		logFile.close();
	}
	else {
		cout<<"CANNOT open file: "+fileName<<endl;
	}
	//cout<<"log entry count in total: "<<retLogs.size()<<endl;
	return;
}

void Utils::getTokens(const string &line, vector<string> &logTokens) {
	boost::split(logTokens, line, boost::is_any_of("\t "),boost::token_compress_on);
}

void Utils::tokensToStr(const vector<string> &logTokens, string &line) {
	line = boost::algorithm::join(logTokens, " ");
}

void Utils::rmSchemas(const string &line, string &retLine, int nIgnored) {
	vector<string> logTokens;
	boost::split(logTokens, line, boost::is_any_of("\t =")/*,boost::token_compress_on*/);
	if (logTokens.size()>nIgnored)
		logTokens.erase(logTokens.begin(), logTokens.begin()+nIgnored);
	retLine = boost::algorithm::join(logTokens, " ");
}

bool Utils::preHPCLogs(string &line, vector<string> &logTokens, int nIGNORED, string &logTimeStamp) {
//	const int nIGNORED = 6;
//	cout<<line<<endl;

/*		// remove schemas, boost way
	boost::split(logTokens, line, boost::is_any_of(","),boost::token_compress_on);

	if (logTokens.size()>nIGNORED)
		logTokens.erase(logTokens.begin(), logTokens.begin()+6);
//	dumpVecStr(logTokens);
	line = boost::algorithm::join(logTokens, ",");
*/

	// remove schemas, iterate way. faster than boost way
	uint i = 0, cnt=0;
	string ordinalNumber = "";
	for (; i<line.size(); i++) {
		if (line[i]==',') cnt++;
		if (cnt >= 0 && cnt<1) ordinalNumber += line[i];
		if (cnt >=5) break;
		if (cnt>=4) logTimeStamp += line[i];
	} 
	logTimeStamp.erase(0, 1);
	logTimeStamp += "||"+ ordinalNumber;
	for (; i<line.size(); i++) {
		if (line[i]==',') cnt++;
		if (cnt>=nIGNORED) break;
	}
	line.erase(0, i+1);
//	cout<<line<<endl;
	boost::split(logTokens, line, boost::is_any_of("\t =")/*,boost::token_compress_on*/);
	return true;
//	dumpVecStr(logTokens);
}

bool Utils::preSparkLogs(string &line, vector<string> &logTokens, int nIGNORED) {
	boost::split(logTokens, line, boost::is_any_of("\t =")/*,boost::token_compress_on*/);
	if (logTokens[0]=="fileName:") return false;
	logTokens.erase(logTokens.begin(), logTokens.begin()+nIGNORED);
	return true;
}

void Utils::preSysLogs(const string &line, vector<string> &logTokens) {
	boost::split(logTokens, line, boost::is_any_of("\t ")/*,boost::token_compress_on*/);
	if (logTokens.size()>6)
		logTokens.erase(logTokens.begin(), logTokens.begin()+6);
}

bool Utils::preProcessLog(string &line, vector<string> &logTokens, int logType, string &logTimeStamp) {
	if (logType==0)
		return preHPCLogs(line, logTokens, 6, logTimeStamp);
	else if (logType==1)
		return preBglLogs(line, logTokens, 0, logTimeStamp);
	else if (logType==2)
		return preOpenstackLogs(line, logTokens, 0, logTimeStamp);
	else if (logType==3)
		return preSparkLogs(line, logTokens, 2);
	else
		assert(0);
	return false;
}

bool Utils::preBglLogs(string &line, vector<string> &logTokens, int nIgnored, string &logTimeStamp) {
	boost::split(logTokens, line, boost::is_any_of("\t ="));
	const string myints[]= {"INFO", "FATAL", "ERROR", "WARNING", "SEVERE", "FAILURE"};
	std::set<string> LOGLEVEL (myints,myints+6);
	uint i=0, cnt=0;
	for (; i<logTokens.size(); i++) {
		if (LOGLEVEL.find(logTokens[i]) != LOGLEVEL.end()) {
			break;
		}
	}
	if (i==logTokens.size()) {
//		printf("WARNING! Failed to find log level\n");
		return false;
	}
	logTimeStamp = logTokens[4];
	logTokens.erase(logTokens.begin(), logTokens.begin()+i+1);
	return true;

//	rmSchemas(line, retLog, nIgnored);
//	getTokens(retLog, logTokens);
}

bool Utils::preOpenstackLogs(string &line, vector<string> &logTokens, int nIgnored, string &logTimeStamp) {
	boost::split(logTokens, line, boost::is_any_of("\t "));
	const string myints[]= {"DEBUG", "INFO", "AUDIT", "WARNING", "ERROR", "CRITICAL", "TRACE"};
	std::set<string> LOGLEVEL (myints,myints+6);
	uint i=0, cnt=0;
	for (; i<logTokens.size(); i++) {
		if (LOGLEVEL.find(logTokens[i]) != LOGLEVEL.end()) {
			break;
		}
	}
	if (i==logTokens.size()) {
//		printf("WARNING! Failed to find log level\n");
		return false;
	}
	logTimeStamp = logTokens[0] + "||" + logTokens[1];
	logTokens.erase(logTokens.begin(), logTokens.begin()+i+1);
	return true;
//	rmSchemas(line, retLog, nIgnored);
//	getTokens(retLog, logTokens);
}

int Utils::getHashValOfVecStr(vector<string> &inVec) {
    unsigned long seed = 0;
    for (auto &ele : inVec)
    	boost::hash_combine(seed, ele);

    return seed;
}

void Utils::alloc2dIntArray(int m, int n, int** &array) {
	array = new int*[m];
	for (int i=0; i<m; i++)
		array[i] = new int[n];
}

void Utils::dealloc2dIntArray(int m, int** &array) {
	for (int i=0; i<m; i++) {
		delete [] array[i];
		array[i] = NULL;
	}
	delete [] array;
	array = NULL;
}

//this function is add to return string of vec!!!
string Utils::VectoString(vector<string> &inVec) {
	string s = "";
	for (uint i=0; i<inVec.size(); i++) {
		s = s + inVec[i] + " ";
	}
	return s;
}

void Utils::dumpVecStr(vector<string> &inVec) {
	for (uint i=0; i<inVec.size(); i++) {
		cout << inVec[i] << " ";
	}
	cout<<endl;
}
/*
template <typename A, typename F>
void Utils::dumpVec(vector<T, A> &inVec) {
	printf("size: %d, content:", inVec.size());
	for (uint i=0; i<inVec.size(); i++) {
		cout<<inVec[i]<<" ";
	}
	cout<<endl;
}
*/

void Utils::dumpVecInt(vector<int> &inVec) {
	printf("size: %d\n", inVec.size());
	for (uint i=0; i<inVec.size(); i++) {
		cout<<inVec[i]<<" ";
	}
	cout<<endl;
}
void Utils::dumpSetInt(set<int> &inVec) {
	printf("size: %d\n", inVec.size());
	for (auto it=inVec.begin(); it!=inVec.end(); it++) {
		cout<<*it<<" ";
	}
	cout<<endl;
}

void Utils::dumpVecVecInt(vector<vector<int>> &inGroup) {
	for (uint i=0; i<inGroup.size(); i++) {
		printf("id %d, size %d\n", i, inGroup[i].size());
		for (uint j=0; j<inGroup[i].size(); j++)
			cout <<inGroup[i][j]<<" ";
		cout<<endl;
	}
}

void Utils::dumpVecVecStr(vector<vector<string>> &inGroup) {
	for (uint i=0; i<inGroup.size(); i++) {
		printf("id %d, size %d\n", i, inGroup[i].size());
		for (uint j=0; j<inGroup[i].size(); j++)
			cout <<inGroup[i][j]<<" ";
		cout<<endl;
	}
}

void Utils::dumpVecVecVecStr(vector<vector<vector<string>>> &inGroups) {
	printf("total size: %d\n", inGroups.size());
	for (uint k=0; k<inGroups.size(); k++) {
		printf("Group %d, size %d\n", k, inGroups[k].size());
		for (uint i=0; i<inGroups[k].size(); i++) {
			printf("id %d, size %d\n", i, inGroups[k][i].size());
			for (uint j=0; j<inGroups[k][i].size(); j++)
				cout <<inGroups[k][i][j]<<" ";
			cout<<endl;
		}
	}
}

void Utils::dumpVecMapStrInt(vector<unordered_map<string, int>> &inVec) {
	for (uint i=0; i<inVec.size(); i++) {
		cout<<"map index "<<i<<": "<<endl;
		for (auto it=inVec[i].begin(); it!=inVec[i].end(); it++) {
			cout<<it->first<<": "<<it->second<<endl;
		}
	}
}

void Utils::dumpMapIntVecInt(unordered_map<int, vector<int>> &inMap) {
	for (auto it=inMap.begin(); it!=inMap.end(); it++) {
		cout<<it->first<<":    ";
		for (auto iit=it->second.begin(); iit!=it->second.end(); iit++) {
			cout<<*iit<<" ";
		}
		cout<<endl;
	}
}

void Utils::dumpMapStrVecInt(unordered_map<string, vector<int>> &inMap) {
	for (auto it=inMap.begin(); it!=inMap.end(); it++) {
		cout<<"message: "<<it->first<<endl;
		cout<<"size: "<<it->second.size()<<endl<<"lineIds: ";
		for (auto iit=it->second.begin(); iit!=it->second.end(); iit++) {
			cout<<*iit<<" ";
		}
		cout<<endl;
	}
}

void Utils::dumpMapIntSetStr(unordered_map<int, unordered_set<string>> &inMap) {
	for (auto it=inMap.begin(); it!=inMap.end(); it++) {
		cout<<it->first<<"   ";
//		printf("%s:   ", it->first);
		for (auto iit=it->second.begin(); iit!=it->second.end(); iit++) {
			cout<<*iit<<" ";
//			printf("%s ", *iit);
		}
		printf("\n");
	}
}

void Utils::dumpMapStrSetStr(unordered_map<string, unordered_set<string>> &inMap) {
	for (auto it=inMap.begin(); it!=inMap.end(); it++) {
		cout<<it->first<<"   ";
//		printf("%s:   ", it->first);
		for (auto iit=it->second.begin(); iit!=it->second.end(); iit++) {
			cout<<*iit<<" ";
//			printf("%s ", *iit);
		}
		printf("\n");
	}
}

void Utils::dumpSetVecStr(unordered_set<vector<string>, Utils::VectorHash> &inSetVec) {
	for (auto it=inSetVec.begin(); it!=inSetVec.end(); it++) {
//		printf("%s:   ", it->first);
		for (auto iit=it->begin(); iit!=it->end(); iit++) {
			cout<<*iit<<" ";
//			printf("%s ", *iit);
		}
		printf("\n");
	}
}


void Utils::rmSpaceAndStars(vector<string> &inVec) {
	for (auto it=inVec.begin(); it!=inVec.end(); ) {
//		cout <<"*it: "<<*it<<endl;
		if ((*it)==" " || (*it)=="*" || (*it).size()==0)
			inVec.erase(it);
		else
			it++;
	}
}

bool Utils::equalMapKeys(unordered_map<int, unordered_set<string>> &map1,
		unordered_map<int, unordered_set<string>> &map2) {
	for (auto it=map1.begin(); it!=map1.end(); it++) {
		if (map2.find(it->first) == map2.end())
			return false;
	}
	for (auto it=map2.begin(); it!=map2.end(); it++) {
		if (map1.find(it->first) == map1.end())
			return false;
	}
	return true;
}

bool Utils::is_number(const std::string& s) // count as number if at least one digit appeared.
{
    std::string::const_iterator it = s.begin();
    int numberCnt = 0;
//    cout <<"splitting: "<< s << endl;
    while (it != s.end()) {
    	if (std::isdigit(*it))
    		numberCnt++;
    	if (numberCnt>=1) return true;
    	++it;
    }
    return false;
    //    return !s.empty() && it == s.end();
}


