#ifndef LOGTRIE_H_
#define LOGTRIE_H_

#include "common.h"
using namespace std;

// reference: http://www.sourcetricks.com/2011/06/c-tries.html#.VtNkxZc2w_s

// each node is a word in currently parsed message type
class Node {
public:
    Node() { m_marker = -1; }
    ~Node() {}
//    string content() { return m_Content; }
//    void setContent(string c) { m_Content = c; }
//    int wordMarker() { return m_marker; } // indicate end of a log msg
//    void setWordMarker(int marker) { m_marker = marker; }
    Node* findChild(string &c);
    void addChild(string &chdStr, Node* chdPtr) { m_children[chdStr] = chdPtr; }
//    unordered_map<string, Node*> children() { return m_Children; }
//    bool isLeafNode() {return m_Marker;}
//    void unmarkLeaf() {m_Marker = false;}

//private:
//    string m_Content; // no need for content because it's stored in parent node
    int m_marker; // -1 if not leaf, otherwise the index in m_LCSMap vector
    unordered_map<string, Node*> m_children; // a child word : pointer to it
};

class logTrie {
public:
    logTrie();
    ~logTrie();
    void addLCS(vector<string> &s, int idInLCSMap);
    Node * exactSearchLCS(vector<string> &s);
    // e.g. incoming "file f1 completed" matches "file * completed"
    // matched is the longest matching LCS,
    // return m_marker if # of matched words with s is longer than half of s. -1 if no appropriate match
    int fuzzySearchLCS(vector<string> &s, vector<string> &mS, int forDebug);
    int fuzzySearchLCS0(vector<string> &s, vector<string> &mS, int forDebug); // original
    int fuzzySearchLCS1(vector<string> &s, vector<string> &mS, int forDebug);
    int fuzzySearchLCS2(vector<string> &s, vector<string> &mS, int forDebug);
    void deleteLCS(vector<string> &s);
    void dumpTrie();
    void setMarker(vector<string> &lcs, int oldMarker, int newMarker);

private:
    Node* m_root;
    bool deleteHelper(vector<string> &s, Node* pNode, int level, int len);
    bool isFreeNode(Node* pNode); // no children
    void dumpHelper(Node* pNode, int level);
    bool chdWild(Node *pNode); // check if no children or children has "*"
};

# endif /* LOGTRIE_H_ */
