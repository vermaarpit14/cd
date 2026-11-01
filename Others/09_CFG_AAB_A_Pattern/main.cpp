#include <bits/stdc++.h>
using namespace std;
int main(){string s;cout<<"Enter string for (aab)^n a^n: ";cin>>s;size_t i=0;int blocks=0;while(i+2<s.size()&&s.compare(i,3,"aab")==0){i+=3;++blocks;}size_t tail=0;while(i<s.size()&&s[i]=='a'){++i;++tail;}bool ok=i==s.size()&&blocks>0&&tail==(size_t)blocks;cout<<(ok?"Accepted":"Rejected")<<'\n';}
