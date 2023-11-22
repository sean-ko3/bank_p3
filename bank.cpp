//Project Identifier: 292F24D17A4455C1B5133EDD8C7CEAA0C9570A98
#include "bank.h"
#include <iostream>
#include <string>
#include <vector>

uint64_t convert_time(const string &timestamp) {
    size_t begin = 0;
    size_t end = timestamp.find(':');
    std::vector<uint64_t> digits;
    digits.reserve(6);

    while (end != std::string::npos) {
        digits.push_back(std::stoull(timestamp.substr(begin, end - begin)));
        begin = end + 1;
        end = timestamp.find(':', begin);
    }

    // Add the last part of the timestamp
    digits.push_back(std::stoull(timestamp.substr(begin)));

    // Calculate the uint64_t value
    uint64_t result = 0;
    result += digits[0] * 10000000000ULL;
    result += digits[1] * 100000000ULL;
    result += digits[2] * 1000000ULL;
    result += digits[3] * 10000ULL;
    result += digits[4] * 100ULL;
    result += digits[5];

    return result;
}




void Bank::get_options(int argc, char** argv){
    int choice;
    int option_index = 0;

    option long_options[]{
        {"help", no_argument, nullptr, 'h'},
        {"verbose", no_argument, nullptr, 'v'},
        {"file", required_argument, nullptr, 'f'},
    }; //long options

    bool help = false;
    while((choice = getopt_long(argc, argv, "hvf:", long_options, &option_index)) != -1){
        switch(choice){
            case 'h':
                //help message
                help = true;
                cout << "This is help message. You need a registration file and a command file to get started";
                exit(0);
            
            case 'v':
                verbose = true;
                break;
            
            case 'f':
                registration = optarg;
                break;
        }
    }
    if(registration.empty() && help == false){
        cerr << "Need to provide file name\n";
        exit(1);
    }
}
//Error: Reading from cin has failed
void Bank::read_in_customers(){
    
    ifstream fin; 
    fin.open(registration);
    if(!fin.is_open()){
        cerr << "Registration file failed to open.\n";
        exit(0);
    }

    string time_st;
    string id;
    string pin;
    string balance;
    
    while(getline(fin, time_st, '|')){
       
       getline(fin, id, '|');
       getline(fin, pin, '|');
       getline(fin, balance);

       User new_user;
       //changing time_st into uint64
        new_user.reg_time = convert_time(time_st);
        new_user.userID = id;
        new_user.pin = static_cast<uint32_t>(stoul(pin));
        new_user.balance = static_cast<uint32_t>(stoul(balance));

        customers[new_user.userID] = new_user;

    }
    fin.close();
    
}

void Bank::login(){
    //first break the line 
    string id;
    uint32_t pass_atmpt;
    string IP_addy;
    cin >> id;
    cin >> pass_atmpt;

    auto it = customers.find(id);
    if(it != customers.end() && it->second.pin == pass_atmpt){
        cin >> IP_addy;
        it->second.validIPs.insert(IP_addy);

        if(verbose){
            cout << "User " << id << " logged in.\n";
        }
    }
    else{
        if(verbose){
           cout << "Failed to log in " << id << ".\n"; 
        }
        cin >> IP_addy;
    }


}

void Bank::logout(){
    string id;
    string IP_addy;
    
    cin >> id;
    cin >> IP_addy;

    auto it = customers.find(id);
    
        if(it != customers.end()){
            auto it2 = it->second.validIPs.find(IP_addy);
            if(it2 != it->second.validIPs.end()){
            
                it->second.validIPs.erase(IP_addy);

                if(verbose){
                    cout << "User " << it->second.userID << " logged out.\n";
                }
            }
            else{
                if(verbose){
                cout << "Failed to log out " << it->second.userID << ".\n";
                }
            }
        }
        else{
            if(verbose){
            cout << "Failed to log out " << it->second.userID << ".\n";
            }
        }
    
}

