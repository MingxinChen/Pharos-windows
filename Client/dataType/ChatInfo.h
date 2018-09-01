#pragma once
#include "CommonDefine.h"
#include "PeerList.h"
#include <set>
#include <string>
#include <QDateTime>
using namespace std;

class Graph {
public:
    map<string, int>	node;
    int					edge[MAX_ROOMMATE][MAX_ROOMMATE];
    int					size;

    Graph() {
        size = 0;
    }

    Graph(map<string, char> state) {
        size = state.size();

        map<string, char>::iterator i;
        int j = 0;
        for (i = state.begin(); i != state.end(); i++, j++) {
            node[i->first] = j;
        }

        for (int x = 0; x < size; x++) {
            for (int y = 0; y < size; y++) {
                edge[x][y] = G_UNREACHABLE;
            }
        }
    }

    string find(int num) {
        map<string, int>::iterator i;
        for (i = node.begin(); i != node.end(); i++) {
            if (i->second == num) {
                return i->first;
            }
        }
        return "";
    }
};

class Chat_Info
{
public:
    string					roomId;
    int						roomSize;
    map<string, char>		state;
    string                  headerPath;
    QDateTime               setUpTime;

    bool					online;
    HANDLE                  c_allReplySemaphore;
    string					parent;

    bool					m_forwardAlert;
    MSGDef::TMSG_P2PMSG*	m_forwardMsg;
    string					m_sender;
    bool					m_endNode;
    CRITICAL_SECTION		m_forwardLock;

    bool					w_selectionAlert;
    Graph*                  w_graph;
    bool                    w_newLinkState;

    Chat_Info(char* rid, char* h, set<char*> roommates, int rs, char c, QDateTime st);
	~Chat_Info();

    string getName();
    string getListItemMsg();

	bool AllGuestReply();
	bool DeleteUser(char* user);
    bool SetAllState(char c);
    bool SetState(char* user, char c);

    void setNewGraph();
    void copyEdge(int s_edge[][MAX_ROOMMATE]);
    bool isNewEdge(int s_edge[][MAX_ROOMMATE]);
    void addEdge(char *node1, char *node2, int w);
    bool calculateTree(char *myName);
};

