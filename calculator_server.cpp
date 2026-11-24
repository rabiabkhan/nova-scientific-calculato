#include <arpa/inet.h>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>

static std::string readFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return "";
    std::ostringstream ss; ss << f.rdbuf(); return ss.str();
}
static std::string jsonEscape(const std::string& s) {
    std::string o;
    for (char c : s) { if (c=='"' || c=='\\') o+='\\'; if (c=='\n') {o+="\\n"; continue;} o+=c; }
    return o;
}

class Parser {
    std::string s; size_t p=0; bool degrees;
    void ws(){ while(p<s.size() && isspace((unsigned char)s[p])) ++p; }
    bool eat(char c){ ws(); if(p<s.size()&&s[p]==c){++p;return true;} return false; }
    double expr(){ double x=term(); for(;;){if(eat('+'))x+=term();else if(eat('-'))x-=term();else return x;} }
    double term(){ double x=power(); for(;;){if(eat('*'))x*=power();else if(eat('/')){double y=power();if(y==0)throw std::runtime_error("Cannot divide by zero");x/=y;}else if(eat('%'))x=std::fmod(x,power());else return x;} }
    double power(){ double x=unary(); if(eat('^')) x=std::pow(x,power()); return x; }
    double unary(){ if(eat('+'))return unary(); if(eat('-'))return -unary(); return atom(); }
    std::string identifier(){ ws(); size_t b=p; while(p<s.size()&&(isalpha((unsigned char)s[p])||s[p]=='_'))++p; return s.substr(b,p-b); }
    double atom(){
        ws();
        if(eat('(')){double x=expr();if(!eat(')'))throw std::runtime_error("Missing closing bracket");return x;}
        if(p<s.size() && (isdigit((unsigned char)s[p])||s[p]=='.')) {
            char* end=nullptr; const char* start=s.c_str()+p; double x=strtod(start,&end);
            if(end==start)throw std::runtime_error("Invalid number"); p += (end-start); return x;
        }
        std::string id=identifier();
        if(id.empty()) throw std::runtime_error("Please enter a valid expression");
        if(id=="pi") return M_PI; if(id=="e") return M_E;
        if(!eat('(')) throw std::runtime_error("Unknown value: "+id);
        double x=expr(); if(!eat(')'))throw std::runtime_error("Missing closing bracket");
        double a=degrees?x*M_PI/180.0:x;
        if(id=="sin")return std::sin(a); if(id=="cos")return std::cos(a); if(id=="tan")return std::tan(a);
        if(id=="sqrt"){if(x<0)throw std::runtime_error("Square root needs a non-negative number");return std::sqrt(x);}
        if(id=="log"){if(x<=0)throw std::runtime_error("Log needs a positive number");return std::log10(x);}
        if(id=="ln"){if(x<=0)throw std::runtime_error("Ln needs a positive number");return std::log(x);}
        if(id=="abs")return std::fabs(x);
        throw std::runtime_error("Unknown function: "+id);
    }
public:
    Parser(std::string input,bool deg):s(input),degrees(deg){}
    double run(){double v=expr();ws();if(p!=s.size())throw std::runtime_error("Unexpected character");if(!std::isfinite(v))throw std::runtime_error("Result is not a finite number");return v;}
};

static std::string getValue(const std::string& body,const std::string& key) {
    std::string needle="\""+key+"\""; size_t p=body.find(needle);
    if(p==std::string::npos)return "";
    p=body.find(':',p); if(p==std::string::npos)return ""; ++p;
    while(p<body.size()&&isspace((unsigned char)body[p]))++p;
    if(p<body.size()&&body[p]=='"'){++p;std::string out;while(p<body.size()&&body[p]!='"'){if(body[p]=='\\'&&p+1<body.size())++p;out+=body[p++];}return out;}
    size_t e=body.find_first_of(",}",p);return body.substr(p,e==std::string::npos?std::string::npos:e-p);
}
static std::string response(const std::string& status,const std::string& type,const std::string& body) {
    std::ostringstream o; o<<"HTTP/1.1 "<<status<<"\r\nContent-Type: "<<type<<"\r\nContent-Length: "<<body.size()
      <<"\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n"<<body; return o.str();
}
int main(){
    int port=8080; if(const char* p=getenv("PORT"))port=atoi(p);
    int server=socket(AF_INET,SOCK_STREAM,0); if(server<0){perror("socket");return 1;}
    int opt=1; setsockopt(server,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    sockaddr_in addr{};addr.sin_family=AF_INET;addr.sin_addr.s_addr=INADDR_ANY;addr.sin_port=htons(port);
    if(bind(server,(sockaddr*)&addr,sizeof(addr))<0){perror("bind");return 1;}
    if(listen(server,20)<0){perror("listen");return 1;}
    std::cout<<"Calculator server running on port "<<port<<std::endl;
    while(true){
        int client=accept(server,nullptr,nullptr); if(client<0)continue;
        std::string req; char buf[4096]; ssize_t n;
        while((n=recv(client,buf,sizeof(buf),0))>0){req.append(buf,n); if(req.find("\r\n\r\n")!=std::string::npos)break;}
        size_t headEnd=req.find("\r\n\r\n"); std::string head=req.substr(0,headEnd), body=headEnd==std::string::npos?"":req.substr(headEnd+4);
        size_t cl=head.find("Content-Length:"); if(cl!=std::string::npos){size_t e=head.find("\r\n",cl);int len=atoi(head.substr(cl+15,e-cl-15).c_str());while((int)body.size()<len){n=recv(client,buf,sizeof(buf),0);if(n<=0)break;body.append(buf,n);}}
        std::istringstream hs(head);std::string method,path,version;hs>>method>>path>>version;
        std::string out;
        if(method=="OPTIONS") out=response("204 No Content","text/plain","");
        else if(method=="POST"&&path=="/api/calculate"){
            try{std::string expression=getValue(body,"expression");bool deg=getValue(body,"degrees")!="false";
                double result=Parser(expression,deg).run();std::ostringstream v;v.precision(15);v<<result;
                out=response("200 OK","application/json","{\"ok\":true,\"result\":"+v.str()+"}");
            }catch(const std::exception& e){out=response("400 Bad Request","application/json","{\"ok\":false,\"error\":\""+jsonEscape(e.what())+"\"}");}
        } else if(method=="GET"&&(path=="/"||path=="/index.html"||path=="/style.css"||path=="/script.js")){
            std::string file=path=="/"? "public/index.html":"public"+path;
            std::string data=readFile(file);
            std::string type=path=="/style.css"?"text/css":path=="/script.js"?"application/javascript":"text/html";
            out=data.empty()?response("404 Not Found","text/plain","File not found"):response("200 OK",type,data);
        } else out=response("404 Not Found","text/plain","Not found");
        send(client,out.c_str(),out.size(),0);close(client);
    }
    close(server);return 0;
}