void Bank::place(){
    string time_st, execdate,  IP_addy, sender_id, receive_id;
    uint32_t amount;
    string fee;
    char fee_cover;

    cin >> time_st >> IP_addy >> sender_id >> receive_id
     >> amount >> execdate >> fee_cover;
    
    uint64_t timestamp = convert_time(time_st);
    uint64_t exe_date = convert_time(execdate);

    //Error check
    if(timestamp > exe_date){
        cerr << "You cannot have an execution date before the current timestamp.\n";
        exit(1);
    }

    if(timestamp < last_place_date){
        cerr << "Invalid decreasing timestamp in 'place' command.\n";
        exit(1);
    } // END OF ERROR CHECKS

    auto sender = customers.find(sender_id);
    auto recipient = customers.find(receive_id);
    
    
    if(exe_date - timestamp > 3000000){
        if(verbose){
            cout << "Select a time less than three days in the future.\n";
        }
    }
    else if(sender == customers.end()){
        if(verbose){
            //sender not found
            cout << "Sender " << sender_id  << " does not exist.\n";
        }
    }
    else if(recipient == customers.end()){
        if(verbose){
            //recipient not found
            cout << "Recipient " << receive_id << " does not exist.\n";
        }
    }
    else if(exe_date < sender->second.reg_time || exe_date < recipient->second.reg_time){
        if(verbose){
            //exe date not greater than sender and registration time
            cout << "At the time of execution, sender and/or recipient have not registered.\n";
        }     
    }
    else if((sender->second.validIPs.empty())){
            //sender not logged in
            if(verbose){
                cout << "Sender " << sender_id << " is not logged in.\n";
            }
    }
    else if(sender->second.validIPs.find(IP_addy) == sender->second.validIPs.end()){
        if(verbose){
            //IP address not found
            cout << "Fraudulent transaction detected, aborting request.\n";
        }
    }
    else{
            Transaction new_transaction;//  {timestamp, IP_addy, sender_id, receive_id, amount, exe_date, bool, id, fee};
            new_transaction.timestamp = timestamp;
            new_transaction.IPaddress = IP_addy;
            new_transaction.senderID = sender_id;
            new_transaction.recipientID = receive_id;
            new_transaction.amount = amount;
            new_transaction.exec_date = exe_date;
            new_transaction.fee_shared = fee_cover;
            new_transaction.id = id_count;
            id_count ++;

            new_transaction.fee = (new_transaction.amount / static_cast<uint32_t>(100));
            //bound 10 <= fee <= 
            new_transaction.fee = min(new_transaction.fee, static_cast<uint32_t> (450));
            new_transaction.fee = max(new_transaction.fee, static_cast<uint32_t>(10));
            
            //checking is sender is loyal customer or not
            if(new_transaction.exec_date - sender->second.reg_time > static_cast<uint64_t>(50000000000)){
                new_transaction.fee = static_cast<uint32_t>((new_transaction.fee * static_cast<uint32_t>(3)) / static_cast<uint32_t>(4));   
            }
                            
            last_place_date = new_transaction.timestamp;
            transfers.push(new_transaction);

            //executing any transactions that should have been executed before the current transaction is placed 
                            
            while(!transfers.empty() && new_transaction.timestamp >= transfers.top().exec_date){
                execute();
            }
            
            if(verbose){
                cout << "Transaction placed at " << timestamp << ": " << "$" << amount << " from "
                        << sender_id << " to " << receive_id << " at " << exe_date << ".\n";
            }
                            
                            
        }

}

void Bank::execute(){
    Transaction transaction_exec = transfers.top();
    transfers.pop();
    auto sender = customers.find(transaction_exec.senderID);
    auto recipient = customers.find(transaction_exec.recipientID);
    
   if(sender == recipient){
    transaction_exec.fee_shared = 'o';
   }
    //case fee is shared
    if(transaction_exec.fee_shared == 's'){
        if(recipient->second.balance < (transaction_exec.fee / static_cast<uint32_t>(2)) 
            || sender->second.balance < ((transaction_exec.fee - (transaction_exec.fee / static_cast<uint32_t>(2))) + transaction_exec.amount)){
                if(verbose){
                cout << "Insufficient funds to process transaction " << transaction_exec.id << ".\n";
                }
        }
        else{
              //update balances
                recipient->second.balance -= (transaction_exec.fee / static_cast<uint32_t>(2));
                recipient->second.balance += transaction_exec.amount;

                sender->second.balance -= (transaction_exec.fee - (transaction_exec.fee / static_cast<uint32_t>(2)));
                sender->second.balance -= transaction_exec.amount;

                sender->second.outgoing++;
                recipient->second.incoming++;
                
                if(verbose){
                cout << "Transaction executed at " << transaction_exec.exec_date << ": $" << transaction_exec.amount
                << " from " << transaction_exec.senderID << " to " << transaction_exec.recipientID << ".\n";
                }
                record.push_back(transaction_exec);  
            }
    }
    //case sender takes all of it 
    if(transaction_exec.fee_shared == 'o'){
        if(sender->second.balance < (transaction_exec.fee + transaction_exec.amount)){
            if(verbose){
            cout << "Insufficient funds to process transaction " << transaction_exec.id << ".\n";
            }   
        }
        else{
            //update balances
            sender->second.balance -= (transaction_exec.fee + transaction_exec.amount);
            recipient->second.balance += transaction_exec.amount;

            sender->second.outgoing++;
            recipient->second.incoming++;

            if(verbose){
            cout << "Transaction executed at " << transaction_exec.exec_date << ": $" << transaction_exec.amount
                << " from " << transaction_exec.senderID << " to " << transaction_exec.recipientID << ".\n";
            //push back to record keep track all of our executed transaction
            }
             record.push_back(transaction_exec);     
        }
    }
}

