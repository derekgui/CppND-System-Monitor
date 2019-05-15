#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"


using namespace std;

class ProcessParser{
private:
    std::ifstream stream;

    public:
    static std::string getCmd(std::string pid);
    static vector<std::string> getPidList();
    static std::string getVmSize(std::string pid);
    static std::string getCpuPercent(std::string pid);
    static long int getSysUpTime();
    static std::string getProcUpTime(std::string pid);
    static std::string getProcUser(std::string pid);
    static vector<std::string> getSysCpuPercent(std::string coreNumber = "");
    static float getSysRamPercent();
    static std::string getSysKernelVersion();
    static int getTotalThreads();
    static int getNumberOfCores();
    static int getTotalNumberOfProcesses();
    static int getNumberOfRunningProcesses();
    static std::string getOSName();
    static std::string PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2);
    static bool isPidExisting(std::string pid);
};

// TODO: Define all of the above functions below:
std::vector<string> splitByWhiteSpace(std::string stringToSplit){
    std::istringstream buf(stringToSplit);
    std::istream_iterator<string> beg(buf),end;
    return std::vector<string> (beg,end);
}
//cmd
std::string ProcessParser::getCmd(std::string pid){
    std::string result;

    //Opening stream for specific file
    std::ifstream stream;
    Util::getStream((Path::basePath() + pid + Path::cmdPath()),stream);
    getline(stream,result);
    
    return result;
};

//PID LIST
vector<std::string> ProcessParser::getPidList(){
    DIR* dir;

    vector<string> container;
    if(!(dir = opendir("/proc")))
        throw std::runtime_error(std::strerror(errno));
    
    while(dirent* dirp = readdir(dir)){
        if(dirp->d_type != DT_DIR)
            continue;
        
        if(all_of(dirp->d_name, dirp->d_name + std::strlen(dirp->d_name),
        [](char c){return std::isdigit(c);})){
            container.push_back(dirp->d_name);
        }
    }

    if(closedir(dir))
        throw std::runtime_error(std::strerror(errno));
    return container;
}

//RAM Usage
std::string ProcessParser::getVmSize(std::string pid){
    std::string line;
    //declaring search attribute for file
    std::string name = "vmData";
    float result;
    //Opening stream for specific file
    std::ifstream stream;
    Util::getStream((Path::basePath() + pid + Path::statusPath()),stream);
    while(std::getline(stream, line)){
        if(line.compare(0,name.size(),name) == 0){
            //Searching string line on ws for values using sstream
            std::vector<string> values = splitByWhiteSpace(line);
            //converts KB to GB
            result = (stof(values[1])/float(1024));
            break;
        }
    }
    return to_string(result);
};

//CPU Usage
std::string ProcessParser::getCpuPercent(std::string pid){
    std::string line;
    std::ifstream stream;
    //fetching raw data from below path
    Util::getStream((Path::basePath() + pid +"/" + Path::statPath()),stream);
    std::getline(stream, line);
    
    //slicing the string obj to a vector of string obj
    std::vector<string> values = splitByWhiteSpace(line);

    //fetching all uptime in system clock tick
    float utime = stof(ProcessParser::getProcUpTime(pid));
    float stime = stof(values[14]);
    float cutime = stof(values[15]);
    float cstime = stof(values[16]);
    float starttime = stof(values[21]);

    float uptime = ProcessParser::getSysUpTime();

    float freq = sysconf(_SC_CLK_TCK);

    float total_time = utime + stime + cutime + cstime;
    float seconds = uptime - (starttime/freq);
    float result = 100.0*((total_time/freq)/seconds);

    return to_string(result);
}
    
long int ProcessParser::getSysUpTime(){
    
    std::string line;
    std::ifstream stream;

    Util::getStream((Path::basePath() + Path::upTimePath()),stream);
    std::getline(stream, line);
    std::vector<string> values = splitByWhiteSpace(line);

    return stoi(values[0]);

}

//Process up time
std::string ProcessParser::getProcUpTime(std::string pid){
    
    std::string line;
    std:ifstream stream;

    Util::getStream((Path::basePath() + pid + "/" + Path::statPath()),stream);
    std::getline(stream, line);

    std::vector<string> values = splitByWhiteSpace(line);

    return to_string(float(stof(values[13])/sysconf(_SC_CLK_TCK)));

}

std::string ProcessParser::getProcUser(std::string pid){
    std::string line;
    std::ifstream stream;
    std::string uid= "Uid:";
    std::string result = "";

    //fetch stream from /proc/pid/status
    Util::getStream((Path::basePath() + pid + Path::statusPath()),stream);
    
    while (std::getline(stream , line))
    {   
        //Search for Uid: line
        if(line.compare(0,uid.size(),uid) == 0){
            std::vector<string> values = splitByWhiteSpace(line);
            result = values[1];
            break;
        }
    }
    
    std::ifstream stream2;
    Util::getStream(("/etc/passwd"),stream2);
    uid = "x:" + result;
    // Searching for name of the user with selected UID
    while (std::getline(stream2, line)) {
        if (line.find(uid) != std::string::npos) {
            result = line.substr(0, line.find(":"));
            return result;
        }
    }
    return "";    
}

