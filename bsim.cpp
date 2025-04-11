/*
    1st gshare and tounroment are getting the wrong values, 
    goal: call the base class function and add the needed logic to the derived class

*/
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <cmath>
using namespace std;

//use to store file entires 
struct Trace_e{
    unsigned long long pc;
    string branch;
    unsigned long long target;
};

double round3(double val) {
    return std::round(val * 1000.0) / 1000.0;
}

//base class
class StaticTaken{
    protected:
        string name = "Static Taken btb(8)";
        string predictedBranch;
        unsigned long long predictedTarget;
        long numCorrectPred;
        long numCorrectTarget;
        long totalBranch;
        unsigned long long progCounter;
        unsigned long long actualTarget;
        string actualBranch;
        vector <unsigned long long> btb; 
        int btb_size;
        int btb_i;

    public:
        StaticTaken(){
            predictedBranch = "T";
            predictedTarget = 0;
            numCorrectPred = 0;
            numCorrectTarget = 0;
            totalBranch = 0;
            progCounter = 0;
            actualTarget = 0;
            actualBranch = "T";
            btb_size = 8;
            btb.resize(btb_size, 0);
            btb_i = 0;
        }
    
        virtual string prediction(unsigned long long progCounter, const string &actualBranch){
            return "T";
        }

        virtual void update(){

            btb_i = (progCounter  >> 2)% btb_size;  
            unsigned long long predictedTarget;
            if (predictedBranch == "T"){ 
                predictedTarget = btb[btb_i];  
            } else {
                predictedTarget = progCounter + 4; 
            }


            if((actualBranch == "T" && predictedTarget == actualTarget) || (actualBranch == "NT" && predictedTarget == progCounter + 4)){
                numCorrectTarget++;
            }
            if(predictedBranch == actualBranch){
                numCorrectPred++;
            }
            if(actualBranch == "T"){
                btb[btb_i] = actualTarget; 
            }

        }

        virtual void run_sim(const vector<Trace_e>& trace) {

            for(Trace_e t : trace) {
              
                progCounter = t.pc;
                actualBranch = t.branch;
                actualTarget = t.target;
                totalBranch++;

                predictedBranch = prediction(progCounter, actualBranch);
                update();

            }
            return;
        }
        void output_vals(){

            //double branchAccuracy = 100.0 * (static_cast<double>(numCorrectPred) / totalBranch);
            //double targetAccuracy = 100.0 * (static_cast<double>(numCorrectTarget) / totalBranch);

            cout << setw(40) << right << name << ": " << setw(1) << fixed << setprecision(3) << 100.0 * (static_cast<double>(numCorrectPred) / totalBranch) << "%; " << setw(1) << 100.0 * (static_cast<double>(numCorrectTarget) / totalBranch) << "%" <<   endl;
            return;
        }    
};

//DONE
class StaticNotTaken : public StaticTaken{
    public:
        StaticNotTaken(){
            name = "Static Not Taken btb(8)";
            predictedBranch = "NT";
        }
        string prediction (unsigned long long progCounter, const string &actualBranch) override{  
            return "NT";
        }
};

//DONE
class Binomial : public StaticTaken{
    protected:
        vector <bool> binomialTable = vector<bool>(16, true);
        int btable_size;

    public:
        Binomial(){}
        Binomial(int size){
            if (size == 16){
                btb_size = 16;
                name = "Binomial(16) btb(16)";
            }
            else if (size == 32){
                btb_size = 16;
                name = "Binomial(32) btb(16)";
            }
            else if (size == 128){
                btb_size = 128;
                name = "Binomial(128) btb(32)";
            }
            else if (size == 2048){
                btb_size = 64;
                name = "Binomial(2048) btb(64)";
            }
            else
                cout << "Size not in selected testing range" << endl;
            
            binomialTable.resize(size, true);
            btb.resize(btb_size, 0);
            predictedBranch = "T";
            btable_size = size;
        }
         string prediction(unsigned long long progCounter, const string &actualBranch) override{  
            int i = (progCounter >> 2) % btable_size;
            int btb_i = (progCounter >> 2) % btb_size;

            if(binomialTable[i] == true){
                predictedBranch = "T";
            }
            else{
                predictedBranch = "NT";
            }

            //update table 
            if(actualBranch == "T"){
                binomialTable[i] = true;
            }
            else{
                binomialTable[i] = false;
            }
            return predictedBranch;
        }
};

