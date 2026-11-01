#include <bits/stdc++.h>
using namespace std;
class TwoBuffer {
    static constexpr size_t N=8;
    string data; array<array<char,N+1>,2> buf{}; array<size_t,2> len{};
    int active=0; size_t pos=0, source=0; bool ended[2]={false,false};
    void load(int b){len[b]=0;while(len[b]<N&&source<data.size())buf[b][len[b]++]=data[source++];buf[b][len[b]]='\0';ended[b]=(source>=data.size());}
public:
    explicit TwoBuffer(string x):data(move(x)){load(0);}
    char get(){if(pos>=len[active]){if(ended[active])return '\0';active=1-active;load(active);pos=0;if(len[active]==0)return '\0';}return buf[active][pos++];}
};
int main(int argc,char**argv){string src;if(argc>1){ifstream f(argv[1]);if(!f){cerr<<"Cannot open input\n";return 1;}src.assign(istreambuf_iterator<char>(f),{});}else src.assign(istreambuf_iterator<char>(cin),{});
    TwoBuffer scanner(src); string lex; char c; cout<<"Lexemes read using two buffers:\n";
    while((c=scanner.get())!='\0'){if(isspace((unsigned char)c)){if(!lex.empty()){cout<<lex<<'\n';lex.clear();}}else lex+=c;}if(!lex.empty())cout<<lex<<'\n';
}
