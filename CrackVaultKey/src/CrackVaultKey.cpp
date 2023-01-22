#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <queue>

using namespace std;

typedef struct {
    int id;
    string  data;
    vector<int> neighbors;
} SimpleNode;

using _path = vector<SimpleNode>;

static _path nmap = {{
        {0,     string("22"),   vector<int>({1, 4})},
        {1,     string("-"),    vector<int>({2, 5})},
        {2,     string("9"),    vector<int>({1, 3, 6})},
        {3,     string("*"),    vector<int>({2, 7})},
        {4,     string("+"),    vector<int>({5, 8})},
        {5,     string("4"),    vector<int>({1, 4, 6, 9})},
        {6,     string("-"),    vector<int>({2, 5, 7, 10})},
        {7,     string("18"),   vector<int>({3, 6, 11})},
        {8,     string("4"),    vector<int>({4, 9, 12})},
        {9,     string("*"),    vector<int>({5, 8, 10, 13})},
        {10,    string("11"),   vector<int>({6, 9, 11, 14})},
        {11,    string("*"),    vector<int>({7, 10, 15})},
        {12,    string("*"),    vector<int>({8, 13})},
        {13,    string("8"),    vector<int>({9, 12, 14})},
        {14,    string("-"),    vector<int>({10, 13, 15})},
        {15,    string("1"),    vector<int>({15})}
    }};

int eval_path(_path& path) {
    int res;
    _path::iterator it;

    it = path.begin();
    res = stoi(it->data);
    for (/* init above */; it != path.end() - 1; it += 2) {
        int b = stoi((it + 2)->data);
        string& op((it + 1)->data);

        if (!op.compare("-"))
            res -= b;
        else if (!op.compare("+"))
            res += b;
        else if (!op.compare("*"))
            res *= b;
    }

    return res;
}

constexpr int tval = 30;

void crackKey(const _path& nmap, SimpleNode& root, SimpleNode& target) {
    const int NCount = nmap.size();
    queue<_path> qpaths;
    map<int, int> visitCount;
    int visitLimit = NCount;

    for (int i = 0; i < NCount; i++)
        visitCount.emplace(make_pair(i, 0));

    visitCount[root.id]++;

    qpaths.push(_path({root}));

    while (!qpaths.empty()) {
        auto current = qpaths.front();
        qpaths.pop();
        auto last = current.crbegin();

        if (last->id == target.id) {
            int res = eval_path(current);
            if (res == tval) {
                cout << "Found a path to open Vault:" << endl;
                for (auto& node : current)
                    cout << node.data << " ";
                cout << endl;
                break;
            } else {
                /* lookup for another path */
                visitLimit += NCount;
            }
        } else {
            for (auto& neighbor : last->neighbors) {
                if (visitCount[neighbor] < visitLimit) {
                    visitCount[neighbor]++;

                    const SimpleNode& nn = nmap[neighbor];
                    _path tmp(current);
                    tmp.push_back(nn);

                    qpaths.push(tmp);
                }
            }
        }
    }

}

int main(int argc, char **argv) {

    crackKey (nmap, nmap[0], nmap[15]);

    return 0;
}