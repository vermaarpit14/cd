#include <bits/stdc++.h>
using namespace std;
int main(){
    string s; s.assign(istreambuf_iterator<char>(cin),{});
    vector<string> ops={">>=","<<=","++","--","==","!=","<=",">=","&&","||","+=","-=","*=","/=","%=","<<",">>","->","+","-","*","/","%","=","<",">","!","&","|","^","~"};
    sort(ops.begin(),ops.end(),[](auto&a,auto&b){return a.size()>b.size();});
    for(size_t i=0;i<s.size();){ if(isspace((unsigned char)s[i])){++i;continue;} string m;
        for(auto &op:ops) if(s.compare(i,op.size(),op)==0){m=op;break;}
        if(!m.empty()){cout<<"OPERATOR: "<<m<<'\n';i+=m.size();}
        else if(isalpha((unsigned char)s[i])||s[i]=='_'){size_t j=i+1;while(j<s.size()&&(isalnum((unsigned char)s[j])||s[j]=='_'))++j;cout<<"IDENTIFIER: "<<s.substr(i,j-i)<<'\n';i=j;}
        else if(isdigit((unsigned char)s[i])){size_t j=i+1;while(j<s.size()&&(isdigit((unsigned char)s[j])||s[j]=='.'))++j;cout<<"NUMBER: "<<s.substr(i,j-i)<<'\n';i=j;}
        else {cout<<"SYMBOL: "<<s[i]<<'\n';++i;}
    }
}
