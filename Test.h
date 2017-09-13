/*
 * Test.h
 *
 *  Created on: Dec 20, 2015
 *      Author: mind
 */

#ifndef TEST_H_
#define TEST_H_

#include "lcs.h"
#include "Clustering.h"
#include "IPLoM.h"
#include "logTrie.h"

class Test {
public:
	Test();
	virtual ~Test();
	LCSParser m_lcs;
	Clustering m_clustering;
	IPLoM m_iplom;

	void test_clustering();
	void test_lcs();
	void test_lcs2();
	void test_clustering_ed();
	void test_iplom();
	void test_lcs_infloop();
	int test_logTrie();
	int test_logTrie2();
	void logTrie_add(const string&, logTrie* trie);
	void logTrie_delete(const string&, logTrie* trie);
};

#endif /* TEST_H_ */
