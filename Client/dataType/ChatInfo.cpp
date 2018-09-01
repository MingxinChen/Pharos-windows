#include "ChatInfo.h"

Chat_Info::Chat_Info(char* rid, char* h, set<char*> roommates, int rs, char c, QDateTime st)
{
    headerPath = "./images/headers/roomHeader.png";
    setUpTime = st;

    parent = string(h);
	roomId = string(rid);
	roomSize = rs;
	online = false;

    m_endNode = true;
    m_forwardAlert = false;

    w_selectionAlert = false;
    w_newLinkState = false;
    w_graph = NULL;

    ::InitializeCriticalSection(&m_forwardLock);

    c_allReplySemaphore = CreateSemaphore(NULL, 0, 1, NULL);

	set<char*>::iterator i;
	for (i = roommates.begin(); i != roommates.end(); i++) {
		string mate = string(*i);
        state[mate] = c;
	}
    if(c == 'O') {
        state[h] = 'A';
    }
}

Chat_Info::~Chat_Info()
{
    ::DeleteCriticalSection(&m_forwardLock);
    CloseHandle(c_allReplySemaphore);
    state.clear();
}

string Chat_Info::getName()
{
    map<string, char>::iterator i = state.begin();
    string namelist = i->first;
    for(++i;i!=state.end();i++) {
        namelist = namelist + ", " + i->first;
    }
    return namelist;
}

string Chat_Info::getListItemMsg()
{
    string msg = "";
    if(online) msg += "[活跃]";
    else msg += "[静默]";
    return msg;
}

bool Chat_Info::AllGuestReply()
{
	map<string, char>::iterator i;
    for (i = state.begin(); i != state.end(); i++) {
        if (i->second == 'O') {
            return false;
        }
	}
    ReleaseSemaphore(c_allReplySemaphore, 1, NULL);
	return true;

}

bool Chat_Info::DeleteUser(char* user)
{
    string key = string(user);
    if (state.find(key) == state.end()) {
        return false;
    }
    state.erase(key);
    roomSize--;
    return true;
}

bool Chat_Info::SetState(char* user, char c)
{
	string key = string(user);
    if (state.find(key) == state.end()) {
		return false;
	}
    state[key] = c;
	return true;
}

bool Chat_Info::SetAllState(char c)
{
    map<string, char>::iterator i;
    for (i = state.begin(); i != state.end(); i++) {
        i->second = c;
    }
    return true;
}

void Chat_Info::setNewGraph()
{
    w_graph = new Graph(state);
}

void  Chat_Info::copyEdge(int s_edge[][MAX_ROOMMATE])
{
    for (int i = 0; i < MAX_ROOMMATE; i++) {
        for (int j = 0; j < MAX_ROOMMATE; j++) {
            if (s_edge[i][j] != G_UNREACHABLE || w_graph->edge[i][j] != G_UNREACHABLE) {
                if (w_graph->edge[i][j] >= s_edge[i][j]) {
                    w_graph->edge[i][j] = s_edge[i][j];
                }
                else {
                    s_edge[i][j] = w_graph->edge[i][j];
                }
            }
            else {
                s_edge[i][j] = G_UNREACHABLE;
            }
        }
    }
}

bool Chat_Info::isNewEdge(int s_edge[][MAX_ROOMMATE])
{
    for (int i = 0; i < MAX_ROOMMATE; i++) {
        for (int j = 0; j < MAX_ROOMMATE; j++) {
            if (s_edge[i][j] != G_UNREACHABLE && w_graph->edge[i][j] == G_UNREACHABLE) {
                return true;
            }
        }
    }
    return false;
}

void Chat_Info::addEdge(char *node1, char *node2, int w)
{
    int a = w_graph->node[string(node1)];
    int b = w_graph->node[string(node2)];

    w_graph->edge[a][b] = w;
    w_graph->edge[b][a] = w;
}

bool Chat_Info::calculateTree(char *myName)
{
    SetAllState('F');
    int myNode = w_graph->node[string(myName)];

    int* lowcost = new int[w_graph->size];
    int* closest = new int[w_graph->size];
    bool* arivial = new bool[w_graph->size];

    int root = -1, k = 0;
    for (int i = 0; i < w_graph->size; i++) {
        arivial[i] = false;

        int nk = 0;
        for (int j = 0; j < w_graph->size; j++) {
            if (w_graph->edge[i][j] == 1) {
                nk++;
            }
        }
        if (nk > k) {
            root = i;
            k = nk;
        }
        if (nk == 0) {
            arivial[i] = true; // 不可达
        }
    }

    arivial[root] = true;

    for (int i = 0; i < w_graph->size; i++) {
        if (arivial[i] == true) {
            closest[i] = -1;
        }
        else {
            lowcost[i] = w_graph->edge[root][i];
            closest[i] = root;
        }
    }

    for (int i = 0; i < w_graph->size; i++)
    {
        int min = G_UNREACHABLE;
        int nextNode = -1;
        for (int k = 0; k < w_graph->size; k++)
        {
            if ((lowcost[k] < min) && (arivial[k] == false))
            {
                min = lowcost[k];
                nextNode = k;
            }
        }

        if (nextNode == -1) {
            break;
        }

        arivial[nextNode] = true;

        for (int k = 0; k < w_graph->size; k++) {
            if ((w_graph->edge[nextNode][k]<lowcost[k]) && (arivial[k] == false)) {
                lowcost[k] = w_graph->edge[nextNode][k];
                closest[k] = nextNode;
            }
        }
    }

    for (int i = 0; i < w_graph->size; i++) {
        if (i != myNode && closest[i] == myNode) {
            state[w_graph->find(i)] = 'T'; // child
            printf("child %s\n", w_graph->find(i).c_str());
            m_endNode = false;
        }
    }
    if (closest[myNode] != -1) {
        state[w_graph->find(closest[myNode])] = 'T'; //parent
        printf("parent %s\n", w_graph->find(closest[myNode]).c_str());
    }

    delete w_graph;
    return true;
}
