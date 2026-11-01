#include <bits/stdc++.h>
using namespace std;
int main(){string s;cout<<"Enter string in a^n b^n: ";cin>>s;size_t i=0;while(i<s.size()&&s[i]=='a')++i;size_t n=i;while(i<s.size()&&s[i]=='b')++i;bool ok=n>0&&i==s.size()&&s.size()-n==n;cout<<(ok?"Accepted by S -> aSb | ab":"Rejected")<<'\n';}
