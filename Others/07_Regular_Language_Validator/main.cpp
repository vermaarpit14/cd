#include <bits/stdc++.h>
using namespace std;
int main(){string s;cout<<"Enter string for a*b+: ";cin>>s;size_t i=0;while(i<s.size()&&s[i]=='a')++i;size_t b=i;while(i<s.size()&&s[i]=='b')++i;bool ok=(i==s.size()&&i>b);cout<<(ok?"Accepted":"Rejected")<<'\n';}
