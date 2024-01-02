#include <iostream>
#include <list>
#include <set>
#include <queue>
#include <unordered_map>

struct SetHash {
    std::size_t operator()(const std::set<int>& set) const {
        std::size_t hash = 0;
        for (const int& element : set) {
            hash ^= std::hash<int>{}(element) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

struct SetEqual {
    bool operator()(const std::set<int>& set1, const std::set<int>& set2) const {
        return set1 == set2;
    }
};

class NFA{
private:
    int q,s,a,m,currentState,nextState,accepted,count = 0,trash;
    char symbol;
    char* alphabet;
    int* states;
    std::list<int> DFA_acceptingStates;
    std::queue<std::pair<int,std::pair<char,int>>> queue0,queue2;
    std::queue<std::pair<int,std::pair<char,std::set<int>>>> queue1;
    std::list<std::pair<char, std::set<int>>>* NFA_deltaTable;
    std::list<std::pair<int, std::set<std::pair<char, int>>>> DFA_deltaTable;
    std::set<int>* lambdaClosure;
    std::set<int>* lambdaClosureHold;
    std::pair<int,std::pair<char,int>> holdDelta;
    std::pair<char, std::set<int>> holdColumn;
    std::pair<int,std::pair<char,std::set<int>>> holdRow;
    std::unordered_map<std::set<int>, int, SetHash, SetEqual> hashTable;
    void initiateNFA_deltaTable();
    void initiateDFA_deltaTable();
    void makeDFA();
public:
    NFA(int Q, int S, int A, int M);
    bool check(std::string);
};

NFA::NFA(int Q, int S, int A, int M) {
    q = Q;
    s = S;
    a = A;
    m = M;
    std::set<int>::iterator itSet;
    const int c1 = s;
    alphabet = new char[c1];
    const int c2 = q;
    states = new int[c2];
    NFA_deltaTable = new std::list<std::pair<char, std::set<int>>>[c2];
    lambdaClosure = new std::set<int>[c2];
    lambdaClosureHold = new std::set<int>[c2];
    int i;
    for(i = 0; i < s; i++){
        std::cin >> symbol;
        alphabet[i] = symbol;
    }
    for(i = 0; i < q; i++){
        states[i] = 0;
        while(!NFA_deltaTable[i].empty()) NFA_deltaTable[i].pop_front();
        if(!lambdaClosure[i].empty()) lambdaClosure[i].clear();
        if(!lambdaClosureHold[i].empty()) lambdaClosureHold[i].clear();
        lambdaClosure[i].insert(i);
    }
    while(!DFA_acceptingStates.empty()) DFA_acceptingStates.pop_front();
    std::cin >> trash;
    for(i = 0; i < a; i++){
        std::cin >> accepted;
        states[accepted] = 1;
    }
    if(!hashTable.empty()) hashTable.clear();
    for(i = 0; i < m; i++){
        std::cin >> currentState >> symbol >> nextState;
        if(symbol == '$') {
            lambdaClosure[currentState].insert(nextState);
            lambdaClosureHold[nextState].insert(currentState);
            //for(itSet = )
        }
        else {
            holdDelta.first = currentState;
            holdDelta.second.first = symbol;
            holdDelta.second.second = nextState;
            queue0.push(holdDelta);
        }
    }
    initiateNFA_deltaTable();
    makeDFA();
}

void NFA::initiateNFA_deltaTable() {
    std::set<int> holdSet;
    holdSet.clear();
    std::list<std::pair<char, std::set<int>>>::iterator it1;
    std::set<int>::iterator it2;
    std::cout << "\nlambdaClosure table: \n";
    for(int i = 0; i < q; i++){
        std::cout << '\n' << i << " -> {";
        for(it2 = lambdaClosure[i].begin(); it2 != lambdaClosure[i].end(); it2++) std::cout << " " << *it2 << " ";
        std::cout << "}\n";
    }
    std::cout << "\nprocessing queue0: " << '\n';
    while(!queue0.empty()){
        currentState = queue0.front().first;
        symbol = queue0.front().second.first;
        nextState = queue0.front().second.second;
        std::cout << "\n# "<< currentState << ", " << symbol << ", " << nextState << '\n';
        holdColumn.first = symbol;
        holdColumn.second.clear();
        holdSet.clear();
        holdSet = lambdaClosure[nextState];
        holdColumn.second = holdSet;
        for(it1 = NFA_deltaTable[currentState].begin();it1 != NFA_deltaTable[currentState].end();it1++)
            if (it1->first == symbol) break;

        if(it1 == NFA_deltaTable[currentState].end()) NFA_deltaTable[currentState].push_front(holdColumn);
        else {
            it1->second.merge(holdSet);
        }
        queue0.pop();
    }
    std::cout << '\n' << "NFA table:" << "\n\n";
    for(int i = 0; i < q; i++) {
        std::cout << i << ": || ";
        for (it1 = NFA_deltaTable[i].begin(); it1 != NFA_deltaTable[i].end(); it1++) {
            std::cout << it1->first << " -> {" ;
            for(it2 = it1->second.begin(); it2 != it1->second.end(); it2++) std::cout << " " << *it2 << " ";
            std::cout << "} ||";
        }
        std::cout<< "\n";
    }
}

void NFA::makeDFA() {

    int i, j;
    std::set<int> dfaStart,holdState;
    std::set<int>::iterator itSet,itSet2;
    std::list<std::pair<char,std::set<int>>>::iterator itDelta;
    std::pair<std::set<int>,int> holdHash;
    if(!dfaStart.empty()) dfaStart.clear();
    dfaStart = lambdaClosure[0];
    holdRow.first = -1;
    holdRow.second.first = '\0';
    holdRow.second.second = dfaStart;
    queue1.push(holdRow);
    std::cout << "\nprocessing queue1:" << '\n';
    while(!queue1.empty()){
        std::cout << "\n";
        std::cout<< "#  " << queue1.front().first << ", " << queue1.front().second.first << ", {";
        for(itSet = queue1.front().second.second.begin(); itSet != queue1.front().second.second.end(); itSet++) std::cout << " " << *itSet << " ";
        std::cout << "}\n";
        holdRow = queue1.front();
        if(hashTable.find(holdRow.second.second) == hashTable.end()){
            j = count;
            holdHash.first = holdRow.second.second;
            holdHash.second = count;
            hashTable.insert(holdHash);
            for(itSet = holdHash.first.begin(); itSet != holdHash.first.end(); itSet++)
                if(states[*itSet] == 1) {
                    DFA_acceptingStates.push_front(count);
                    break;
                }
            currentState = holdRow.first;
            symbol = holdRow.second.first;
            holdDelta.first = currentState;
            holdDelta.second.first = symbol;
            holdDelta.second.second = j;
            if(currentState != -1) queue2.push(holdDelta);
            count++;
            for(i = 0; i < s; i++){
                holdState.clear();
                for(itSet = holdHash.first.begin(); itSet != holdHash.first.end(); itSet++) {
                    for(itDelta = NFA_deltaTable[*itSet].begin(); itDelta != NFA_deltaTable[*itSet].end(); itDelta++)
                        if(itDelta->first == alphabet[i]) {
                            holdColumn = *itDelta;
                            holdState.merge(holdColumn.second);
                            break;
                        }
                }
                holdRow.first = j;
                holdRow.second.first = alphabet[i];
                holdRow.second.second = holdState;
                if(!holdState.empty()) queue1.push(holdRow);
            }
            queue1.pop();
        } else {
            j = hashTable.find(holdRow.second.second)->second;
            holdDelta.first = holdRow.first;
            holdDelta.second.first = holdRow.second.first;
            holdDelta.second.second = j;
            queue2.push(holdDelta);
            queue1.pop();
        }
    }
    initiateDFA_deltaTable();
}

void NFA::initiateDFA_deltaTable() {
    std::pair<char, int> holdPair;
    std::set<std::pair<char, int>> holdSet;
    std::set<std::pair<char, int>>::iterator itSet;
    std::pair<int, std::set<std::pair<char, int>>> holdListPair;
    std::list<std::pair<int, std::set<std::pair<char, int>>>>::iterator itList;
    std::cout << "\nprocessing queue2:\n";
    while(!queue2.empty()){
        std::cout<< "\n#  " << queue2.front().first << ", " << queue2.front().second.first << ", " << queue2.front().second.second << "\n";
        holdDelta = queue2.front();
        holdPair.first = holdDelta.second.first;
        holdPair.second = holdDelta.second.second;
        for(itList = DFA_deltaTable.begin(); itList != DFA_deltaTable.end(); itList++)
            if(itList->first == holdDelta.first) {
                itList->second.insert(holdPair);
                break;
            }
        if(itList == DFA_deltaTable.end()){
            holdSet.clear();
            holdSet.insert(holdPair);
            holdListPair.first = holdDelta.first;
            holdListPair.second = holdSet;
            DFA_deltaTable.push_front(holdListPair);
        }
        queue2.pop();
    }
    DFA_deltaTable.sort();
    std::cout << "\n\n DFA table: \n\n";
    for(itList = DFA_deltaTable.begin(); itList != DFA_deltaTable.end(); itList++){
        std::cout << itList->first << ": || ";
        for(itSet = itList->second.begin(); itSet != itList->second.end(); itSet++){
            std::cout << itSet->first << " -> " << itSet->second << " ||";
        }
        std::cout << "\n";
    }
    std::cout << "\nReady to compute!!\n\n\n\n";
}

bool NFA::check(std::string x) {
    currentState = 0;
    std::set<std::pair<char, int>> holdSet;
    std::string::iterator itString;
    std::list<int>::iterator itAccepted;
    std::list<std::pair<int, std::set<std::pair<char, int>>>>::iterator itList;
    std::set<std::pair<char, int>>::iterator itSet;
    std::cout << "steps: ";
    for(itString = x.begin(); itString != x.end(); itString++) {

        for (itList = DFA_deltaTable.begin(); itList != DFA_deltaTable.end(); itList++) {
            if (itList->first == currentState) {
                for (itSet = itList->second.begin(); itSet != itList->second.end(); itSet++) if (itSet->first == *itString) break;
                break;
            }
        }
        if (itSet != itList->second.end()) {
            currentState = itSet->second;
            std::cout << "->" << currentState;
        }
        else {
            currentState = itSet->second;
            return false;
        }
    }
    for(itAccepted = DFA_acceptingStates.begin(); itAccepted != DFA_acceptingStates.end(); itAccepted++) if(*itAccepted == currentState) return true;
    return false;
}



int main() {
    int q,s,a,m,n;
    std::string x;
    std::cin >> q >> s >> a >> m >> n;
    NFA myNFA(q,s,a,m);
    while(n > 0){
        std::cin >> x;
        if(myNFA.check(x)) std::cout << "\n => YES!\n\n";
        else std::cout << "\n => NO!\n\n";
        n--;
    }
    return 0;
}
