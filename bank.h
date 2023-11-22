//Project Identifier: 292F24D17A4455C1B5133EDD8C7CEAA0C9570A98

#include <getopt.h>
#include <iostream>
#include <fstream> 
#include <string>
#include <vector>
#include <algorithm>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <queue>
using namespace std;

struct User{
    uint64_t reg_time = 0;
    string userID = "";
    uint32_t pin = 0;
    uint32_t balance = 0;
    unordered_set<string> validIPs = {};
    uint32_t incoming = 0;
    uint32_t outgoing = 0;
};

struct Transaction{
    uint64_t timestamp = 0 ;
    string IPaddress = "";
    string senderID = "";
    string recipientID = "";
    uint32_t amount = 0;
    uint64_t exec_date = 0;

    //boolean for if transaction fee covered or not
    char fee_shared;

    uint32_t id = 0;
    uint32_t fee = 0 ;


};

class Transaction_comparator{
    public:
    bool operator()(const Transaction &trans1, const Transaction &trans2) const {
        if(trans1.exec_date == trans2.exec_date){
            return trans1.id > trans2.id;
        }
        return trans1.exec_date > trans2.exec_date;
    }
};

class Bank{
    private:
    uint32_t id_count = 0;
    bool verbose = false;
    string registration = "";

    //hash table that holds Users
    unordered_map <string, User> customers;
    
    //master list that holds all the past transaction (after execution)
    vector <Transaction> record;

    public:
        //container holding Transactions and sorts by exec date
        priority_queue <Transaction, vector<Transaction>, Transaction_comparator> transfers;

        void get_options(int argc, char** argv);
        void read_in_customers();
    
        //commmand methods
        void login();
        void logout();
        void place();
        uint64_t last_place_date = static_cast<uint64_t>(0);
        void execute();


        //query methods
        void list_transactions(const uint64_t &start, const uint64_t &end);
        void bank_revenue(const uint64_t &start, const uint64_t &end);
        void client_history(const string &user_id);
        void day_summary(const uint64_t &time);
                        
    };