//DONE
class BinomialTwo : public StaticTaken{
    protected: 
        vector <int> twoBitTable = vector <int> (16, 3);
        int btable_size;

    public:
        BinomialTwo(){}
        BinomialTwo(int size, int bsize){
            if (size == 16) {
                btb_size = bsize;
                name = "2Bit Bi(16) btb(16)";
            } else if (size == 32) {
                btb_size = bsize;
                name = "2Bit Bi(32) btb(16)";
            } else if (size == 128) {
                btb_size = bsize;
                name = "2Bit Bi(128) btb(32)";
            } else if (size == 1024) {
                btb_size = bsize;
                name = "2Bit Bi(1024) btb(32)";
            } else if (size == 2048) {
                btb_size = bsize;
                name = "2Bit Bi(2048) btb(64)";
            }
            twoBitTable.resize(size, 3);
            btb.resize(size, 0);
            predictedBranch = "T";
            btable_size = size;
        }

        string prediction(unsigned long long progCounter, const string &actualBranch) override{
            //index into table 
            int i = (progCounter >> 2) % btable_size;
            //get prediction
            if (twoBitTable[i] >= 2){
                predictedBranch = "T";
            }
            else{
                predictedBranch = "NT";
            }
            //update table 
            if(actualBranch == "T" && twoBitTable[i] < 3){
                twoBitTable[i]++;
            }
            else if(actualBranch == "NT" && twoBitTable[i] > 0){
                twoBitTable[i]--;
            }
            return predictedBranch;
        }
};

class Correlated : public StaticTaken{
    protected: 
        //declare the binomial predcitors 
        BinomialTwo one;
        BinomialTwo two;
        BinomialTwo three;
        BinomialTwo four;
        int branch_history = 0;

    public: 
        Correlated(){}
        //size is the size of the binomial predictors
        //intialize here to fic the memory error
        Correlated(int size) : one(size, btb_size), two(size, btb_size), three(size, btb_size), four(size, btb_size){
            if (size == 16) {
                btb_size = 32;
                name = "Corr(4): 2Bit Bi(16) btb(32)";
            } else if (size == 1024) {
                btb_size = 32;
                name = "Corr(4): 2Bit Bi(1024) btb(32)";
            } else {
                cout << "Size not in selected testing range for correlated predictor" << endl;
            }
            

            //CHANGED BTB HERE
            btb.resize(btb_size, 0);
        }
        string prediction(unsigned long long progCounter, const string &actualBranch) override{
            //use the history to select the predictor 
            if (branch_history == 0) {
                predictedBranch = one.prediction(progCounter, actualBranch);
            } else if (branch_history == 1) {
                predictedBranch = two.prediction(progCounter, actualBranch);
            } else if (branch_history == 2) {
                predictedBranch = three.prediction(progCounter, actualBranch);
            } else if (branch_history == 3) {
                predictedBranch = four.prediction(progCounter, actualBranch);
            }
            return predictedBranch;
        }
        void update()override{
                StaticTaken :: update();
                branch_history = branch_history << 1;

                //if true we add 1
                if(actualBranch == "T"){
                    branch_history |= 1;
                }

                //clear unneeeded history
                branch_history = branch_history & 0b11;
        }
};

class Gshare : public StaticTaken{
    protected:
        //always 2048 mask is variable based on the bit size of the predicitor 
        vector <int> gshare_table;
        int gshare_mask;
        int history_register = 0;
        int gshare_i;
        int table_size;
    public:
        Gshare(){}
        Gshare(int size, int btbs){
            if (size == 3) {
                name = "Gshare(3 bit) btb(16)";
                gshare_mask = 0x00000007;
            } else if (size == 4) {
                name = "Gshare(4 bit) btb(32)";
                gshare_mask = 0x0000000F;
            } else if (size == 5) {
                name = "Gshare(5 bit) btb(32)";
                gshare_mask = 0x0000001F;
            } else if (size == 10) {
                name = "Gshare(10 bit) btb(64)";
                gshare_mask = 0x000003FF;
            } 
            gshare_table.resize(2048, 3);
            btb_size = btbs;
            btb.resize(btb_size, 0);
            //gshare_mask = (1 << size) - 1 ;
            table_size = size;
        }

