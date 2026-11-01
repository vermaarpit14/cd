#include <bits/stdc++.h>
using namespace std;
int main() {
    string s; cout << "Enter identifier: "; if(!(cin>>s)) return 0;
    bool ok=!s.empty() && (isalpha((unsigned char)s[0]) || s[0]=='_');
    for(char c:s) ok &= (isalnum((unsigned char)c) || c=='_');
    unordered_set<string> kw={"int","float","double","char","void","if","else","while","for","return","break","continue"};
    if(kw.count(s)) ok=false;
    cout << (ok ? "Valid identifier\n" : "Invalid identifier\n");
}