struct query_comparator{
    bool operator()(const Transaction &t1, const Transaction &t2){
        return t1.exec_date <= t2.exec_date;
    }
};

void Bank::list_transactions(const uint64_t &start, const uint64_t &end){
    Transaction comparator;
    comparator.exec_date = start;

    auto it = lower_bound(record.begin(), record.end(), comparator, query_comparator());

    int total_transactions = 0;
    
        while(it != record.end() && it->exec_date < end){
            total_transactions++;
            cout << it->id << ": " << it->senderID << " sent " << it->amount;
            if(it->amount != static_cast<uint32_t>(1)){
                cout << " dollars to " << it->recipientID << " at " << it->exec_date << ".\n";
            }
            else{
                cout << " dollar to " << it->recipientID << " at " << it->exec_date << ".\n";
            }
            it++;
        }
    
    cout << "There ";
     if(total_transactions != 1){
        cout << "were " << total_transactions<< " transactions that were placed between time " << start << " to " << end << ".\n";
     }
     else{
        cout << "was 1 transaction that was placed between time " << start << " to " << end << ".\n";
     }
     
}

void Bank::bank_revenue(const uint64_t &start, const uint64_t &end){
    uint32_t total = 0;

    Transaction comparator;
    comparator.exec_date = start;

    auto it = lower_bound(record.begin(), record.end(), comparator, query_comparator());

    while(it != record.end() && it->exec_date < end){
        total += it->fee;
        it++;
    }

    cout << "281Bank has collected " << total << " dollars in fees over";
    uint64_t time_diff = end - start;
    
    //years
    if(time_diff / static_cast<uint64_t>(10000000000) != 0){
        if(time_diff / static_cast<uint64_t>(10000000000) != 1){
        cout << " " << time_diff / static_cast<uint64_t>(10000000000) << " years";
        }
        else{
             cout << " 1 year";
            
        } 
        time_diff -= ((time_diff /  static_cast<uint64_t>(10000000000)) * static_cast<uint64_t>(10000000000));
    }
    //months
    if(time_diff / static_cast<uint64_t>(100000000) != 0){
        if(time_diff / static_cast<uint64_t>(100000000) != 1){
        cout << " " << time_diff / static_cast<uint64_t>(100000000) << " months";
        }
        else{
             cout << " 1 month";
        }
        time_diff -= ((time_diff /  static_cast<uint64_t>(100000000)) * static_cast<uint64_t>(100000000));
    }

    //days
     if(time_diff / static_cast<uint64_t>(1000000) != 0){
        if(time_diff / static_cast<uint64_t>(1000000) != 1){
        cout << " " << time_diff / static_cast<uint64_t>(1000000) << " days";
        }
        else{
             cout << " 1 day";
        }
        time_diff -= ((time_diff /  static_cast<uint64_t>(1000000)) * static_cast<uint64_t>(1000000));
    }
    
    //hours
    if(time_diff / static_cast<uint64_t>(10000) != 0){
        if(time_diff / static_cast<uint64_t>(10000) != 1){
        cout << " " << time_diff / static_cast<uint64_t>(10000) << " hours";
        }
        else{
             cout << " 1 hour";
        }
        time_diff -= ((time_diff /  static_cast<uint64_t>(10000)) * static_cast<uint64_t>(10000));
    }

    //minutes
     if(time_diff / static_cast<uint64_t>(100) != 0){
        if(time_diff / static_cast<uint64_t>(100) != 1){
        cout << " " << time_diff / static_cast<uint64_t>(100) << " minutes";
        }
        else{
             cout << " 1 minute";
             
        }
        time_diff -= ((time_diff /  static_cast<uint64_t>(100)) * static_cast<uint64_t>(100));
    }
    
    //seconds  
     if(time_diff != 0){
        if(time_diff  != 1){
            cout << " " << time_diff << " seconds";
        }
        else{
             cout << " " << "1 second" ;
        }
    }

    cout << ".\n";
}

