/*
 * Test.cpp
 *
 *  Created on: Dec 20, 2015
 *      Author: mind
 */

#include "Test.h"

Test::Test() {
	// TODO Auto-generated constructor stub

}

Test::~Test() {
	// TODO Auto-generated destructor stub
}

void Test::test_iplom() {
	vector<string> ttLogs;

	ttLogs.push_back("Command has completed successfully to test step 1");
	ttLogs.push_back("Command has completed successfully");
	ttLogs.push_back("Command has completed successfully");
	ttLogs.push_back("Targeting domains:node-D3 and nodes:node-127");
	ttLogs.push_back("Targeting domains:node-D7 and nodes:node-230");
	ttLogs.push_back("Command has been aborted");
	ttLogs.push_back("Command has been aborted");
	ttLogs.push_back("Command Failed on: node-D4");
	ttLogs.push_back("Command Failed on: node-D7");
	ttLogs.push_back("Linkerror event interval expired");
	ttLogs.push_back("Linkerror event interval expired");
	ttLogs.push_back("link errors remain current");
	ttLogs.push_back("link errors remain current");
	m_iplom.runIPLoM(ttLogs, 0);

/*
	ttLogs.clear();
	m_iplom.init(); // test step 3.
	ttLogs.push_back("link errors remain current");
	ttLogs.push_back("Command has completed successfully");
	ttLogs.push_back("Command has been aborted");
	ttLogs.push_back("Command has been aborted");
	ttLogs.push_back("Command has been aborted");
	ttLogs.push_back("Command failed on starting");
	m_iplom.runIPLoM(ttLogs);
*/
}

void Test::test_clustering_ed() {
	string s2 = "P U M P K I N";
	string s1 = "P U N T I N G";
	vector<string> rk1, rk2;
	vector<int> retIds;
	Utils::getTokens(s1, rk1);
	Utils::getTokens(s2, rk2);
	m_clustering.calcEditDist(rk1, rk2, retIds);
}

void Test::test_clustering() {
//	test_clustering_ed();
	vector<string> ttLogs;
	ttLogs.push_back("running");
	ttLogs.push_back("down");
	ttLogs.push_back("responding");
	ttLogs.push_back("active");
	ttLogs.push_back("closing");
	m_clustering.runClustering(ttLogs, 0);
}

void Test::test_lcs() {
	vector<string> ttLogs;
	ttLogs.push_back("File 1 completed at time t1");
	ttLogs.push_back("File 2 completed at time t2");
	ttLogs.push_back("File 5 6 completed at time t3");
	for (uint i=0; i<ttLogs.size(); i++) {
		cout<<i<<": log tokens"<<endl;
		cout<<i<<": "<<ttLogs[i]<<endl;
		m_lcs.getLCS(ttLogs[i], 0, -1); // no pre check
	}
	m_lcs.dumpLCSMap();
}

void Test::test_lcs2() {
	vector<string> ttLogs;
	ttLogs.push_back("- 1118989279 2005.06.16 R11-M1-NB-C:J02-U11 2005-06-16-23.21.19.079935 R11-M1-NB-C:J02-U11 RAS KERNEL FATAL a b c d");
	ttLogs.push_back("- 1118989279 2005.06.16 R11-M1-NB-C:J02-U11 2005-06-16-23.21.19.079935 R11-M1-NB-C:J02-U11 RAS KERNEL FATAL e f c d");
	for (uint i=0; i<ttLogs.size(); i++) {
		cout<<i<<": log tokens"<<endl;
		cout<<i<<": "<<ttLogs[i]<<endl;
		m_lcs.getLCS(ttLogs[i], 0, -1);
	}
	m_lcs.dumpLCSMap();
}


void Test::test_lcs_infloop() {
	vector<string> ttLogs;
	//ttLogs.push_back("2587263 node-D0 domain status 1074241800 1 inconsistent nodesets node-1 0x000001fe <ok> node-0 0x000001fe <ok> node-4 0x000001fe <ok> node-2 0x000001fe <ok> node-5 0x000001fe <ok> node-6 0x000001fe <ok> node-7 0x000001fe <ok> node-3 0x000001fe <ok> node-15 0x1001fdfe <ok> node-14 0x1001fdfe <ok> node-12 0x1001fdfe <ok> node-9 0x11fffdfe <ok> node-27 0x11fffdfe <ok> node-13 0x11fffdfe <ok> node-10 0x11fffdfe <ok> node-11 0x11fffdfe <ok>");
	//ttLogs.push_back("2588407 node-D4 domain status 1074242940 1 inconsistent nodesets node-128 0x000001fe <ok> node-129 0x000001fe <ok> node-130 0x0001fdfe <ok> node-132 0x2001fdfe <ok>");


	ttLogs.push_back("2587263 node-D0 domain status 1074241800 1 inconsistent nodesets node-1 0x000001fe <ok> node-0 0x000001fe <ok> node-4 0x000001fe <ok> node-2 0x000001fe <ok> node-5 0x000001fe <ok> node-6 0x000001fe <ok> node-7 0x000001fe <ok> node-3 0x000001fe <ok> node-15 0x1001fdfe <ok> node-14 0x1001fdfe <ok> node-12 0x1001fdfe <ok> node-9 0x11fffdfe <ok> node-27 0x11fffdfe <ok> node-13 0x11fffdfe <ok> node-10 0x11fffdfe <ok>");
	ttLogs.push_back("2588407 node-D4 domain status 1074242940 1 inconsistent nodesets node-128 0x000001fe <ok> node-129 0x000001fe <ok> node-130 0x0001fdfe <ok> node-132 0x2001fdfe <ok>");
	for (uint i=0; i<ttLogs.size(); i++) {
		cout<<i<<": log tokens"<<endl;
		cout<<i<<": "<<ttLogs[i]<<endl;
		m_lcs.getLCS(ttLogs[i], 0, -1);
	}
	m_lcs.dumpLCSMap();

}

