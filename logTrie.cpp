#include "logTrie.h"

Node* Node::findChild(string &c) {
	if (m_children.find(c) != m_children.end()) {
		return m_children[c];
	}
	else return NULL;
}

logTrie::logTrie() {
	m_root = new Node();
}

logTrie::~logTrie() {
    // Free memory
}

void logTrie::setMarker(vector<string> &lcs, int oldMarker, int newMarker) {
#ifdef PRINT_LOGTRIE
	printf("in setMarker, oldMarker: %d, newMarker: %d, lcs:\n", oldMarker, newMarker);
	Utils::dumpVecStr(lcs);
#endif
	Node * leaf = exactSearchLCS(lcs);
	assert(leaf != NULL);
	assert(leaf->m_marker==oldMarker);
	leaf->m_marker = newMarker;
}

void logTrie::addLCS(vector<string> &s, int idInLCSMap) {
    Node* current = m_root;

    if ( s.size() == 0 ) {
    	current->m_marker = idInLCSMap; // an empty word
        return;
    }

    for ( uint i = 0; i < s.size(); i++ ) {
        Node* child = current->findChild(s[i]);
        if ( child != NULL ) {
            current = child;
        }
        else {
            Node* tmp = new Node();
            current->addChild(s[i], tmp);
            current = tmp;
        }
        if ( i == s.size() - 1 )
        	current->m_marker = idInLCSMap;
    }
}

Node * logTrie::exactSearchLCS(vector<string> &s) {
    Node* current = m_root;

    if ( current != NULL ) {
        for ( int i = 0; i < s.size(); i++ ) {
#ifdef PRINT_LOGTRIE
      cout<<"current: "<<s[i]<<endl;
      printf("dump current children:\n");
      for (auto it=current->m_children.begin(); it!=current->m_children.end(); it++)
    	  cout << it->first<<": "<<it->second<<endl;
#endif
            Node* tmp = current->findChild(s[i]);
            if ( tmp == NULL )
                return NULL;
            current = tmp;
        }

        return current;
    }

    return NULL;
}