vector<std::string> ProcessParser::getSysCpuPercent(std::string coreNumber){
    std::string line;
    std::ifstream stream;

    std::string name = "cpu"+coreNumber;
    Util::getStream((Path::basePath() + Path::statPath()), stream);
    while(std::getline(stream,line)){
        if(line.compare(0,name.size(),name) == 0){
            std::vector<string> values = splitByWhiteSpace(line);
            return values;
        }
    }
    
    return (std::vector<string>());
}

float getSysActiveCpuTime(vector<string> values){
    return(stof(values[S_USER])+
           stof(values[S_NICE])+
           stof(values[S_SYSTEM])+
           stof(values[S_IRQ])+
           stof(values[S_SOFTIRQ])+
           stof(values[S_STEAL])+
           stof(values[S_GUEST])+
           stof(values[S_GUEST_NICE]));
}

float getSysIdleCpuTime(vector<string> values){
    return (stof(values[S_IDLE]) + stof(values[S_IOWAIT]));
}

float ProcessParser::getSysRamPercent(){

    std::string line;
    std::ifstream stream;
    std::vector<string> names = {"MemAvailable:","MemFree:","Buffers:"};
    
    float totalMem = 0;
    float freeMem = 0;
    float buffers = 0;
    float memStat[3] = {0};

    Util::getStream((Path::basePath() + Path::memInfoPath()),stream);
    while(std::getline(stream, line)){
        if(totalMem != 0 && freeMem != 0)
            break;
        for(int i = 0; i < names.size(); i++){
        if(line.compare(0,names[i].size(),names[i]) == 0){
            std::vector<string> values = splitByWhiteSpace(line);
            memStat[i] = stof(values[1]);
        }
        }
        totalMem = memStat[0];
        freeMem = memStat[1];
        buffers = memStat[2];
    }

    return float(100.0*(1-(freeMem/(totalMem-buffers))));

}


std::string ProcessParser::getSysKernelVersion(){
    std::string line;
    std::string name = "Linux version ";
    std::ifstream stream;

    Util::getStream((Path::basePath() + Path::versionPath()), stream);
    while(std::getline(stream, line)){
        if(line.compare(0,name.size(), name) == 0){
            std::vector<string> values = splitByWhiteSpace(line);
            return values[2];
        }
    }
    return "";
}

int ProcessParser::getTotalThreads(){
    std::string line;
    std::string name = "Threads:";
    int result = 0;
    std::ifstream stream;

    for(auto pid : getPidList()){
        Util::getStream(Path::basePath() + pid + Path::statusPath(),stream);
        while(std::getline(stream, line)){
            if(line.compare(0,name.size(), name) == 0){
                std::vector<string> values = splitByWhiteSpace(line);
                result += stoi(values[1]);
                break;
            }
        }
    }
    return result;
}

int ProcessParser::getNumberOfCores(){
    
    std::string line;
    std::ifstream stream;

    std::string name = "cpu cores";
    Util::getStream((Path::basePath() + "cpuinfo"),stream);
    while(std::getline(stream, line)){
        if(line.compare(0,name.size(),name) == 0){
            std::vector<string> values = splitByWhiteSpace(line);
            return stoi(values[3]);
        }
    }
    return 0;
}

int ProcessParser::getTotalNumberOfProcesses(){
    std::string line;
    std::ifstream stream;
    std::string name = "processes";

    Util::getStream((Path::basePath()+ Path::statPath()),stream);
    while(std::getline(stream, line)){
        if(line.compare(0,name.size(), name) == 0){
            std::vector<string> values = splitByWhiteSpace(line);
            return stoi(values[1]);
        }
    }
    return 0;
}

int ProcessParser::getNumberOfRunningProcesses(){
    std::string line;
    std::ifstream stream;
    std::string name = "procs_running";

    Util::getStream((Path::basePath() + Path::statPath()), stream);
    while(std::getline(stream, line)){
        if(line.compare(0, name.size(), name) == 0){
            std::vector<string> values = splitByWhiteSpace(line);
            return stoi(values[1]); 
        }
    }
    return 0;
}

std::string ProcessParser::getOSName(){
    std::string line;
    std::ifstream stream;

    std::string name = "PRETTY_NAME=";
    Util::getStream(("/etc/os-release"),stream);
    
    while (std::getline(stream, line)) {
        if (line.compare(0, name.size(), name) == 0) {
              string result = line.substr(line.find("=")+1);
              result.erase(std::remove(result.begin(), result.end(), '"'), result.end());
              return result;
        }
    }
    return "";
}

std::string ProcessParser::PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2){
    
    float activeTime = getSysActiveCpuTime(values2) - getSysActiveCpuTime(values1);
    float idleTime = getSysIdleCpuTime(values2) - getSysIdleCpuTime(values1);
    float totalTime = activeTime + idleTime;
    float result = 100.0*(activeTime / totalTime);
    std::string s = (to_string(result));

    return s;
}

bool ProcessParser::isPidExisting(std::string pid){
    
    for(auto _pid : getPidList()){
        if(_pid == pid)
         return true;
    }
    
    return false;
}