void Test::logTrie_add(const string& str1, logTrie* trie) {
	cout<<"add: "<<str1<<endl;
	static int id=0;
	vector<string> tt1;
    Utils::getTokens(str1, tt1);
    trie->addLCS(tt1, id);
    id++;
}

void Test::logTrie_delete(const string& str1, logTrie* trie) {
    cout<<"erase "<<str1<<endl;
    vector<string> tt1;
    Utils::getTokens(str1, tt1);
    trie->deleteLCS(tt1);
}


// Test program
int Test::test_logTrie()
{
    logTrie* trie = new logTrie();
    vector<string> tt, tmpO;

    Utils::getTokens("Command has been aborted", tt);
    if ( trie->fuzzySearchLCS(tt, tmpO, 0) >=0) {
        cout << "Found incorrect" << endl;
        cout << "Search Result: "<<endl;
        Utils::dumpVecStr(tmpO);
    }


    cout<< "test add and search "<<endl;
    logTrie_add("File * completed at time *", trie);
    logTrie_add("Command has completed successfully", trie);
    logTrie_add("Command has been aborted", trie);

    Utils::getTokens("File * completed at time", tt);
    if ( trie->exactSearchLCS(tt) )
        cout << "Found incorrect" << endl;

    Utils::getTokens("File * completed at time *", tt);
    if ( trie->exactSearchLCS(tt) )
        cout << "Found correct" << endl;

    Utils::getTokens("File * completed at time * dummy", tt);
    if ( trie->exactSearchLCS(tt) )
        cout << "Found incorrect" << endl;

    Utils::getTokens("Command has been aborted", tt);
    if ( trie->exactSearchLCS(tt) )
        cout << "Found correct" << endl;
    trie->dumpTrie();

    Utils::getTokens("Command has completed successfully", tt);
    if ( trie->exactSearchLCS(tt) )
        cout << "Found correct" << endl;

    cout<< "test delete "<<endl;
    logTrie_delete("Command has completed successfully", trie);
    trie->dumpTrie();
    if ( trie->exactSearchLCS(tt) )
        cout << "Found incorrect" << endl;
    Utils::getTokens("Command has been aborted", tt);
    if ( trie->exactSearchLCS(tt) )
        cout << "Found correct" << endl;

    cout << "test search with wild card "<< endl;
    Utils::getTokens("File f1 completed at time *", tt);
    if ( trie->fuzzySearchLCS(tt, tmpO, 0) >=0 ) {
        cout << "Found correct" << endl;
        cout << "Search Result: "<<endl;
        Utils::dumpVecStr(tmpO);
    }
    Utils::getTokens("File * completed at time t1", tt);
    if ( trie->fuzzySearchLCS(tt, tmpO, 0) >=0 ) {
        cout << "Found correct" << endl;
        cout << "Search Result: "<<endl;
        Utils::dumpVecStr(tmpO);
    }
    Utils::getTokens("File f1 completed at time t1", tt);
    if ( trie->fuzzySearchLCS(tt, tmpO, 0) >=0 ) {
        cout << "Found correct" << endl;
        cout << "Search Result: "<<endl;
        Utils::dumpVecStr(tmpO);
    }
    Utils::getTokens("File f1, f2 completed at time t1", tt);
    if ( trie->fuzzySearchLCS(tt, tmpO, 0) >=0 ) {
        cout << "Found correct" << endl;
        cout << "Search Result: "<<endl;
        Utils::dumpVecStr(tmpO);
    }
    logTrie_add("Command has completed successfully", trie);
    Utils::getTokens("Command has been aborted", tt);
    if ( trie->fuzzySearchLCS(tt, tmpO, 0) >=0 ) {
        cout << "Found correct" << endl;
        cout << "Search Result: "<<endl;
        Utils::dumpVecStr(tmpO);
    }


    delete trie;
}

// Test program
int Test::test_logTrie2()
{
    logTrie* trie = new logTrie();
    vector<string> tt, tmpO;

    cout<< "test add and search "<<endl;
    logTrie_add("* c d", trie);

    Utils::getTokens("a b c d", tt);
    cout << "test search with wild card "<< endl;
    Utils::getTokens("a b c d", tt);
    if ( trie->fuzzySearchLCS(tt, tmpO, 0) >=0 ) {
        cout << "Found correct" << endl;
        cout << "Search Result: "<<endl;
        Utils::dumpVecStr(tmpO);
    }
    delete trie;
}