int logTrie::fuzzySearchLCS(vector<string> &s, vector<string> &mS, int forDebug) {
	fuzzySearchLCS0(s, mS, forDebug);
}
int logTrie::fuzzySearchLCS0(vector<string> &s, vector<string> &mS, int forDebug) {
	if (m_root==NULL) return -1;

	typedef struct tbdLCS_t {
		vector<string> sfLCS; // longest LCS in trie so far
		Node* curPtr; // cur pointer, to find childs
		int nMatched; // how many words in s so far exact matched
		int iS; // which id in S to be matched
	} tbdLCS_t;

	queue<tbdLCS_t> tbd1;
	tbdLCS_t tmp;
	tmp.nMatched=0;
	tmp.curPtr=m_root;
	tmp.iS = 0;
	tbd1.push(tmp);
	//int i=0; // current id in s
	while (tbd1.size()>0) {

		tbdLCS_t &cur = tbd1.front();
		if (cur.iS >= s.size()) break;
		if (cur.curPtr->m_children.find(s[cur.iS]) != cur.curPtr->m_children.end()) {
			// found exact match
			tbdLCS_t nex;
			nex.curPtr = cur.curPtr->m_children[s[cur.iS]];
			nex.iS = cur.iS+1;
			nex.nMatched = cur.nMatched+1;
			nex.sfLCS = cur.sfLCS;
			nex.sfLCS.push_back(s[cur.iS]);
			tbd1.pop();
			tbd1.push(nex);
		}
		else if (cur.curPtr->m_children.find("*") != cur.curPtr->m_children.end()) {
			// wild card in trie, match all
			tbdLCS_t nex;
			nex.curPtr = cur.curPtr->m_children["*"];
			nex.iS = cur.iS+1;
			nex.nMatched = cur.nMatched;
			nex.sfLCS = cur.sfLCS;
			nex.sfLCS.push_back("*");
			tbd1.pop();
			tbd1.push(nex);
		}
		else if (cur.sfLCS.size()>0 && cur.sfLCS.back()=="*") {
			// not match, but last word in sfLCS is "*", advance ptr in S
			tbdLCS_t nex;
			nex.curPtr = cur.curPtr;
			nex.iS = cur.iS+1;
			nex.nMatched = cur.nMatched;
			nex.sfLCS = cur.sfLCS;
			tbd1.pop();
			tbd1.push(nex);
		}
		else { // not match, just remove?
			tbd1.pop();
		}
	}
	if (tbd1.size()>0) { // check current tbd1
		int maxLen=0;
		int marker = -1;
		while (tbd1.size()>0) {
			tbdLCS_t &cur = tbd1.front();
			if (cur.curPtr!=NULL && (chdWild(cur.curPtr) ) )
//				if (cur.nMatched >= 1/2 * s.size() +1 && cur.nMatched>maxLen) {
				if (cur.nMatched >= ceil(1/2 * (double)s.size()) && cur.nMatched>maxLen) {
					mS = cur.sfLCS;
					maxLen = cur.nMatched;
					marker = cur.curPtr->m_marker;
				}
			tbd1.pop();
		}
//		if (maxLen >= 1/2 * s.size() +1) return marker;
		if (maxLen >= (int)ceil((double)s.size()/2)) {
#ifdef PRINT_LOGTRIE
//			printf("found match in logtrie, matched id: %d, matched lcs:\n", marker);
//			Utils::dumpVecStr(mS);
#endif
			return marker;
		}
	}

	return -1;
}
int logTrie::fuzzySearchLCS1(vector<string> &s, vector<string> &mS, int forDebug) {
	if (m_root==NULL) return -1;

	typedef struct tbdLCS_t {
		vector<string> sfLCS; // longest LCS in trie so far
		Node* curPtr; // cur pointer, to find childs
		int nMatched; // how many words in s so far exact matched
		int iS; // which id in S to be matched
	} tbdLCS_t;

	queue<tbdLCS_t> tbd1;
	tbdLCS_t tmp;
	tmp.nMatched=0;
	tmp.curPtr=m_root;
	tmp.iS = 0;
	tbd1.push(tmp);
	//int i=0; // current id in s
	while (tbd1.size()>0) {

		tbdLCS_t &cur = tbd1.front();
		if (cur.iS >= s.size()) break;
		if (cur.curPtr->m_children.find(s[cur.iS]) != cur.curPtr->m_children.end()) {
			// found exact match
			tbdLCS_t nex;
			nex.curPtr = cur.curPtr->m_children[s[cur.iS]];
			nex.iS = cur.iS+1;
			nex.nMatched = cur.nMatched+1;
			nex.sfLCS = cur.sfLCS;
			nex.sfLCS.push_back(s[cur.iS]);
			tbd1.pop();
			tbd1.push(nex);
		}
		else if (cur.curPtr->m_children.find("*") != cur.curPtr->m_children.end()) {
			// wild card in trie, match all
			tbdLCS_t nex;
			nex.curPtr = cur.curPtr->m_children["*"];
			nex.iS = cur.iS+1;
			nex.nMatched = cur.nMatched;
			nex.sfLCS = cur.sfLCS;
			nex.sfLCS.push_back("*");
			tbd1.pop();
			tbd1.push(nex);
		}
		else if (cur.sfLCS.size()>0 && cur.sfLCS.back()=="*") {
			// not match, but last word in sfLCS is "*", advance ptr in S
			tbdLCS_t nex;
			nex.curPtr = cur.curPtr;
			nex.iS = cur.iS+1;
			nex.nMatched = cur.nMatched;
			nex.sfLCS = cur.sfLCS;
			tbd1.pop();
			tbd1.push(nex);
		}
		else { // not match, just remove?
			tbd1.pop();
		}
	}
	if (tbd1.size()>0) { // check current tbd1
		int maxLen=0;
		int marker = -1;
		while (tbd1.size()>0) {
			tbdLCS_t &cur = tbd1.front();
			if (cur.curPtr!=NULL && (chdWild(cur.curPtr) ) )
//				if (cur.nMatched >= 1/2 * s.size() +1 && cur.nMatched>maxLen) {
				if (cur.nMatched >= ceil(1/2 * (double)s.size()) && cur.nMatched>maxLen) {
					mS = cur.sfLCS;
					maxLen = cur.nMatched;
					marker = cur.curPtr->m_marker;
				}
			tbd1.pop();
		}
//		if (maxLen >= 1/2 * s.size() +1) return marker;
		if (maxLen >= (int)ceil((double)s.size()/2)) {
#ifdef PRINT_LOGTRIE
//			printf("found match in logtrie, matched id: %d, matched lcs:\n", marker);
//			Utils::dumpVecStr(mS);
#endif
			return marker;
		}
	}

	return -1;
}

