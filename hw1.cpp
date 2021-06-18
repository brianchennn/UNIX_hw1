#include<iostream>
#include<stdio.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<dirent.h>
#include<string.h>
#include<getopt.h>
#include<cstring>
#include<string>
#include<cstdlib>
#include<unistd.h>
#include<regex.h>
#include<pwd.h>
#include<vector>
#define printALL if( (reg_comp(opt_COMMAND,COMMAND) || opt_COMMAND == "") && (reg_comp(opt_TYPE, TYPE) || opt_TYPE == "") && (reg_comp(opt_FILENAME,NAME) || opt_FILENAME == "") )printf("%-20s %-7s %-19s %-6s %-8s %-8s %s %s\n",COMMAND.c_str(), PID.c_str(),USER.c_str(),FD.c_str(),TYPE.c_str(),NODE.c_str(),NAME.c_str(),DELETED.c_str());
using namespace std;

string get_USER(struct stat buf){
    //printf("in get_USER\n");
    struct passwd *password = getpwuid(buf.st_uid);
    //printf("%-20s",password->pw_name);
    
    return (string)password->pw_name;
}
string get_FD(string FD){
    //printf("%-7s",FD.c_str());
    return (string)FD.c_str();
}
string get_FD(char *FD){
    //printf("%-7s",FD);
    return (string)FD;
}
string get_TYPE(struct stat buf){
    switch(buf.st_mode & S_IFMT){ // TYPE
               
        case S_IFCHR:  return "CHR";       break;       
        case S_IFDIR:  return("DIR");       break;        
        case S_IFIFO:  return("FIFO");       break;        
        case S_IFREG:  return("REG");       break;    
        case S_IFSOCK: return("SOCK");       break;     
        default:       return("unknown");       break;        
    }
    
}
string get_NODE(struct stat buf){
    return to_string(buf.st_ino);
    
    
}

bool is_digit(char *str){
    for(int i=0; i< sizeof(str); i++){
        if(str[i] == '\0')return 1;
        //printf("char : %c\n",str[i]);
        if( str[i] < 48 || str[i] > 57 ){
            return 0;
        }
        
        if(i >= 11)return 0;
    }
}

