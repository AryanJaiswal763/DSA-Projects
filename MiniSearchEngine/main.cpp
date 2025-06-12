// Mini Search Engine in C++ (dynamic directory version)
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <string>
#include <algorithm>
#include <dirent.h>  // for directory reading (cross-platform alternative)

using namespace std;

unordered_set<string> stopWords = {"is", "the", "and", "a", "an", "of", "to", "in", "on", "with"};

struct TrieNode {
    unordered_map<char, TrieNode*> children;
    bool isEndOfWord = false;
};

class Trie {
public:
    Trie() { root = new TrieNode(); }

    void insert(const string &word) {
        TrieNode* node = root;
        for (char ch : word) {
            if (!node->children[ch]) node->children[ch] = new TrieNode();
            node = node->children[ch];
        }
        node->isEndOfWord = true;
    }

    bool search(const string &word) {
        TrieNode* node = root;
        for (char ch : word) {
            if (!node->children[ch]) return false;
            node = node->children[ch];
        }
        return node->isEndOfWord;
    }

private:
    TrieNode* root;
};

class SearchEngine {
private:
    Trie trie;
    unordered_map<string, unordered_map<string, int>> wordToFileFreq;

    vector<string> tokenize(const string &text) {
        vector<string> tokens;
        stringstream ss(text);
        string word;
        while (ss >> word) {
            transform(word.begin(), word.end(), word.begin(), ::tolower);
            word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
            if (!word.empty() && stopWords.find(word) == stopWords.end())
                tokens.push_back(word);
        }
        return tokens;
    }

public:
    vector<string> getTxtFiles(const string &folderPath) {
        vector<string> files;
        DIR *dir;
        struct dirent *entry;

        if ((dir = opendir(folderPath.c_str())) != NULL) {
            while ((entry = readdir(dir)) != NULL) {
                string name = entry->d_name;
                if (name.length() > 4 && name.substr(name.length() - 4) == ".txt") {
                    files.push_back(folderPath + "/" + name);
                }
            }
            closedir(dir);
        }
        return files;
    }

    void indexFiles(const vector<string> &filePaths) {
        for (const string &path : filePaths) {
            ifstream file(path);
            if (!file) continue;

            stringstream buffer;
            buffer << file.rdbuf();
            string content = buffer.str();
            vector<string> words = tokenize(content);

            string filename = path.substr(path.find_last_of("/") + 1);

            for (const string &word : words) {
                trie.insert(word);
                wordToFileFreq[word][filename]++;
            }
        }
    }

    void searchQuery(const string &query) {
        unordered_map<string, int> fileScores;
        vector<string> queryWords = tokenize(query);

        for (const string &word : queryWords) {
            if (!trie.search(word)) continue;
            for (auto &entry : wordToFileFreq[word]) {
                fileScores[entry.first] += entry.second;
            }
        }

        priority_queue<pair<int, string>> pq;
        for (auto &entry : fileScores) {
            pq.push({entry.second, entry.first});
        }

        cout << "\nResults for: \"" << query << "\"\n";
        int rank = 1;
        while (!pq.empty()) {
            pair<int, string> top = pq.top(); pq.pop();
            cout << rank++ << ". " << top.second << " - Score: " << top.first << endl;
        }
        if (rank == 1) cout << "No matches found.\n";
    }
};

int main() {
    SearchEngine engine;
    string folder = "documents";  // path to folder with .txt files
    vector<string> fileList = engine.getTxtFiles(folder);

    if (fileList.empty()) {
        cout << "No .txt files found in folder: " << folder << endl;
        return 0;
    }

    engine.indexFiles(fileList);

    string query;
    cout << "\nEnter search query: ";
    getline(cin, query);
    engine.searchQuery(query);

    return 0;
}
