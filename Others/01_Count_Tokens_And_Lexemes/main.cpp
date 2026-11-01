#include <bits/stdc++.h>
using namespace std;
int main(int argc, char** argv) {
    string src;
    if (argc > 1) { ifstream f(argv[1]); if (!f) { cerr << "Cannot open input file\n"; return 1; } src.assign(istreambuf_iterator<char>(f), {}); }
    else src.assign(istreambuf_iterator<char>(cin), {});
    unordered_set<string> kw={"int","float","double","char","void","if","else","while","for","return","break","continue","long","short","const"};
    vector<pair<string,string>> tokens;
    vector<string> ops={"<<=",">>=","++","--","==","!=","<=",">=","&&","||","+=","-=","*=","/=","%=","<<",">>","->","+","-","*","/","%","=","<",">","!","&","|","^","~","?",":"};
    string seps="(){}[];, .";
    for (size_t i=0;i<src.size();) {
        if (isspace((unsigned char)src[i])) { ++i; continue; }
        if (src.compare(i,2,"//")==0) { while(i<src.size() && src[i]!='\n') ++i; continue; }
        if (src.compare(i,2,"/*")==0) { i+=2; while(i+1<src.size() && src.compare(i,2,"*/")!=0) ++i; i=min(src.size(),i+2); continue; }
        size_t st=i;
        if (isalpha((unsigned char)src[i]) || src[i]=='_') { ++i; while(i<src.size() && (isalnum((unsigned char)src[i]) || src[i]=='_')) ++i; string x=src.substr(st,i-st); tokens.push_back({kw.count(x)?"KEYWORD":"IDENTIFIER",x}); }
        else if (isdigit((unsigned char)src[i])) { ++i; while(i<src.size() && isdigit((unsigned char)src[i])) ++i; if(i<src.size() && src[i]=='.' && i+1<src.size() && isdigit((unsigned char)src[i+1])) { ++i; while(i<src.size() && isdigit((unsigned char)src[i])) ++i; } tokens.push_back({"NUMBER",src.substr(st,i-st)}); }
        else if (src[i]=='"' || src[i]=='\'') { char q=src[i++]; while(i<src.size() && src[i]!=q) { if(src[i]=='\\' && i+1<src.size()) i+=2; else ++i; } if(i<src.size()) ++i; tokens.push_back({q=='"'?"STRING":"CHAR",src.substr(st,i-st)}); }
        else { string found; for(auto &op:ops) if(src.compare(i,op.size(),op)==0 && op.size()>found.size()) found=op; if(!found.empty()) { tokens.push_back({"OPERATOR",found}); i+=found.size(); } else if(seps.find(src[i])!=string::npos) { tokens.push_back({"SEPARATOR",string(1,src[i])}); ++i; } else { tokens.push_back({"UNKNOWN",string(1,src[i])}); ++i; } }
    }
    map<string,int> count;
    cout << left << setw(14) << "TOKEN TYPE" << "LEXEME\n";
    for(auto &[t,x]:tokens) { cout << left << setw(14) << t << x << '\n'; ++count[t]; }
    cout << "\nToken counts\n"; for(auto &[t,n]:count) cout << t << ": " << n << '\n';
}
