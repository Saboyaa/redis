#include <iostream>
#include <string.h>
#include <vector>

using namespace std;

#define MAXDATASIZE 100
#define BULK "bulk"
#define NUM "num"

struct resp
{
    string typ;
    string str;
    int num;
    string bulk;
    vector<resp> vec; 
};
bool check_number(char x){
    if((x>='0')&&(x<='9')){
        return true;
    }else{return false;}
}
void write_bulk(resp s,string *ans){
    (*ans) += "$";
    (*ans) += to_string(s.bulk.size());
    (*ans) += "\r\n";
    (*ans) += s.bulk;
    (*ans) += "\r\n";
}
void write_int(resp s,string *ans){
    string var = to_string(s.num);
    (*ans) += ":";
    (*ans) += to_string(var.size());
    (*ans) += "\r\n";
    (*ans) += var;
    (*ans) += "\r\n";
}
void write_vec(vector<resp> s, string *ans){
    for(int i =0;i<s.size();i++){
        if(s[i].typ == BULK){
            write_bulk(s[i],ans);
        }
        else if(s[i].typ == NUM){
            write_int(s[i],ans);
        }
        else{
            cerr<<"type not found";
        }
    }
}
resp read_bulk(int *actual,char input[]){
    (*actual)++;
    int var=0;
    string ans = "";
    while(check_number(input[*actual])){
        var *=10;
        var += input[*actual]-'0';
        (*actual)++;
    }
    (*actual)+=2;
    for(int i=0;i<var;i++){
        ans+=input[*actual];
        (*actual)++;
    }
    (*actual)+=2;
    resp x;
    x.typ = BULK;
    x.bulk = ans;
    return x;
}
resp read_int(int *actual,char input[]){
    (*actual)++;
    int var=0;
    int ans = 0;
    while(check_number(input[*actual])){
        var *=10;
        var += input[*actual]-'0';
        (*actual)++;
    }
    (*actual)+=2;
    for(int i=0;i<var;i++){
        ans*=10;
        ans+=input[*actual]-'0';
        (*actual)++;
    }
    (*actual)+=2;
    resp x;
    x.typ = NUM;
    x.num = ans;
    return x;
}
resp read_array(int *actual,char input[]){
    (*actual)++;
    int var=0;
    vector<resp> ans;
    while(check_number(input[*actual])){
        var *=10;
        var += input[*actual]-'0';
        (*actual)++;
    }
    (*actual)+=2;
    for(int i =0;i< var;i++){
        if(input[*actual]=='$'){
            ans.push_back(read_bulk(actual,input));
        }
        else if(input[*actual]==':'){
            ans.push_back(read_int(actual,input));
        }
        else if(input[*actual]=='*'){
            ans.push_back(read_array(actual,input));
        }
        else{
            cerr<<"format not found";
        }
    }
    resp x;
    x.vec = ans;
    return x;
}

vector<resp> read_all(char input[]){
    int i =0;
    vector<resp> ans;
    while(input[i]!='\0'){
        cout<<input[i];
        cout<<i;
        if(input[i]=='$'){
            ans.push_back(read_bulk(&i,input));
        }
        else if(input[i]==':'){
            ans.push_back(read_int(&i,input));
        }
        else if(input[i]=='*'){
            ans.push_back(read_array(&i,input));
        }
        else{
            cerr<<"format not found";
        }
    }
    return ans;
}