string get_TOKEN(char str[], int col){
    
    int C=1;
    string buf = "";
    int i;
    for(i=0 ; C < col ; i++){
        
        if(str[i] == '\0') return "fail";
        if( (str[i] == ' ' || str[i] == '\t') && buf == "")continue;
        else if(str[i] == ' ' || str[i] == '\t'){
            C++;
            buf = "";  
        }
        else{
            buf.push_back(str[i]);
        }
    }
    for(;str[i]==' ';i++);
    for( i ; str[i] != ' ' && str[i] != '\0' && str[i] != '\n' ; i++ ){
        buf.push_back(str[i]);
        
    }
    if(buf[0] == ' ' || buf[0] == '\n' || buf[0] == '\0') return "fail";
    else return buf;
}
bool reg_comp(string reg, string target){ // https://hackmd.io/@CynthiaChuang/Regular-Expressions-in-C
    reg += ".*" ;
    reg = ".*" + reg;
    regex_t preg;
    const char* pattern = reg.c_str();
    int success = regcomp(&preg, pattern, REG_EXTENDED|REG_ICASE);
    regmatch_t matchptr[1];
    const size_t nmatch = 1; 
    int status = regexec(&preg, target.c_str(), nmatch, matchptr, 0);
    if (status == REG_NOMATCH){ // 沒匹配
        return 0;  
    }else if (status == 0){  // 匹配
        return 1;
    }
}
using namespace std;
int main(int argc, char *argv[])
{
    char c; 
    string opt_COMMAND = "",opt_TYPE = "",opt_FILENAME = "";
    while( (c = getopt(argc, argv, "c:t:f:")) != EOF){
        
        switch(c){
            case 'c':
                //cout<<"rerewa\n";
                opt_COMMAND = (string)optarg;break;
            case 't':
                opt_TYPE = (string)optarg;
                if(opt_TYPE != "REG" && 
                   opt_TYPE != "CHR" && 
                   opt_TYPE != "DIR"  && 
                   opt_TYPE != "FIFO"&& 
                   opt_TYPE != "SOCK" &&
                   opt_TYPE != "unknown"){
                       printf("Invalid TYPE option.\n");
                       return 0;
                   }
                break;

            case 'f':
                opt_FILENAME = (string)optarg;break;
            case '?':
                printf("Valid option : [-ctf]\n");return 0;
        }   
    }
    cout << "COMMAND             PID      USER         	 FD     TYPE     NODE     NAME" << endl;
    DIR *dp = opendir("/proc/");
    if(dp == NULL) {
        perror("opendir error\n");
        return 0;
    }
    struct dirent *dirp ;
    
    while( (dirp = readdir(dp)) != NULL){
        if(atoi(dirp->d_name) == 0)continue;
        
        
            string COMMAND, PID, USER, FD, TYPE, NODE, NAME, DELETED="";
            string proc_num = (string)dirp->d_name;
            PID = proc_num; //不會更動了
            struct stat proc_buf;
            stat(("/proc/" + proc_num ).c_str(),&proc_buf);
            
            USER = get_USER(proc_buf);//不會更動了
            
            //printf("proc_num : %s\n", proc_num.c_str() );
            struct stat cwd_buf;
            struct stat root_buf;
            struct stat exe_buf;
            struct stat mem_buf;
            struct stat fd_buf;
            char cwd_NAME[1024],root_NAME[1024],exe_NAME[1024];
            int flag;
            char *line_buf;
            size_t len = 0;
            for(int i=0;i<1024;i++){
                cwd_NAME[i] = '\0';
                root_NAME[i] = '\0';
                exe_NAME[i] = '\0';
            }
            /*cwd*/

            FILE *COMMAND_fp = fopen( ("/proc/" + proc_num + "/comm").c_str(),"r");
            getline(&line_buf,&len,COMMAND_fp);
            COMMAND = get_TOKEN(line_buf,1);
            fclose(COMMAND_fp);
            flag = 0;
            flag = readlink( ("/proc/" + proc_num + "/cwd").c_str(),cwd_NAME,1024);
            stat(cwd_NAME,&cwd_buf);

            
            FD = "cwd";
            TYPE = get_TYPE(cwd_buf);
            NODE = get_NODE(cwd_buf);
            if(flag == -1)      {TYPE="unknown"; NODE = "";NAME = "/proc/" + proc_num + "/cwd" + " (readlink: Permission denied)";}
            else                NAME = (string)cwd_NAME;
            printALL;
            NAME ="";
            /*root*/
           
            flag = readlink( ("/proc/" + proc_num + "/root").c_str(),root_NAME,1024);
            stat(root_NAME,&root_buf);
            FD = "root";
            TYPE = get_TYPE(root_buf);
            NODE = get_NODE(root_buf);
            if(flag == -1)      { TYPE="unknown"; NODE = "";NAME = "/proc/" + proc_num + "/root" + " (readlink: Permission denied)";}
            else                NAME = (string)root_NAME;
            printALL;
            NAME ="";
            DELETED="";
            /*exe*/
           
            flag = readlink( ("/proc/" + proc_num + "/exe").c_str(),exe_NAME,1024);
            stat(exe_NAME,&exe_buf);
            FD = "exe";
            TYPE = get_TYPE(exe_buf);
            NODE = get_NODE(exe_buf);
            if(flag == -1)      {TYPE="unknown"; NODE = "";NAME = "/proc/" + proc_num + "/exe" + " (readlink: Permission denied)";}
            else                NAME = (string)exe_NAME;
            printALL;
            //printf("deleted : %s\n",DELETED.c_str());
            NAME ="";
            DELETED="";
            /*mem*/

            FILE *map_fp = fopen(("/proc/" + proc_num + "/maps").c_str(),"r");
            flag = 0;
            if(map_fp == NULL){         
                flag = 1;
            }
            
            vector<string> NODE_v;
            while(flag == 0 && getline(&line_buf,&len,map_fp) != -1 ){  
                PID = (string)dirp->d_name;                      
                DELETED = "";
                if(get_TOKEN(line_buf,7) == "(deleted)"){
                    FD = "del";
                    TYPE = "unknown";
                    DELETED = "(deleted)";
                }
                else {
                    FD = "mem";      
                    TYPE = "REG";       
                }
                NODE = get_TOKEN(line_buf,5);            
                if(NODE != "fail" && NODE != "0"){
                    int F = 0;
                    for(int i = 0; i < NODE_v.size() ; i++){ //判斷NODE是否曾出現
                        if(NODE == NODE_v[i]){
                            F = 1;break;
                        }
                    }
                    if(F == 1)continue;
                    else NODE_v.push_back(NODE);
                }else{
                    continue;
                }
                NAME = get_TOKEN(line_buf,6);          
                printALL; 
                NAME =""; 
                DELETED="";
            }
            
            /*fd*/
            
            string fd_dirs = "/proc/" + (string)dirp->d_name + "/fd/"; // /proc/31137/fd/
            DIR *dp2 = opendir(fd_dirs.c_str());
            if(dp2 == NULL){
                FD = "NOFD";
                TYPE = "";
                NODE = "";
                NAME = "/proc/" + proc_num + "/fd" + " (opendir: Permission denied)";
                printALL;
                NAME ="";
                DELETED="";
                continue;
            }
            struct dirent *dirp2;
            NODE_v.clear();
            while(dirp2 = readdir(dp2)){
                if( !is_digit(dirp2->d_name) )continue;
                FD = (string)dirp2->d_name;
                char fd_NAME[1024];
                for(int i=0;i<1024;i++){
                    fd_NAME[i] = '\0';
                }
                for(int i=0 ; i< 1024; i++) fd_NAME[i] = '\0';
                string fd_str = fd_dirs + (string)dirp2->d_name; 
                flag = readlink( fd_str.c_str(),fd_NAME,1024 );
                if(flag == -1){
                    NAME = "/proc/" + proc_num + "/fd" + FD + "(readlink: Permission denied)";
                    TYPE = "unknown";
                    NODE = "";
                    printALL;
                    NAME ="";
                    DELETED="";
                    continue;
                }
                else NAME = (string)fd_NAME;
                
                int fd = atoi(FD.c_str());
                
                
                string link = fd_dirs + dirp2->d_name;
                stat(link.c_str(),&fd_buf);
                struct stat fd_link_buf;
                lstat(( "/proc/" + proc_num + "/fd/" + FD).c_str(),&fd_link_buf);
                int can_read = fd_link_buf.st_mode & S_IRUSR;
                int can_write = fd_link_buf.st_mode & S_IWUSR;
                
                if(can_read == 0400 && can_write == 0200){
                    FD.push_back('u');
                }else if(can_read == 0400){
                    FD.push_back('r');
                }else if(can_write == 0200){
                    FD.push_back('w');
                }
                
                TYPE = get_TYPE(fd_buf);
                if(get_TOKEN(fd_NAME,2) == "(deleted)"){
                    TYPE = "unknown";
                }
                NODE = get_NODE(fd_buf);
                flag = 0;
                for(int i=0 ; i< NODE_v.size() ; i++){
                    if(NODE == NODE_v[i]){
                        flag = 1;break;
                    }
                }
                if(flag == 1)continue;
                DELETED = "";
                printALL;
                NAME ="";
                DELETED="";
                
            }
        
        //}

    }
        
    
}