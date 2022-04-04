#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>
#include "io.cpp"
using namespace std;
void RemoveDuplicates(vector<LexItem> &tokens) {
    for (int i = 0; i < tokens.size() - 1; i++) {
        for (int j = i + 1; j < tokens.size(); j++) {
            if (tokens.at(i).GetLexeme() == tokens.at(j).GetLexeme()) {
                tokens.erase(tokens.begin() + j);
                i--;
                break;
            }
        }
    }
}
void Reorganize(vector<LexItem> &tokens) {
    if (tokens.size() == 1)
        return;
    RemoveDuplicates(tokens);
    for (int i = 0; i < tokens.size(); i++) {
        for (int j = i; j < tokens.size(); j++) {
            LexItem remove = tokens.at(j);
            bool erase = false;
            if (tokens.at(i).GetToken() == ICONST) {
                int alpha = stoi(tokens.at(j).GetLexeme());
                int beta = stoi(tokens.at(i).GetLexeme());

                if (alpha < beta)
                    erase = true;
            }
            else if (tokens.at(i).GetToken() == RCONST) {
                int alpha = stof(tokens.at(j).GetLexeme());
                int beta = stof(tokens.at(i).GetLexeme());

                if (alpha < beta)
                    erase = true;
            }
            else if (tokens.at(j).GetLexeme() < tokens.at(i).GetLexeme())
                erase = true;

            if (erase) {
                tokens.erase(tokens.begin() + j);
                tokens.insert(tokens.begin() + i, remove);
            }
        }
    }
}

void Arguments(vector<string> &arguments, int index) {
    for (int i = 1; i < arguments.size(); i++)
        arguments[i] = i == index ? "true" : "false";
}
vector<string> CArgs(int argc, char**argv);
int main(int argc, char** argv) {
    vector<string> arguments = CArgs(argc, argv);

    LexItem token;
    ifstream file;
    file.open(arguments[0]);
    vector<LexItem> tokens;
    int lineNumber = 0;
    int tokenCount = 0;

    if (file.is_open()) {
        while (true) {
            token = getNextToken(file, lineNumber);

            if (token == DONE)
                break;
            tokenCount++;
            if (arguments[1] == "true")
                cout << token << endl;
            else if (arguments[2] == "true" && token.GetToken() == ICONST)
                tokens.push_back(token);
            else if (arguments[3] == "true" && token.GetToken() == RCONST)
                tokens.push_back(token);
            else if (arguments[4] == "true" && token.GetToken() == SCONST)
                tokens.push_back(token);
            else if (arguments[5] == "true" && token.GetToken() == IDENT)
                tokens.push_back(token);

            if (token == ERR) {
                cout << "ERROR ON LINE: " << token.GetLinenum() << " " << token.GetLexeme() << endl;
                exit(1);
            }
        }
        cout << endl;
        cout << "Lines: " << lineNumber << endl;
        if (tokenCount != 0)
            cout << "Tokens: " << tokenCount << endl;
        cout << endl;
        if (tokens.size() > 0) {
            if (arguments[4] == "true")
                cout << "STRINGS:" << endl;
            else if (arguments[2] == "true")
                cout << "INTEGERS:" << endl;
            else if (arguments[3] == "true")
                cout << "REALS:" << endl;
            else if (arguments[5] == "true")
                cout << "IDENTIFIERS: ";
        }
        if (tokens.size() != 0) {
            Reorganize(tokens);
            if (tokens[0].GetToken() != IDENT) {
                for (int i = 0; i < tokens.size(); i++) {
                    cout << tokens.at(i).GetLexeme() << endl;
                }
            }
            else {
                for (int i = 0; i < tokens.size() - 1; i++) {
                    cout << tokens.at(i).GetLexeme() << ", ";
                }
                cout << tokens.at(tokens.size() - 1).GetLexeme() << endl;
            }
        }
    }
    else {
        cerr << "CANNOT OPEN THE FILE " << arguments[0] << endl;
    }
}
vector<string> CArgs(int argc, char** argv) {
    vector<string> properties {"file", "false", "false", "false", "false" , "false"};

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        if (arg == "-v")
            Arguments(properties, 1);
        else if (arg == "-iconsts")
            Arguments(properties, 2);
        else if (arg == "-rconsts")
            Arguments(properties, 3);
        else if (arg == "-sconsts")
            Arguments(properties, 4);
        else if (arg == "-ids")
            Arguments(properties, 5);
        else if (arg.find(".txt") != -1) {
            if (properties[0] != "file") {
                cerr << "CANNOT OPEN MORE THAN ONE FILE NAME" << endl;
                exit(1);
            }
            properties[0] = arg;
        }
        else {
            cerr << "UNRECOGNIZED FLAG " << arg << endl;
            exit(1);
        }
    }
    return properties;
}
