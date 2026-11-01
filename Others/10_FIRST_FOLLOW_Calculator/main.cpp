#include <bits/stdc++.h>
using namespace std;
struct P{char l;string r;};
int main(){int n;cout<<"Number of grammar lines: ";if(!(cin>>n))return 0;vector<P> ps;set<char> nts,ts;string line;
    for(int i=0;i<n;i++){cin>>line;auto k=line.find("->");if(k==string::npos){cerr<<"Use A->rhs1|rhs2 format\n";return 1;}char l=line[0];nts.insert(l);string rhs=line.substr(k+2);stringstream ss(rhs);string x;while(getline(ss,x,'|'))ps.push_back({l,x});}
    char start;cout<<"Start symbol: ";cin>>start;for(auto&p:ps)for(char c:p.r)if(c!='#'&&!isupper((unsigned char)c))ts.insert(c);
    map<char,set<char>> first,follow;bool change=true;
    auto firstString=[&](const string& s){set<char> z;bool nullable=true;for(char c:s){if(c=='#')continue;if(!nts.count(c)){z.insert(c);nullable=false;break;}for(char x:first[c])if(x!='#')z.insert(x);if(!first[c].count('#')){nullable=false;break;}}if(nullable)z.insert('#');return z;};
    while(change){change=false;for(auto&p:ps){auto z=firstString(p.r);size_t old=first[p.l].size();first[p.l].insert(z.begin(),z.end());change|=old!=first[p.l].size();}}
    follow[start].insert('$');change=true;while(change){change=false;for(auto&p:ps)for(size_t i=0;i<p.r.size();i++)if(nts.count(p.r[i])){string beta=p.r.substr(i+1);auto z=firstString(beta);size_t old=follow[p.r[i]].size();for(char c:z)if(c!='#')follow[p.r[i]].insert(c);if(z.count('#'))follow[p.r[i]].insert(follow[p.l].begin(),follow[p.l].end());change|=old!=follow[p.r[i]].size();}}
    for(char c:nts){cout<<"FIRST("<<c<<") = { ";for(char x:first[c])cout<<x<<' ';cout<<"}\n";}
    for(char c:nts){cout<<"FOLLOW("<<c<<") = { ";for(char x:follow[c])cout<<x<<' ';cout<<"}\n";}
}
