#include <iostream>
#include <random>
#include <thread>
#include <ctime>
#include <string>
#include <vector>
#include <experimental/random>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <semaphore.h>
#include <chrono>
#include <fstream>

#define N 10

using namespace std;
static auto stime=clock();

static ofstream outputFile("connecteds.txt");

static bool flag=0;

class Semaphore {
private:
    mutex mtx;
    queue<int> q;
    condition_variable cv;
    unsigned int count;
public:
    Semaphore (unsigned count_ = 1)
        : count(count_)
    {
    }

    inline void up() {
        lock_guard<std::mutex> lock(mtx);
        if(q.empty())
            count++;
        else
        {
            cv.notify_one();
            q.pop();
        }
    }
    inline void down()
    {
        unique_lock<std::mutex> lock(mtx);
        if (count == 0) {
            q.push(0);
            cv.wait(lock);

        }
        else
        {
            count--;
        }


    }
    inline void upall() {
        lock_guard<std::mutex> lock(mtx);
        if(q.empty())
            count++;
        else
        {
            cv.notify_all();
            while(!q.empty())
            {
                q.pop();
            }
        }
    }

    void change(unsigned newcount){//carefull using this. I defined it only for global semaphores inorder to change the count in main.

        count=newcount;
    }
    unsigned access(){
        return count;
    }

};

static Semaphore Full;
static Semaphore Empty(0);
static Semaphore MutualEx;
static Semaphore recived(0);

string genRandom();
string read(vector<string>&sharedData);
void write(vector<string>&sharedData,string data);
string produce();
void sender();
class senderThreadHandler;
void reciever();
class recieverThreadHandler;
void mainReciever();
static const char alphanum[] =
        "0123456789"
        "!@#$%^&*"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
//unsigned semaphoreNum =0;// can you delete this?
static vector<string>sharedData;
static vector<string>recieversShared;
static queue<unsigned>length;

class senderThreadHandler
{
    friend class recieverThreadHandler;
    vector<thread>senders;
public:
/*    senderThreadHandler(unsigned int s,unsigned int t)// with a few changes I wanted to cover all the slots!
    {
        if(s>=t)
        {
            unsigned const threadSpread=s/t;
            unsigned start=0;
            unsigned end=threadSpread;
            for (unsigned i = 0; i < t-1; ++i)
            {
                senders.emplace_back(sender,start,end);
                start+=threadSpread;
                end+=threadSpread;
            }
            senders.emplace_back(sender,start,s);
           // semaphoreNum=threadSpread;


        }
        else
        {

           // for example for 2 slots and 5 threads we need 3 thread for the first and 2 for second slot.thread ans slots both begin from 0.
            for (unsigned i = 1; i < t; ++i)
            {
                senders.emplace_back(sender,i%s,i%s+1);
            }
           // semaphoreNum=s;

        }


    }
    */
     senderThreadHandler(unsigned int s,unsigned int t)
     {
         Full.change(s);
         for(unsigned i=0;i<t;i++)//if the semaphores changed in one, all the ather threads will access to the new change or previous one??
         {
             senders.emplace_back(sender);

         }
         //senders.emplace_back(c);

//         for(auto &r:senders)
//             r.join();
     }


};

class recieverThreadHandler{

public:
    recieverThreadHandler( unsigned t,senderThreadHandler &sender)
    {
        unsigned n=length.front();
        length.pop();
        recived.change(n);
        for(unsigned i=0; i<t ;i++)
        {
            sender.senders.emplace_back(reciever);
        }
        sender.senders.emplace_back(mainReciever);
        //sender.senders.emplace_back(breaker);

        for(auto &r:sender.senders)
            r.join();

        //for connecting

        if(length.empty())
        {
//
            while(!sender.senders.empty())
                sender.senders.erase(sender.senders.begin());
        }

    }


};

int main()
{
    unsigned s,t;
    cout<<" Please enter main sender arguments : " ;
    cin>>s>>t;

    unsigned s1,t1,n;

    cout<<"\n Please enter main reciever arguments : ";
    cin>>s1>>t1;
    while(cin>>n)
    {
       length.push(n);
    }
    if(s!=s1) cout<<" You entered different numbers for slots!\n to avoid any conflicts the program will ignore second slot argumant"<<endl;
     senderThreadHandler sender(s,t);
    recieverThreadHandler reciever(t1,sender);

    return 0;
}

//methods
string genRandom()  // Random string generator function.
{
    srand( static_cast<unsigned int>(clock()));
    int randomLength = rand()%N;
    int stringLength = sizeof(alphanum) - 1;
    string randomStr;
    char random;
    for(int i=0; i<randomLength;i++)
    {
        srand(static_cast<unsigned int>(clock()));
        int random_number = rand()%stringLength;
        random=alphanum[random_number % stringLength];
        randomStr+=random;
    }
    return randomStr;
}

string read(vector<string>&sharedData)
{
    string data=sharedData.front();
    sharedData.erase(sharedData.begin());
    return data;

}

void write(vector<string>&sharedData,string data)
{
    sharedData.push_back(data);
}

string produce()
{
    string data=genRandom();
    return data;
}
void log(string h)
{
    auto etime=clock();
    auto threadid=this_thread::get_id();
    cout<<(etime-stime)/double(CLOCKS_PER_SEC)<<" - thread "<<threadid<<h<<endl;
}


[[noreturn]] void sender( )//end shouldn't count unless it will count twice
{

    while (1)
    {
        if(flag)
            exit(0);

      string data=produce();
      Full.down();
      MutualEx.down();
      write(sharedData,data);
      log(" finished writing ");
      MutualEx.up();
      Empty.up();
    }


}
void mainReciever()
{
    while(1)
    {
        if(flag)
            break;

        if(recived.access()==0)
        {
            string data;
            while(!recieversShared.empty())
            {
                data+=recieversShared.back();
                recieversShared.pop_back();
            }

            outputFile<<data<<endl;
            if(length.empty())
                flag=1;
            else
            {
                unsigned n=length.front();
                length.pop();
                recived.change(n);
                recived.upall();
            }

        }
    }
}


[[noreturn]] void reciever()
{

    while(1)
    {
        if(flag)
            exit(0);

        Empty.down();
        recived.down();
        MutualEx.down();
        log(" started reading");
        write(recieversShared,read(sharedData));
        MutualEx.up();
        Full.up();
    }
}