        string prediction(unsigned long long progCounter, const string &actualBranch) override{
            //correct get the number bits
            int masked = history_register & gshare_mask;

            int pc_shift = progCounter >> 2;
            
            gshare_i = (pc_shift ^ masked) & 0x3FF;

            if(gshare_table[gshare_i] >= 2){
                predictedBranch = "T";
            }
            else{
                predictedBranch = "NT";
            }
            return predictedBranch;
        }
        void update() override{
                StaticTaken :: update();
                //update the gshare table
                if(actualBranch == "T" && gshare_table[gshare_i] < 3){
                    gshare_table[gshare_i]++;
                }
                else if(actualBranch == "NT" && gshare_table[gshare_i] > 0){
                    gshare_table[gshare_i]--;
                }

                //update histroy register, get rid of last predcition
                history_register = history_register << 1;
                if(actualBranch == "T"){
                    history_register |= 1;
                }
                //clean history
                history_register = history_register & gshare_mask;                
            return;
        }            
};

class Tournament : public StaticTaken{
    private:
        BinomialTwo bi;
        Gshare gsh;
        vector <int> t_table;
        int tournament_i;
        string bpred, gpred;
        
    public:
        Tournament() : bi(2048, 64), gsh(4, 64){
            name = "T-2Bit Bi(2048)/Gshare(4 bit) btb(64)";
            t_table.resize(2048, 3);
            btb_size = 64;
            btb.resize(btb_size, 0);
        }

        string prediction(unsigned long long progCounter, const string &actualBranch) override{
            tournament_i = (progCounter >> 2) % 2048;

            gpred = gsh.prediction(progCounter, actualBranch);
            bpred = bi.prediction(progCounter, actualBranch);

            if(t_table[tournament_i] >= 2){
                predictedBranch = bpred;
            }
            else{
                predictedBranch = gpred;
            }
            return predictedBranch;
        }

        void update()override{
            StaticTaken :: update();

            if(bpred == actualBranch && gpred != actualBranch){
                if(t_table[tournament_i] < 3)
                    t_table[tournament_i]++;
            }
            else if(bpred != actualBranch && gpred == actualBranch){
                if(t_table[tournament_i] > 0)
                    t_table[tournament_i]--;
            }
        }
};

template<typename PredictorType>
void run(PredictorType& pred, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening " << filename << endl;
        return;
    }
    pred.run_sim(file);
    pred.output_vals();
}

int main(int argc, char *argv[]){
    //store the entries in a vector 
    vector<Trace_e> trace;
    string line;
    while (getline(cin, line)) {
        stringstream s(line);

        unsigned long long pc, target;
        string behavior;

        s >> hex >> pc >> behavior >> hex >> target;
        trace.push_back({pc, behavior, target});
    }

    StaticTaken pred;
    StaticNotTaken pred2;
    Binomial Bi16(16);
    Binomial Bi32(32);
    Binomial Bi128(128);
    Binomial Bi2048(2048);
    BinomialTwo Bi16Two(16, 16);
    BinomialTwo Bi32Two(32, 16);
    BinomialTwo Bi128Two(128, 32);
    BinomialTwo Bi2048Two(2048, 64);
    Correlated Cor16(16);
    Correlated Cor1024(1024);
    Gshare gshare(3, 16);
    Gshare gshare2(4, 32);
    Gshare gshare3(5, 32);
    Gshare gshare4(10, 64);
    Tournament t;

    //taken
    pred.run_sim(trace);
    pred.output_vals();

    //not takem
    pred2.run_sim(trace);
    pred2.output_vals();
    
    //binomial
    Bi16.run_sim(trace);
    Bi16.output_vals();
    Bi32.run_sim(trace);
    Bi32.output_vals();
    Bi128.run_sim(trace);
    Bi128.output_vals();
    Bi2048.run_sim(trace);
    Bi2048.output_vals();
    //2 bit
    Bi16Two.run_sim(trace);
    Bi16Two.output_vals();
    Bi32Two.run_sim(trace);
    Bi32Two.output_vals();
    Bi128Two.run_sim(trace);
    Bi128Two.output_vals();
    Bi2048Two.run_sim(trace);
    Bi2048Two.output_vals();
    
    //correlated, both have an issue 
    Cor16.run_sim(trace);
    Cor16.output_vals();
    Cor1024.run_sim(trace);
    Cor1024.output_vals();
    
    //gshare
    gshare.run_sim(trace);
    gshare.output_vals();
    gshare2.run_sim(trace);
    gshare2.output_vals();
    gshare3.run_sim(trace);
    gshare3.output_vals();
    gshare4.run_sim(trace);
    gshare4.output_vals();

    //tournament
    t.run_sim(trace);
    t.output_vals();
    
    return 0;
}