void Bank::client_history(const string &user_id){
    auto it = customers.find(user_id);
    if(it == customers.end()){
        cout << "User " << user_id << " does not exist.\n";

            return;
    }

    cout << "Customer " << user_id << " account summary:\n";
    cout << "Balance: $" << it->second.balance << "\n";

    uint32_t total = (it->second.incoming + it->second.outgoing);
    
    cout << "Total # of transactions: " << total << "\n";
    cout << "Incoming " << it->second.incoming << ":\n";
   
    vector<Transaction> incoming;
    vector<Transaction> outgoing;

    auto in = record.rbegin();
    while((incoming.size() < 10 || outgoing.size() < 10) && in != record.rend()){
        if(in->recipientID == user_id && incoming.size() < 10){
            incoming.push_back(*in);       
        }
        if(in->senderID == user_id && outgoing.size() < 10){
            outgoing.push_back(*in);
        }
        in++;
    
    }
    if(!incoming.empty()){
        
        reverse(incoming.begin(),incoming.end());
        
        for(Transaction trans: incoming){
       
            cout << trans.id << ": " << trans.senderID << " sent " << trans.amount;
            if(trans.amount != static_cast<uint32_t>(1)){
                cout << " dollars to " << trans.recipientID << " at " << trans.exec_date << ".\n";
            }
            else{
                cout << " dollar to " << trans.recipientID << " at " << trans.exec_date << ".\n";
            }
        
        }
    }
    cout << "Outgoing " << it->second.outgoing << ":\n";

    if(!outgoing.empty()){
        reverse(outgoing.begin(), outgoing.end());
        for(Transaction trans: outgoing){
        
        cout << trans.id << ": " << trans.senderID << " sent " << trans.amount;
        if(trans.amount != static_cast<uint32_t>(1)){
            cout << " dollars to " << trans.recipientID << " at " << trans.exec_date << ".\n";
        }
        else{
            cout << " dollar to " << trans.recipientID << " at " << trans.exec_date << ".\n";
        }
        
    
    }
    }
    

    
}

void Bank::day_summary(const uint64_t &time){
    uint64_t start = (time / static_cast<uint64_t>(1000000)) * static_cast<uint64_t>(1000000);
    uint64_t end = start + 1000000;
    cout << "Summary of [" << start << ", " << end << "):\n";
   
    Transaction comparator;
   
    comparator.exec_date = start;
    comparator.id = 0;
    

    auto it = lower_bound(record.begin(), record.end(), comparator, query_comparator());

    int total_transactions = 0;
    
    while(it != record.end() && it->exec_date < end){
        total_transactions++;
        cout << it->id << ": " << it->senderID << " sent " << it->amount;
        if(it->amount != static_cast<uint32_t>(1)){
            cout << " dollars to " << it->recipientID << " at " << it->exec_date << ".\n";
        }
        else{
            cout << " dollar to " << it->recipientID << " at " << it->exec_date << ".\n";
        }
        it++;
    }
        
    
    if(total_transactions != 1){
       cout << "There were a total of " << total_transactions << " transactions, 281Bank has collected ";
    }
    else{
        cout << "There was a total of " << total_transactions << " transaction, 281Bank has collected ";
    }
    
    uint32_t total = 0;
    it = lower_bound(record.begin(), record.end(), comparator, query_comparator());
    while(it != record.end() && it->exec_date < end){
        total += it->fee;
        it++;
    }

    cout << total << " dollars in fees.\n";
    
    
    
}


int main(int argc, char ** argv){
    std::ios_base::sync_with_stdio(false);
    Bank bank;
    bank.get_options(argc, argv);
    bank.read_in_customers();
    //done with reading in client data

    string cur_string;
    
    //while loop to read commands
    while(cin >> cur_string){

        if(cur_string[0] == '#'){ //you throw away 
            getline(cin, cur_string);
        }
        else if(cur_string == "login"){ //login command
            bank.login();
        }
        else if(cur_string == "out"){ //log out
            bank.logout();
        }
        else if(cur_string == "place"){
            bank.place();//the place command
        }
        else{
            //you hit "$$$" so execute every transaction
            while(!bank.transfers.empty()){
                bank.execute();
            }
            break;
        }
    }
        //while loop to read queries
    
    while(cin >> cur_string){

        if(cur_string == "l"){
            string start;
            string end;
            cin >> start >> end;
            uint64_t start_time = convert_time(start);
            uint64_t end_time = convert_time(end);
            bank.list_transactions(start_time, end_time);
        }
        else if(cur_string == "r"){
            string start;
            string end;
            cin >> start >> end;
            uint64_t start_time = convert_time(start);
            uint64_t end_time = convert_time(end);
            bank.bank_revenue(start_time, end_time);
        }
        else if(cur_string == "h"){
            string userid;
            cin >> userid;
            bank.client_history(userid);
        }
        else if(cur_string == "s"){
            string start;
            cin >> start;
            uint64_t start_time = convert_time(start);
            bank.day_summary(start_time);

        }
    }



    
}