int logTrie::fuzzySearchLCS2(vector<string> &s, vector<string> &mS, int forDebug) {
	if (m_root==NULL) return -1;

	typedef struct tbdLCS_t {
		vector<string> sfLCS; // longest LCS in trie so far
		Node* curPtr; // cur pointer, to find childs
		int nMatched; // how many words in s so far exact matched
		int iS; // which id in S to be matched
	} tbdLCS_t;

	queue<tbdLCS_t> tbd1, finalTbd;
	for (int i=0; i<s.size(); i++) {
		tbdLCS_t tmp;
		tmp.nMatched=0;
		tmp.curPtr=m_root;
		tmp.iS = i;
		tbd1.push(tmp);
	}
	//int i=0; // current id in s
	while (tbd1.size()>0) {

		tbdLCS_t &cur = tbd1.front();
		if (cur.iS >= s.size()) {
			finalTbd.push(cur);
			tbd1.pop();
			continue;
		}

		if (cur.curPtr->m_children.find(s[cur.iS]) != cur.curPtr->m_children.end()) {
			// found exact match
			tbdLCS_t nex;
			nex.curPtr = cur.curPtr->m_children[s[cur.iS]];
			nex.iS = cur.iS+1;
			nex.nMatched = cur.nMatched+1;
			nex.sfLCS = cur.sfLCS;
			nex.sfLCS.push_back(s[cur.iS]);
			tbd1.pop();
			tbd1.push(nex);
		}
		else if (cur.curPtr->m_children.find("*") != cur.curPtr->m_children.end()) {
			// wild card in trie, match all
			tbdLCS_t nex;
			nex.curPtr = cur.curPtr->m_children["*"];
			nex.iS = cur.iS+1;
			nex.nMatched = cur.nMatched;
			nex.sfLCS = cur.sfLCS;
			nex.sfLCS.push_back("*");
			tbd1.pop();
			tbd1.push(nex);
		}
		else if (cur.sfLCS.size()>0 && cur.sfLCS.back()=="*") {
			// not match, but last word in sfLCS is "*", advance ptr in S
			tbdLCS_t nex;
			nex.curPtr = cur.curPtr;
			nex.iS = cur.iS+1;
			nex.nMatched = cur.nMatched;
			nex.sfLCS = cur.sfLCS;
			tbd1.pop();
			tbd1.push(nex);
		}
		else { // not match, just remove?
			tbd1.pop();
		}
	}
	if (tbd1.size()>0) { // check current tbd1
		int maxLen=0;
		int marker = -1;
		while (tbd1.size()>0) {
			tbdLCS_t &cur = tbd1.front();
			if (cur.curPtr!=NULL && (chdWild(cur.curPtr) ) )
//				if (cur.nMatched >= 1/2 * s.size() +1 && cur.nMatched>maxLen) {
				if (cur.nMatched >= ceil(1/2 * (double)s.size()) && cur.nMatched>maxLen) {
					mS = cur.sfLCS;
					maxLen = cur.nMatched;
					marker = cur.curPtr->m_marker;
				}
			tbd1.pop();
		}
//		if (maxLen >= 1/2 * s.size() +1) return marker;
		if (maxLen >= (int)ceil((double)s.size()/2)) {
#ifdef PRINT_LOGTRIE
//			printf("found match in logtrie, matched id: %d, matched lcs:\n", marker);
//			Utils::dumpVecStr(mS);
#endif
			return marker;
		}
	}

	return -1;
}

bool logTrie::chdWild(Node *pNode) {
	assert(pNode!=NULL);
	Node * cur = pNode;
	while (cur) {
		if (cur->m_marker>=0) return true;
		string wd = "*";
		if (cur->findChild(wd)!=NULL) {
			cur = cur->m_children["*"];
		}
		else return false;
	}
	return false;
}


void logTrie::deleteLCS(vector<string> &s) {
	int sz = s.size();
	if (sz>0)
		deleteHelper(s, m_root, 0, sz);
}

// ret true: need to be freed by parent
// ret false: do not need to be freed, either s does not exist in tree or node has other children
bool logTrie::deleteHelper(vector<string> &s, Node* pNode, int level, int len) {

	if (pNode) {
		// arrives leaf for s
		if (level==len) {
			if (pNode->m_marker>=0) { // leaf of s
				pNode->m_marker = -1;
				return isFreeNode(pNode);  // no children, to be freed
			}
		}
		else { // recursive case
			if (deleteHelper(s, pNode->m_children[s[level]], level+1, len)) {
				free(pNode->m_children[s[level]]);
				pNode->m_children[s[level]] = NULL;
				pNode->m_children.erase(s[level]);
				return( pNode->m_marker<0 && isFreeNode(pNode));
			}
		}
	}
	return false; // s not exist in trie
}

bool logTrie::isFreeNode(Node* pNode) {
	if (pNode->m_children.size()>0)
		return false;
	return true;
}

void logTrie::dumpTrie() {
	printf("dump Trie: \n");
	dumpHelper(m_root, 0);
}

void logTrie::dumpHelper(Node* pNode, int level) {
	if (pNode==NULL || pNode->m_children.size()==0) return;
//	printf("level %d, children: \n", level);
	for (auto it=pNode->m_children.begin(); it!=pNode->m_children.end(); it++) {
		cout<<"level: "<<level+1<<": "<< it->first<<" "<<endl;
		dumpHelper(it->second, level+1);
	}
}
