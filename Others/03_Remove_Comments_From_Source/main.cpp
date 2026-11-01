#include <bits/stdc++.h>
using namespace std;
int main(int argc,char**argv){
    if(argc<2){cerr<<"Usage: ./program input.c [output.c]\n";return 1;}
    ifstream in(argv[1]); if(!in){cerr<<"Cannot open input file\n";return 1;}
    string s((istreambuf_iterator<char>(in)),{}), out; enum State{NORMAL,LINE,BLOCK,STRING,CHAR}; State st=NORMAL;
    for(size_t i=0;i<s.size();++i){ char c=s[i], n=i+1<s.size()?s[i+1]:'\0';
        if(st==NORMAL){ if(c=='/'&&n=='/'){st=LINE;++i;} else if(c=='/'&&n=='*'){st=BLOCK;++i;} else {out+=c;if(c=='"')st=STRING;else if(c=='\'')st=CHAR;} }
        else if(st==LINE){ if(c=='\n'){out+='\n';st=NORMAL;} }
        else if(st==BLOCK){ if(c=='*'&&n=='/'){st=NORMAL;++i;} else if(c=='\n')out+='\n'; }
        else { out+=c; if(c=='\\'&&i+1<s.size())out+=s[++i]; else if((st==STRING&&c=='"')||(st==CHAR&&c=='\''))st=NORMAL; }
    }
    if(argc>=3){ofstream f(argv[2]);f<<out;} else cout<<out;
}
