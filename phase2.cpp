#include <iostream>
#include <bits/stdc++.h>
#include <string>
#include <fstream>
#include <regex>
#include <stack>
#include "phase1.h"
#include "phase2_parser.h"
#include "phase2_leftrec_fact.h"
#include "phase2.h"

using namespace std;

ofstream outfile_syntax;
string syntax_out_filename = "output_syntax.txt";


// phase 3
class stack_node;
class ManualParseTable;
stack<stack_node*> checker_stack;  // number 0
stack<stack_node*> stack_aux;      // number 1



//Data structures declarations
struct production;
struct non_terminal;
class syntax_analyzer;
//Data structures used


//A class to control the syntax analysis operation
/*
    It's main objective is to create the parsing table.
    It contains the data obtained from the file concerning the nonterminals and productions.
    It performs the required algorithms like obtaining the first and follow sets, left factoring and left recursion elimination.
*/

/** it is used for printing both main and aux stack just for testing **/
void printStacks()
{
    stack<stack_node*> temp;
    cout << "checker stack :" << endl << endl;
    while(!checker_stack.empty())
    {
        temp.push(checker_stack.top());
        cout << checker_stack.top()->get_node_name() << endl;
        checker_stack.pop();
    }
    while(!temp.empty())
    {
        checker_stack.push(temp.top());
        temp.pop();
    }
    cout << endl << "auxilary stack :" << endl << endl;
    while(!stack_aux.empty())
    {
        temp.push(stack_aux.top());
        cout << stack_aux.top()->get_node_name() << endl;
        stack_aux.pop();
    }
    while(!temp.empty())
    {
        stack_aux.push(temp.top());
        temp.pop();
    }
}




/*** for parsing table phase3 ***/





/** it returns the RHS of each non terminal after inserting the needed actions**/
vector<string> get_semantic_action(string nonterm_name,vector<string>prods_RHS)
{
    vector <string> prods_with_actions;
    if(nonterm_name == "EXPRESSION"){
       string action = string("{if(AUX[0].op = none) AUX[2].op = AUX[1].op , AUX[2].type = AUX[1].type ")
        + string("# if(AUX[0].op != none) AUX[2].op = AUX[0].op , AUX[2].type = boolean ")
        + string("# AUX[2].code = AUX[1].code + newline + AUX[0].code # pop(2)}");

        prods_with_actions = prods_RHS;
        prods_with_actions.push_back(action);
    }
    else if(nonterm_name == "EXPRESSION1"){
        if(prods_RHS.size() == 2){  /** relop simple_expression **/
           string action = string("{AUX[2].op = AUX[1].value # AUX[2].type = boolean ")
           + string("# AUX[2].code = AUX[0].code + newline ")
           + string("+ if(AUX[0].type = int) Address $ 0 : icmpl; + if(AUX[0].type = float) Address $ 0 : fcmpl; ")
           + string("# Address++ # pop(2)}");

           prods_with_actions = prods_RHS;
           prods_with_actions.push_back(action);
        }
        else { /** epsilon **/
           string action = "{AUX[0].type = none # AUX[0].op = none # AUX[0].code = none}";

           prods_with_actions = prods_RHS;
           prods_with_actions.push_back(action);
        }
    }
    else if(nonterm_name == "SIMPLE_EXPRESSION"){
        if(prods_RHS.size() == 2){  /** term simple_expression' **/
           string action1 =
           string ("{S[0].intype = AUX[0].type # S[0].inop = AUX[0].op # S[0].incode = AUX[0].code}");

           string action2 =
           string("{AUX[2].type = AUX[0].type # AUX[2].op = AUX[0].op # AUX[2].code = AUX[0].code # pop(2)}");

           prods_with_actions.push_back(prods_RHS[0]);
           prods_with_actions.push_back(action1);
           prods_with_actions.push_back(prods_RHS[1]);
           prods_with_actions.push_back(action2);


        }
        else {  /** sign term simple_expression' **/
           string action1 =
           string("{S[0].intype = AUX[0].type # S[0].inop = AUX[0].op # S[0].incode = AUX[0].code}");

           string action2 = string("{AUX[3].type = AUX[0].type # AUX[3].op = mul ")
           + string("# AUX[3].code = AUX[0].code ")
           + string("+ if (AUX[2].type = minus) newline , Address $ 0 : iconst-1 ; , newline , Address $ 1 : imul ; ")
           + string("# Address++ # pop(3)}");

           prods_with_actions.push_back(prods_RHS[0]);
           prods_with_actions.push_back(prods_RHS[1]);
           prods_with_actions.push_back(action1);
           prods_with_actions.push_back(prods_RHS[2]);
           prods_with_actions.push_back(action2);

        }
    }
    else if(nonterm_name == "SIMPLE_EXPRESSION'"){
        if(prods_RHS.size() == 3){  /** addop term simple_expression' **/

           string action1 = string("{S[0].intype = AUX[2].intype # S[0].inop = AUX[2].inop ")
           + string("# S[0].incode = AUX[2].incode + newline + AUX[0].code + newline ")
           + string("+ if(AUX[1].value = add & AUX[0].type = int) Address $ 0 : iadd ; ")
           + string("+ if(AUX[1].value = add & AUX[0].type = float) Address $ 0 : fadd ; ")
           + string("+ if(AUX[1].value = sub & AUX[0].type = int) Address $ 0 : isub ; ")
           + string("+ if(AUX[1].value = sub & AUX[0].type = float) Address $ 0 : fsub ; }");

           string action2 =
             string("{AUX[3].code = AUX[0].code # AUX[3].op = AUX[2].value # AUX[3].type = AUX[1].type # Address++ # pop(3)}");

           prods_with_actions.push_back(prods_RHS[0]);
           prods_with_actions.push_back(prods_RHS[1]);
           prods_with_actions.push_back(action1);
           prods_with_actions.push_back(prods_RHS[2]);
           prods_with_actions.push_back(action2);


        }
        else {  /** epsilon **/
          string action = "{AUX[0].type = AUX[0].intype # AUX[0].op = AUX[0].inop # AUX[0].code = AUX[0].incode}";
           prods_with_actions = prods_RHS;
           prods_with_actions.push_back(action);
        }
    }
    else if(nonterm_name == "TERM"){
        string action1 =
         string("{S[0].inop = AUX[0].op # S[0].intype = AUX[0].type # S[0].incode = AUX[0].code}");

        string action2 =
         string("{AUX[2].type = AUX[0].type # AUX[2].op = AUX[0].op # AUX[2].code = AUX[0].code # pop(2)}");

           prods_with_actions.push_back(prods_RHS[0]);
           prods_with_actions.push_back(action1);
           prods_with_actions.push_back(prods_RHS[1]);
           prods_with_actions.push_back(action2);

    }
    else if(nonterm_name == "TERM'"){
        if(prods_RHS.size() == 3){ /** mulop factor term' **/
          string action1 = string("{S[0].intype = AUX[2].intype # S[0].inop = AUX[2].inop ")
           + string("# S[0].incode = AUX[2].incode + newline + AUX[0].code + newline ")
           + string("+ if(AUX[1].value = mul & AUX[0].type = int) Address $ 0 : imul ; ")
           + string("+ if(AUX[1].value = mul & AUX[0].type = float) Address $ 0 : fmul ; ")
           + string("+ if(AUX[1].value = div & AUX[0].type = int) Address $ 0 : idiv ; ")
           + string("+ if(AUX[1].value = div & AUX[0].type = float) Address $ 0 : fdiv ; }");

           string action2 =
            string("{AUX[3].type = AUX[1].type # AUX[3].op = AUX[2].value # AUX[3].code = AUX[0].code # Address++ # pop(3)}");

           prods_with_actions.push_back(prods_RHS[0]);
           prods_with_actions.push_back(prods_RHS[1]);
           prods_with_actions.push_back(action1);
           prods_with_actions.push_back(prods_RHS[2]);
           prods_with_actions.push_back(action2);

        }
        else{  /** epsilon **/
          string action = "{AUX[0].code = AUX[0].incode # AUX[0].op = AUX[0].inop # AUX[0].type = AUX[0].intype}";
           prods_with_actions = prods_RHS;
           prods_with_actions.push_back(action);
        }
    }
    else if(nonterm_name == "FACTOR"){
        if(prods_RHS.size() == 1 && prods_RHS[0] == "num"){
          string action = string ("{AUX[1].type = AUX[0].type ")
          + string("# AUX[1].code = if(AUX[0].type = int) Address $ 0 : iconst AUX[0].value ; ")
          + string("+ if(AUX[0].type = float) Address $ 0 : fconst AUX[0].value ; ")
          + string("# AUX[1].op = none # Address++ # pop(1)}");

          prods_with_actions = prods_RHS;
          prods_with_actions.push_back(action);
        }
        else if(prods_RHS.size() == 1 && prods_RHS[0] == "id"){
           string action = string("{AUX[1].type = AUX[0].type # AUX[1].op = none ")
          + string("# AUX[1].code = if(AUX[0].type = int) Address $ 0 : iload AUX[0].value ; ")
          + string("+ if(AUX[0].type = float) Address $ 0 : fload AUX[0].value ; # Address++ # pop(1)}");

          prods_with_actions = prods_RHS;
          prods_with_actions.push_back(action);
        }
        else{ /** ( expression ) **/
          string action = "{AUX[3].type = AUX[1].type # AUX[3].op = AUX[1].op # AUX[3].code = AUX[1].code # pop(3)}";

           prods_with_actions = prods_RHS;
           prods_with_actions.push_back(action);
        }
    }
    else if(nonterm_name == "SIGN"){
        if(prods_RHS.size() == 1 && prods_RHS[0] == "plus"){
          string action = "{AUX[1].type = plus # pop(1)}";
          prods_with_actions = prods_RHS;
           prods_with_actions.push_back(action);
        }
        else if(prods_RHS.size() == 1 && prods_RHS[0] == "minus"){
            string action = "{AUX[1].type = minus # pop(1)}";
            prods_with_actions = prods_RHS;
           prods_with_actions.push_back(action);
        }
    }
    else if(nonterm_name == "DECLARATION"){
        string action = "{AUX[1].type = AUX[2].type # pop(3)}";
        prods_with_actions = prods_RHS;
           prods_with_actions.push_back(action);
    }
    else if(nonterm_name == "PRIMITIVE_TYPE"){
        if(prods_RHS.size() == 1 && prods_RHS[0] == "int"){
           string action = "{AUX[1].type = int # pop(1)}";
           prods_with_actions = prods_RHS;
           prods_with_actions.push_back(action);
        }
        else if(prods_RHS.size() == 1 && prods_RHS[0] == "float"){
           string action = "{AUX[1].type = float # pop(1)}";
           prods_with_actions = prods_RHS;
           prods_with_actions.push_back(action);
        }
    }
    else if(nonterm_name == "METHOD_BODY"){
       string action = "{AUX[1].code = AUX[0].code # print(AUX[1].code); # pop(1)}";
       prods_with_actions = prods_RHS;
       prods_with_actions.push_back(action);
    }
    else if(nonterm_name == "STATEMENT_LIST"){
       string action1 = "{S[0].incode = AUX[0].code}";
       string action2 = "{AUX[2].code = AUX[0].code # pop(2)}";
           prods_with_actions.push_back(prods_RHS[0]);
           prods_with_actions.push_back(action1);
           prods_with_actions.push_back(prods_RHS[1]);
           prods_with_actions.push_back(action2);
    }
    else if(nonterm_name == "STATEMENT"){
        if(prods_RHS.size() == 1 && prods_RHS[0] == "DECLARATION"){
            string action = "{AUX[1].code = none # pop(1)}";
            prods_with_actions = prods_RHS;
            prods_with_actions.push_back(action);
        }
        else if(prods_RHS.size() == 1 && prods_RHS[0] == "ASSIGNMENT"){
            string action = "{AUX[1].code = AUX[0].code # pop(1)}";
            prods_with_actions = prods_RHS;
            prods_with_actions.push_back(action);
        }
        else if(prods_RHS.size() == 1 && prods_RHS[0] == "IF"){
            string action = "{AUX[1].code = AUX[0].code # pop(1)}";
            prods_with_actions = prods_RHS;
            prods_with_actions.push_back(action);
        }
        else if(prods_RHS.size() == 1 && prods_RHS[0] == "WHILE"){
            string action = "{AUX[1].code = AUX[0].code # pop(1)}";
            prods_with_actions = prods_RHS;
            prods_with_actions.push_back(action);
        }

    }
    else if(nonterm_name == "STATEMENT_LIST'"){
        if(prods_RHS.size() == 2){ /** statement statement_list' **/
           string action1 = "{S[0].incode = AUX[1].incode + newline + AUX[0].code}";
           string action2 = "{AUX[2].code = AUX[0].code # pop(2)}";

           prods_with_actions.push_back(prods_RHS[0]);
           prods_with_actions.push_back(action1);
           prods_with_actions.push_back(prods_RHS[1]);
           prods_with_actions.push_back(action2);

        }
        else{  /** epsilon **/
            string action = "{AUX[0].code = AUX[0].incode}";
            prods_with_actions = prods_RHS;
            prods_with_actions.push_back(action);
        }
    }
    else if(nonterm_name == "ASSIGNMENT"){
        string action = string("{AUX[4].code = AUX[1].code + newline ")
        + string("+ if(AUX[3].type = int) Address $ 0 : istore_AUX[3].value ; ")
        + string("+ if(AUX[3].type = float) Address $ 0 : fstore_AUX[3].value ; # Address++ # pop(4)}");

        prods_with_actions = prods_RHS;
        prods_with_actions.push_back(action);
    }
    else if(nonterm_name == "IF"){

     string action1 = string("{AUX[4].code = AUX[1].code + newline + Address % 0  + if_ + AUX[1].op + label + newline # Address++ }");
     string action2 = string("{AUX[7].code = AUX[7].code + AUX[1].code + newline + Address % 0  + goto + label + newline # Address++ # Address++ # Address++ # tempadd2 }");
     string action3 = string("{AUX[11].code = AUX[11].code + AUX[1].code + newline # saveadd2 # pop(11)}");



       prods_with_actions.push_back(prods_RHS[0]);
       prods_with_actions.push_back(prods_RHS[1]);
       prods_with_actions.push_back(prods_RHS[2]);
       prods_with_actions.push_back(prods_RHS[3]);
       prods_with_actions.push_back(action1);
       prods_with_actions.push_back(prods_RHS[4]);
       prods_with_actions.push_back(prods_RHS[5]);
       prods_with_actions.push_back(prods_RHS[6]);
       prods_with_actions.push_back(action2);
       prods_with_actions.push_back(prods_RHS[7]);
       prods_with_actions.push_back(prods_RHS[8]);
       prods_with_actions.push_back(prods_RHS[9]);
       prods_with_actions.push_back(prods_RHS[10]);
       prods_with_actions.push_back(action3);
    }
    else if(nonterm_name == "WHILE"){


     string action = string("{tempadd}");
     string action1 = string("{AUX[4].code = AUX[1].code + newline + Address % 0  + if_ + AUX[1].op + label + newline # Address++ }");
     string action2 = string("{AUX[7].code = AUX[7].code + AUX[1].code + newline + Address % 0  + goto + label + newline # Address++ # Address++ # Address++ # saveadd1 # pop(7)}");


       prods_with_actions.push_back(prods_RHS[0]);
       prods_with_actions.push_back(action);
       prods_with_actions.push_back(prods_RHS[1]);
       prods_with_actions.push_back(prods_RHS[2]);
       prods_with_actions.push_back(prods_RHS[3]);
       prods_with_actions.push_back(action1);
       prods_with_actions.push_back(prods_RHS[4]);
       prods_with_actions.push_back(prods_RHS[5]);
       prods_with_actions.push_back(prods_RHS[6]);
       prods_with_actions.push_back(action2);

    }
    return prods_with_actions;
}



//singleton class
class syntax_analyzer
{
//the data structures needed in this class.
private:
    //map gives the nonterminal given its name.
    map<string,non_terminal*> non_terminals;
    //vector to be used in checking the non terminals in the algorithms.
    vector<string> non_terminals_ordered;
    //The parser table implementation.
    //The sync entry is to be a single entry vector with the word synch.
    map<string,map<string,vector<string> > >parsing_table;

//The functions implementing the operations.
public:
    //constructors
    syntax_analyzer()
    {
    }
    syntax_analyzer(map<string,non_terminal*> non_terminals)
    {
        this->non_terminals = non_terminals;
    }
    //setters, getters and utility functions:
    void set_non_terminals(map<string,non_terminal*> non_terminals)
    {
        this->non_terminals = non_terminals;
    }
    void set_non_terminals_ordered(vector<string> non_terminals_ordered)
    {
        this->non_terminals_ordered = non_terminals_ordered;
    }
    map<string,non_terminal*> get_non_terminals()
    {
        return this->non_terminals;
    }
    vector<string> get_non_terminals_ordered()
    {
        return this->non_terminals_ordered;
    }
    //Just work with this when parsing the file
    void add_non_terminal(non_terminal* non_t)
    {
        non_terminals[non_t->name] = non_t;
        non_terminals_ordered.push_back(non_t->name);
    }
    non_terminal* get_non_terminal(string name)
    {
        return non_terminals[name];
    }
    bool is_non_terminal(string name)
    {
        return (non_terminals.find(name) != non_terminals.end());
    }
    //gets the parsing table entry vector of strings
    vector<string> get_table_entry(string non_terminal,string terminal)
    {
        return parsing_table[non_terminal][terminal];
    }
    //gets if the entry is empty
    bool is_empty_entry(string non_terminal,string terminal)
    {
        if(parsing_table.find(non_terminal) == parsing_table.end())
            return true;
        return (parsing_table[non_terminal].find(terminal) == parsing_table[non_terminal].end());
    }
    //gets if entry is a synchronizing token
    bool is_synch_entry(string non_terminal,string terminal)
    {
        return parsing_table[non_terminal][terminal][0] == SYNCH;
    }
    //gets the parsing table to be able to print it
    map<string,map<string,vector<string> > > get_parsing_table()
    {
        return parsing_table;
    }
    //printing functions for testing///////////////////////////////////////////////////////////////////////////////
    void print_first()
    {
        outfile_syntax<<"First:"<<endl;
        for(int i = 0; i < non_terminals_ordered.size(); i++)
        {
            non_terminal* curr = get_non_terminal(non_terminals_ordered[i]);
            outfile_syntax<<"first of "<<curr->name<<" : ";
            print_vector(curr->first);
            outfile_syntax<<endl;
            vector<production*> curr_prod = curr->productions;
            for(int j = 0; j < curr_prod.size(); j++)
            {
                outfile_syntax<<"first of ";
                print_vector(curr_prod[j]->RHS);
                outfile_syntax<<" : ";
                print_vector(curr_prod[j]->first);
                outfile_syntax<<endl;
            }
        }
    }
    void print_follow()
    {
        outfile_syntax<<"Follow:"<<endl;
        for(int i = 0; i < non_terminals_ordered.size(); i++)
        {
            non_terminal* curr = get_non_terminal(non_terminals_ordered[i]);
            outfile_syntax<<"follow of "<<curr->name<<" : ";
            print_vector(curr->follow);
            outfile_syntax<<endl;
        }
    }
    void print_parsing_table()
    {
        outfile_syntax<<"Parsing table:"<<endl;
        map<string, map<string,vector<string> > >::iterator outter_it =  parsing_table.begin();
        while (outter_it != parsing_table.end())
        {
            outfile_syntax << "non-terminal: " << outter_it->first << endl;
            map<string, vector<string> >::iterator inner_it = outter_it->second.begin();
            while (inner_it != outter_it->second.end())
            {
                outfile_syntax << " on " << inner_it->first << " gives the production: ";
                print_vector(inner_it->second);
                outfile_syntax<<endl;
                inner_it++;
            }
            outter_it++;
        }
    }
    void print_productions()
    {
        outfile_syntax<<"Productions:"<<endl;
        for(int i = 0; i < non_terminals_ordered.size(); i++)
        {
            non_terminal* current = get_non_terminal(non_terminals_ordered[i]);
            for(int j = 0 ; j < current->productions.size(); j++)
            {
                outfile_syntax<<current->name<<" -> ";
                print_vector(current->productions[j]->RHS);
                outfile_syntax<<endl;
            }
        }

    }

    //fills in the parsing table using the panic mode error recovery
    bool set_parsing_table()
    {
        set_first();
        set_follow();
        for (int i = 0; i < non_terminals_ordered.size(); i++)
            if(!fill_parsing_table(non_terminals_ordered[i]))
            {
                cout<<"Error: not LL(1) grammar."<<endl;
                outfile_syntax<<"Error: not LL(1) grammar."<<endl;
                return false;
            }
        //ManualParseTable* temp_tab = new ManualParseTable();
        //parsing_table = temp_tab->get_parser_table();
        return true;
    }
//Private functions are to be placed here
private:


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //algorithm functions to be done after setting the data correctly
    //sets the first for the non terminals and their productions
    void set_first()
    {
        for(int i = 0; i < non_terminals_ordered.size(); i++)
            get_first_single(non_terminals_ordered[i]);
    }
    //sets the follow for the non terminals
    void set_follow()
    {
        for (int i = 0; i < non_terminals_ordered.size(); i++)
        {
            vector<pair<string,string> > vec;
            get_follow(non_terminals_ordered[i],vec);
        }
    }

    //Here are the functions that get the first sets for all the non terminals.
    //This is the most basic function that given a string symbol it gets its first set
    //this function works in a recursion like manner.
    vector<string> get_first_single(string symbol)
    {
        vector<string> result;
        //if this is a terminal then just return it.
        if(!is_non_terminal(symbol))
        {
            result.push_back(symbol);
            return result;
        }
        non_terminal* non_t = get_non_terminal(symbol);

        //if first set is already calculated just return it
        if(non_t->first.size())
        {
            return non_t->first;
        }

        //now performing the first calculation algorithm
        //for each production we add the first for the RHS of the production to this non terminal

        for(int i = 0; i < non_t->productions.size(); i++)
        {

            //getting the current production and its RHS
            production* current_production = non_t->productions[i];
            vector<string> RHS = current_production->RHS;
            //we now add to the result the result of obtaining the first set of this right hand side.
            result = merge_vectors(result,(current_production->first = get_first_RHS(RHS)));

        }
        //add the first set as now it is completely calculated and return it.
        non_t->first = result;

        return result;
    }
    //getting the first of a given string vector considered RHS of a production
    vector<string> get_first_RHS(vector<string> RHS)
    {
        vector<string> result;
        //in case of an epsilon production just return the epsilon
        if(RHS.size() == 1 && RHS[0] == EMPTYSTR)
        {
            result.push_back(EMPTYSTR);
            return result;
        }
        //now getting symbol at a time
        int i;
        for(i = 0; i < RHS.size(); i++)
        {
            string current_symbol = RHS[i];
            //add the first of this symbol to the result and exclude empty string
            result = add_vector_without_empty(result,get_first_single(current_symbol));
            //in case the first set does not have empty string we stop other wise we continue to the next string
            if(!has_empty_string(get_first_single(current_symbol)))
                break;
        }
        //now if all the symbols have empty string we add the empty string to the first set
        //ie: if no break occurred
        if(i == RHS.size())
        {
            result.push_back(EMPTYSTR);
        }
        return result;
    }
    /*function to calculate the follow of all non-terminals,
    tracker vector is used to keep track of input to terminate in case of infinite recursion*/
    vector<string> get_follow(string symbol,vector<pair<string,string>> tracker)
    {
        vector<string> result;
        /** Rule-01: For the start symbol S, Then place $ in Follow(S). **/
        if (symbol == non_terminals_ordered[0])
            result.push_back(END_MARKER);
        non_terminal* non_t = get_non_terminal(symbol);

        /*if follow set is already calculated just return it*/
        if(non_t->follow.size())
        {
            return non_t->follow;
        }

        /*We here access all the production rules to set the follow of the given non-terminal*/
        for (int i = 0; i < non_terminals_ordered.size(); i++)
        {
            non_terminal* temp = get_non_terminal(non_terminals_ordered[i]);
            vector<production*> prods = temp->productions;
            for (int j = 0; j < prods.size(); j++)
            {
                vector<string> rhs = prods[j]->RHS;
                /*exclude the productions that doesn't contain the given non-terminal in the RHS*/
                if (!is_found(symbol,rhs))
                    continue;
                /*get all indices of the non-terminal in the RHS, handling multiple appearances*/
                vector<int> indices = get_indices(symbol,rhs);
                /*in case the non-terminal to be calculated is found at the end of the production and it's same as the LHS
                avoiding self loops that won't lead to a final answer*/
                if (non_terminals_ordered[i] == symbol && indices.size() == 1 && indices[0] == rhs.size()-1)
                    continue;
                /** Rule-02: For any production rule A -> αB, Then Follow(B) = Follow(A). **/
                if (indices.size() == 1 && indices[0] == rhs.size()-1)
                {
                    pair<string,string> new_pair = make_pair(symbol,non_terminals_ordered[i]);
                    tracker.push_back(new_pair);
                    /*if the follow to be added isn't calculated and has been accessed before we terminate,
                    the infinite recursion is detected here*/
                    if (found_pair(new_pair,tracker) && get_non_terminal(non_terminals_ordered[i])->follow.size() == 0)
                        break;
                    result = add_vector_without_empty(result,get_follow(non_terminals_ordered[i],tracker));
                }
                else
                {
                    /*the next vector contains all the next symbols in the production, handling multiple appearances*/
                    vector<string> first_of_following;
                    for (int l = 0; l < indices.size(); l++)
                        first_of_following = merge_vectors(first_of_following,get_first_RHS(get_following_symbols(indices[l],rhs)));
                    /** Rule-03: For any production rule A -> αBβ,
                                a) If Є not in First(β), Then Follow(B) = First(β) **/
                    result = add_vector_without_empty(result, first_of_following);
                    /** Rule-03: For any production rule A -> αBβ,
                                b) If Є in First(β), Then Follow(B) = { First(β) – ∈ } ∪ Follow(A) **/
                    if (is_found(EMPTYSTR,first_of_following))
                    {
                        pair<string,string> new_pair = make_pair(symbol,non_terminals_ordered[i]);
                        tracker.push_back(new_pair);
                        /*if the follow to be added isn't calculated and has been accessed before we terminate,
                        the infinite recursion is detected here*/
                        if (found_pair(new_pair,tracker) && get_non_terminal(non_terminals_ordered[i])->follow.size() == 0)
                            break;
                        result = add_vector_without_empty(result, get_follow(non_terminals_ordered[i],tracker));
                    }
                }
            }
        }
        /*finally setting the follow of the non-terminal*/
        non_t->follow = result;
        return result;
    }
    //function to fill the parsing table
    bool fill_parsing_table(string symbol)
    {
        //The synchronization token
        vector<string> sync;
        sync.push_back(SYNCH);
        //The required nonterminal to work on
        non_terminal* non_t = get_non_terminal(symbol);
        //The row of this non terminal
        map<string,vector<string> > non_t_row;
        //For each production
        for (int i = 0; i < non_t->productions.size(); i++)
        {
            vector<string> first_of_prod = non_t->productions[i]->first;
            /**for each terminal a in the FIRST(production) -> add the production in the table**/
            for (int j = 0; j < first_of_prod.size(); j++)
            {
                //in case of double entry report the error and return false.
                if(non_t_row.find(first_of_prod[j])!= non_t_row.end())
                {
                    outfile_syntax<<"double entry at non terminal: "<<non_t->name<<" input: "<<first_of_prod[j]<<" productions: ";
                    print_vector(non_t_row[first_of_prod[j]]);
                    outfile_syntax<<" and ";
                    print_vector(non_t->productions[i]->RHS);
                    outfile_syntax<<endl;
                    return false;
                }
                if (first_of_prod[j] != EMPTYSTR){

                    non_t_row[first_of_prod[j]] = get_semantic_action(non_t->name,non_t->productions[i]->RHS);
                }
            }
            /**If epsilon in FIRST(production) Then for each terminal a in FOLLOW(A) -> add the production in the table**/
            if (is_found(EMPTYSTR,first_of_prod))
            {
                for (int j = 0; j < non_t->follow.size(); j++)
                {
                    //in case of double entry report the error and return false.
                    if(non_t_row.find(non_t->follow[j])!= non_t_row.end())
                    {
                        outfile_syntax<<"double entry at non terminal: "<<non_t->name<<" input: "<<non_t->follow[j]<<" productions: ";
                        print_vector(non_t_row[non_t->follow[j]]);
                        outfile_syntax<<" and ";
                        print_vector(non_t->productions[i]->RHS);
                        outfile_syntax<<endl;
                        return false;
                    }

                    non_t_row[non_t->follow[j]] = get_semantic_action(non_t->name,non_t->productions[i]->RHS);

                }
            }
        }
        /*this is loop to add the synch entries to the table at empty follow entries*/
        for (int i = 0; i < non_t->productions.size(); i++)
        {
                for (int j = 0; j < non_t->follow.size(); j++)
                {
                    if(non_t_row.find(non_t->follow[j]) == non_t_row.end())
                    non_t_row[non_t->follow[j]] = sync;
                }
        }
        parsing_table[symbol] = non_t_row;
        return true;
    }
    //utility functions
    //function to check if a pair in the vector
    bool found_pair(pair<string,string> p, vector<pair<string,string>> v)
    {
        for (int i = 0; i < v.size() - 1; i++)
            if (v[i] == p)
                return true;
        return false;
    }
    //function to get all strings after a certain index
    vector<string> get_following_symbols(int index, vector<string> rhs)
    {
        vector<string> following_sym;
        for (int i = 0; i < rhs.size(); i++)
            if (i > index)
                following_sym.push_back(rhs[i]);
        return following_sym;
    }
    //gets the index of a string in a vector
    vector<int> get_indices(string str,vector<string> vec)
    {
        vector<int> v;
        for (int i = 0; i < vec.size(); i++)
            if (str == vec[i])
                v.push_back(i);
        return v;
    }
    //checks if a string is in a vector
    bool is_found(string str,vector<string> vec)
    {
        for (int i = 0; i < vec.size(); i++)
            if (str == vec[i])
                return true;
        return false;
    }
    //function to add v2 to v1 excluding the epsilon
    vector<string> add_vector_without_empty(vector<string> v1,vector<string> v2)
    {
        for (int i = 0; i < v2.size(); i++)
            if (!is_found(v2[i],v1) && v2[i] != EMPTYSTR)
                v1.push_back(v2[i]);
        return v1;
    }
    //function to merge two vectors excluding the repititions
    vector<string> merge_vectors(vector<string> vector1,vector<string> vector2)
    {
        for(int i = 0; i < vector2.size(); i++)
        {
            string current = vector2[i];
            if(!is_found(current,vector1))
                vector1.push_back(current);
        }
        return vector1;
    }
    //function checks if empty string is in a vector of strings
    bool has_empty_string(vector<string> input)
    {
        return is_found(EMPTYSTR,input);
    }



    //utility functions for testing //////////////////////////
    //function that prints a vector
    void print_vector(vector<string> v)
    {
        for(int i = 0; i < v.size(); i++)
        {
            outfile_syntax<<v[i]<<" ";
        }
    }

} syntax_analyzer;

// add all non terminals to syntax analyzer class
void add_all_nonterminals()
{
    // get all non terminals after removing left recursion and applying left factoring
    vector<nonTerminal_parser> all_nonterms_parser = final_grammar();
    // loop on all non terminals
    for(int i=0;i<all_nonterms_parser.size();i++){
        struct non_terminal *non_term = new non_terminal();
        // get non terminal name
        non_term->name = all_nonterms_parser[i].name;
        // get non terminal productions
        vector<vector<string>> prods = all_nonterms_parser[i].productions;
        vector<production*> added_prods;
        // set the production struct
        for(int j=0;j<prods.size();j++){
            struct production* prod = new production();
            prod->RHS = prods[j];
            added_prods.push_back(prod);
        }
        non_term->productions = added_prods;
        // add non terminal to syntax analyzer class
        syntax_analyzer.add_non_terminal(non_term);
    }
}

void print_vector(vector<string> vec_str)
{
    for(int i=0;i<vec_str.size();i++){
        if(vec_str[i]!=EMPTYSTR)
        outfile_syntax << vec_str[i] << "  " ;
    }
    outfile_syntax << endl;
}


/**-------------------- modifications for phase 3 -------------------------------------**/




/** strings for regex **/
string str_semantic_rules = "\\{(.*)\\}";
string str_stackatts = "\\s*(AUX|S)\\s*\\[([0-9]+)\\].([a-zA-Z0-9]+)\\s*";
string str_assign = str_stackatts +"=(.*)";
string str_if_structure = "\\s*if\\s*\\((.*)\\)(.*)";
string str_if_condition = str_stackatts + "=(.*)";
string str_ifnot_condition = str_stackatts + "!=(.*)";
string str_address_print = "\\s*(Address)\\s*\\$\\s*([0-9]+)\\s*:\\s*(\\w*-?\\w*?)(\\s)?(" + str_stackatts + ")?\\s*\\;\\s*";
string str_address_inc_print_RHS = "\\s*(Address)\\s*\\%(\\%)?\\s*([0-9]+)\\s*";
string str_print_bytecode = "\\s*print\\s*\\(" + str_stackatts + "\\)\\s*\\;\\s*";
string str_instructions = "\\s*([0-9]+)\\s*:(\\w*)\\s*(\\L([0-9]+))\\s*";
string str_pop = "\\s*pop\\s*\\(([0-9]+)\\s*\\)";
string str_int = "[0-9]+";
string str_float = "[0-9]+.[0-9]+";

stack<int> tempaddresses;
vector<int> tempaddresses_else;
vector<int> jump_addresses;
int label_num = 0;
int address = 0;
char newline = '\n';


/* it returns the type of specific number either it is integer or float */
string get_type(string str)
{
    regex reg_int(str_int);
    regex reg_float(str_float);
    if(regex_match(str,reg_int)){
        return "int";
    }
    else if(regex_match(str,reg_float)){
        return "float";
    }
    return "";
}

/* removing first and last spaces of string */
string remove_extra_spaces(string str)
{
    string temp1 = regex_replace(str,regex("^\\s+"),"");
    string temp2 = regex_replace(temp1,regex("\\s+$"),"");
    return temp2;
}

/* split string by a specific delimiter and returns vector of string of these splitted strings */
vector<string> split_strs(string RHS,char delimiter)
{
    stringstream ss(RHS);
    string one_element;
    vector<string> all_elements;
    while(getline(ss,one_element,delimiter)){
        one_element = remove_extra_spaces(one_element);
        all_elements.push_back(one_element);
    }
    return all_elements;
}

stack_node* getElement(int stackType, int position)
{
    stack_node *node ;
    stack<stack_node *> tempStack;
    if(stackType == 0)
    {
        //error
        if(position >= checker_stack.size())
        {
            return new stack_node("null");
        }
        while(position > 0)
        {
            position--;
            tempStack.push(checker_stack.top());
            checker_stack.pop();
        }
        node = checker_stack.top();
        while(!tempStack.empty())
        {
            checker_stack.push(tempStack.top());
            tempStack.pop();
        }
    }
    else
    {
         //error
        if(position >= stack_aux.size())
        {
            return new stack_node("null");
        }
        while(position > 0)
        {
            position--;
            tempStack.push(stack_aux.top());
            stack_aux.pop();
        }
        node = stack_aux.top();
        while(!tempStack.empty())
        {
            stack_aux.push(tempStack.top());
            tempStack.pop();
        }
    }

    return node;
}


/* get a value of specific attribute  */
string get_one_att(string stack_name , int att_number , string att_name)
{
            // auxiliary stack
        int stack_num = 1;
        if(stack_name == "AUX"){
            /** set attributes of auxiliary **/
            stack_num = 1;
        }
        // main stack
        else{
            /** set attributes of main stack **/
            stack_num = 0;
        }
        if(getElement(stack_num,att_number)->get_node_name() == "null"){
            return "";
        }

        stack_node* temp_RHS = getElement(stack_num,att_number);
        return (temp_RHS->getAttr(att_name));

}

bool evaluate_conditions(string conditions)
{
   bool check_equal = false;
   vector<string> all_conditions = split_strs(conditions,'\\&');
   for(int i=0;i<all_conditions.size();i++){
        string cond = all_conditions[i];

        regex reg_condition(str_if_condition);
        regex reg_condition_not(str_ifnot_condition);
        smatch matches;


        if(regex_match(cond,matches,reg_condition) || regex_match(cond,matches,reg_condition_not)){
            string stack_name = matches[1];
            string att_num = matches[2];
            string att_name = matches[3];

            /** get attributes from map **/
            string cond_val_LHS = get_one_att(stack_name,stoi(att_num),att_name);

            /** if equal conditions ---> check_equal = true;**/
            /** else return false **/
            string RHS_cond = remove_extra_spaces(matches[4]);

            if(regex_match(cond,reg_condition)){

               if((cond_val_LHS == RHS_cond && RHS_cond != "none") || (RHS_cond == "none" && cond_val_LHS == "")){
                  check_equal = true;
               }
               else{
                  return false;
               }
            }
            else if(regex_match(cond,reg_condition_not)){

               if((cond_val_LHS != RHS_cond && RHS_cond != "none") || (RHS_cond == "none" && cond_val_LHS != "")){
                  check_equal = true;
               }
               else{
                  return false;
               }
            }

        }
   }
   return check_equal;
}


/* get values of all RHS elements */
string get_all_RHSelements(vector<string> elements)
{
    string total_RHS = "";
    for(int i=0;i<elements.size();i++){
        string temp = elements[i];
        regex reg_stackatts(str_stackatts);
        smatch matches;

        regex reg_if_structure(str_if_structure);
        smatch matches_if;

        regex reg_address_rhs_print(str_address_inc_print_RHS);
        smatch matches_inc_rhs_print;


        if(regex_match(temp,matches,reg_stackatts)){
            string stack_name = matches[1];
            string att_num = matches[2];
            string att_name = matches[3];

            /*** get attributes from map ***/

            total_RHS+= get_one_att(stack_name,stoi(att_num),att_name);

        }
        else if(temp == "newline"){
            total_RHS+= newline;
        }
        /** if statements in RHS**/
        else if(regex_match(temp,matches_if,reg_if_structure))
        {
                string conditions = remove_extra_spaces(matches_if[1]);
                if(evaluate_conditions(conditions)){
                    vector<string> adds = split_strs(matches_if[2],',');
                    for(int j=0;j<adds.size();j++){
                        string add = adds[j];
                        regex reg_address(str_address_print);
                        smatch matches_address;
                        if(regex_match(add,matches_address,reg_address)){
                           address = address + stoi(matches_address[2]);
                           string new_address = to_string(address);
                           string word = matches_address[3];
                           string att_value = "";
                           if(matches_address[5]!=""){
                             string is_space = matches_address[4];
                             att_value += is_space;
                             string stack_name = matches_address[6];
                             string att_num = matches_address[7];
                             string att_name = matches_address[8];
                             att_value += get_one_att(stack_name,stoi(att_num),att_name);
                           }
                           total_RHS += (new_address + ":" + word + att_value );
                        }
                        else if(add == "newline"){
                           total_RHS+= newline;
                        }
                }

            }
        }

        else if(regex_match(temp,matches_inc_rhs_print,reg_address_rhs_print)){
            address = address + stoi(matches_inc_rhs_print[3]);
            if(matches_inc_rhs_print[2]==""){
                total_RHS += to_string(address) + ":";
            }
        }

        /** temp = boolean or int or float or none or label ..... **/
        else {
            if(temp == "none"){
                total_RHS+= "";
            }
            else if(temp == "label"){
                string label = "   L" + to_string(label_num);
                total_RHS+= label;
                label_num++;
            }
            else{
                total_RHS += temp;
            }
        }
    }

    return total_RHS;
}

/** takes string has assign operation  eg(AUX[2].val = AUX[1].val + newline + if() + AUX[0].val ) **/
void set_attributes(string str)
{
    regex main_reg(str_assign);
    smatch main_matches;

    if(regex_match(str,main_matches,main_reg)){
        //string LHS = main_matches[1];
        string RHS = main_matches[4];
        vector<string> all_elements;

        /*** separate elements of RHS by (+) char ***/
        all_elements = split_strs(RHS,'+');

        /*** get each element of RHS ***/
        string RHS_elements = get_all_RHSelements(all_elements);
        string temp_RHS = RHS_elements;
        temp_RHS.erase(std::remove(temp_RHS.begin(), temp_RHS.end(), '\n'), temp_RHS.end());
         if(remove_extra_spaces(temp_RHS)==""){
            RHS_elements = "";
           }

            if(RHS_elements != "" ){

            string stack_name = main_matches[1];
            string att_number = main_matches[2];
            string att_name = main_matches[3];
            // auxiliary stack
            int stack_num = 1;
            if(stack_name == "AUX"){
                /** set attributes of auxiliary **/
                stack_num = 1;
            }
            // main stack
            else{
                /** set attributes of main stack **/
                stack_num = 0;
            }
            if(getElement(stack_num,stoi(att_number))->get_node_name() == "null"){
                return;
            }
            else{
              stack_node* temp_LHS = getElement(stack_num,stoi(att_number));
              temp_LHS->setAttr(att_name,RHS_elements);
            }



        }
    }

}

/** takes any form of rule eg (address++ or if () else ()  or (AUX[2].val = AUX[1].val + newline + if() + AUX[0].val )) **/
void final_parser(string rule)
{
    regex main_reg(str_assign);
    smatch main_matches;

    regex reg_if(str_if_structure);
    smatch if_matches;

    regex reg_print(str_print_bytecode);
    smatch print_matches;

    regex reg_pop(str_pop);
    smatch pop_matches;

    if(regex_match(rule,main_matches,main_reg)){
         set_attributes(rule);
    }
    else if(regex_match(rule,if_matches,reg_if)){
        string conditions = remove_extra_spaces(if_matches[1]);
        bool check = evaluate_conditions(conditions);
        if(check){
            vector<string> assign_atts = split_strs(if_matches[2],',');
            for(int i=0;i<assign_atts.size();i++){
                string temp = assign_atts[i];
                set_attributes(temp);
            }
        }
    }
    else if(regex_match(rule,print_matches,reg_print)){
        string stack_name = print_matches[1];
        string att_num = print_matches[2];
        string att_name = print_matches[3];
        /** printing attributes values **/
        ofstream outfile_byte;
        string byte_out_filename = "output_bytecode.txt";
        outfile_byte.open(byte_out_filename);
        string final_output = get_one_att(stack_name,stoi(att_num),att_name);
        vector<string> instructions = split_strs(final_output,'\n');

        int count_inst = 0;
        int if_stat = 0;
        int goto_stat = 0;
        bool if_stat_check = 0;
        bool goto_stat_check = 0;

        for(int i=0;i<instructions.size();i++){
            if(instructions[i]!=""){



                regex reg_ifinst("\\s*([0-9]+)\\s*:if\\_(\\w*)\\s*(\\L([0-9]+))\\s*");
                regex reg_gotoinst("\\s*([0-9]+)\\s*:goto\\s*(\\L([0-9]+))\\s*");
                if(regex_match(instructions[i],reg_ifinst) || regex_match(instructions[i],reg_gotoinst)){
                    if(regex_match(instructions[i],reg_ifinst)){
                        if_stat++;
                        if_stat_check = true;
                        goto_stat_check = false;

                    }
                    else if(regex_match(instructions[i],reg_gotoinst)){

                        goto_stat= goto_stat + 2;
                        goto_stat_check = true;
                        if(!if_stat_check){
                           int temp = jump_addresses[goto_stat-2];
                           jump_addresses.erase(jump_addresses.begin() + (goto_stat -2) );
                           jump_addresses.insert(jump_addresses.begin() + count_inst,temp);
                        }
                        if_stat_check = false;
                        if_stat--;
                        if(if_stat == 0){
                            count_inst= count_inst+2;
                        }

                    }
                }


           }

        }



        for(int i=0;i<instructions.size();i++){
            if(instructions[i]!=""){
                regex reg_inst(str_instructions);
                smatch inst_matches;

                if(regex_match(instructions[i],inst_matches,reg_inst)){
                   instructions[i] = string(inst_matches[1]) + ":" +string(inst_matches[2]) + "  #" + to_string(jump_addresses[0]);
                   jump_addresses.erase(jump_addresses.begin());
                }
                 outfile_byte << instructions[i] <<endl;
            }

        }


        outfile_byte.close();
    }




    else if(regex_match(rule,pop_matches,reg_pop)){
        string pop_times = pop_matches[1];
        for(int i=0;i<stoi(pop_times);i++){
            if(stack_aux.size()!=0){
              stack_aux.pop();
            }
        }
    }

    /// if stats

    else if(remove_extra_spaces(rule) == "tempadd2"){
       tempaddresses_else.push_back(address);
    }

    else if(remove_extra_spaces(rule) == "saveadd2"){
          if(tempaddresses_else.size()!=0){
            jump_addresses.push_back(tempaddresses_else[0]);
            tempaddresses_else.erase(tempaddresses_else.begin());
          }
          jump_addresses.push_back(address);
    }


    /// while stats

    else if(remove_extra_spaces(rule) == "tempadd"){
        tempaddresses.push(address);
    }

    else if(remove_extra_spaces(rule) == "saveadd"){
          jump_addresses.push_back(address);
    }

    else if(remove_extra_spaces(rule) == "saveadd1"){
        jump_addresses.push_back(address);
        jump_addresses.push_back(tempaddresses.top());
        tempaddresses.pop();
    }

    else{

        address++;
    }
}

/* check if current node of stack is action or not */
bool is_semantic(string semantic)
{
    regex reg(str_semantic_rules);
    if(regex_match(semantic,reg)){
        return true;
    }
    return false;
}

void parse_semantic(string semantic_rules)
{
    regex reg(str_semantic_rules);
    smatch matches;
    if(regex_match(semantic_rules,matches,reg)){
       vector<string> all_rules = split_strs(matches[1],'#');
       for(int i=0;i<all_rules.size();i++){
           string rule = all_rules[i];
           final_parser(rule);
       }
    }
}

void run_stack_nodes_final_test()
{
    /* assumption
       1) remove all error tokens from lexical analyzer
    */
   bool has_error = false;
   string id_type = "";
   vector<string> input_tokens = get_all_tokens();
   vector<string> tokens_values = get_all_tokens_values();
   for(int i=0;i<input_tokens.size();i++){
    // remove error from tokens
    if(input_tokens[i]=="Error"){
        input_tokens.erase(input_tokens.begin()+i);
        i--;
    }
   }
   // check if there are no tokens
   if(input_tokens.size()==0){
      cout << "No tokens found" << endl;
      outfile_syntax << "No tokens found" << endl;
      return;
   }

   input_tokens.push_back(END_MARKER);
   tokens_values.push_back(END_MARKER);
   stack_node* endnode = new stack_node(END_MARKER);
   checker_stack.push(endnode);
   string start_symbol = syntax_analyzer.get_non_terminals_ordered().at(0);
   stack_node* startnode = new stack_node(start_symbol);


   checker_stack.push(startnode);
   vector<string> remaining_elements;
   remaining_elements.push_back(start_symbol);
   print_vector(remaining_elements);
   int counter = 0;
   while(checker_stack.top()->get_node_name()!=END_MARKER){
     string st_top = checker_stack.top()->get_node_name();

    // check if top is terminal
     if( !is_semantic(st_top) && !syntax_analyzer.is_non_terminal(st_top) ){
        if(input_tokens[0] == st_top){
            if(st_top == END_MARKER){
                // string matches the grammar
                outfile_syntax << "the end"<<endl;
                break;
            }
            else{
                // matched terminals
                map<string , pair<int,string>> temp_map = get_symbol_table();
                if(st_top == "int" || st_top == "float"){
                    id_type = st_top;
                }
                else if(st_top == "num"){
                    checker_stack.top()->setAttr("value",tokens_values[0]);
                    checker_stack.top()->setAttr("type",get_type(tokens_values[0]));
                }
                else if(st_top == "id"){
                    checker_stack.top()->setAttr("value",to_string(temp_map[tokens_values[0]].first));
                    if(id_type == "int" || id_type == "float"){
                        temp_map[tokens_values[0]].second = id_type;
                        set_symbol_table(temp_map);
                        id_type = "";
                    }
                    else{
                        checker_stack.top()->setAttr("type",temp_map[tokens_values[0]].second);
                    }

                }
                else if(st_top == "addop"){
                    if(tokens_values[0] == "+"){
                        checker_stack.top()->setAttr("value","add");
                    }else if(tokens_values[0] == "-"){
                        checker_stack.top()->setAttr("value","sub");
                    }
                }
                else if(st_top == "mulop"){
                    if(tokens_values[0] == "*"){
                        checker_stack.top()->setAttr("value","mul");
                    }else if(tokens_values[0] == "/"){
                        checker_stack.top()->setAttr("value","div");
                    }
                }
                else if(st_top == "relop"){
                    if(tokens_values[0] == "=="){
                        checker_stack.top()->setAttr("value","ne");
                    }
                    else if(tokens_values[0] == ">"){
                        checker_stack.top()->setAttr("value","le");
                    }
                    else if(tokens_values[0] == "<"){
                        checker_stack.top()->setAttr("value","ge");
                    }
                    else if(tokens_values[0] == "<="){
                        checker_stack.top()->setAttr("value","gt");
                    }
                    else if(tokens_values[0] == ">="){
                        checker_stack.top()->setAttr("value","lt");
                    }
                    else if(tokens_values[0] == "!="){
                        checker_stack.top()->setAttr("value","eq");
                    }
                }

                /// push terminals to aux stack ///
                stack_aux.push(checker_stack.top());
                checker_stack.pop();


                input_tokens.erase(input_tokens.begin());
                tokens_values.erase(tokens_values.begin());

                outfile_syntax << "terminals match" << endl;
                print_vector(remaining_elements);
                counter++;
            }
        }
        else{

            outfile_syntax << "terminals mismatch(pop from stack)"<< endl;
            has_error = true;
            checker_stack.pop();
            remaining_elements.erase(remaining_elements.begin()+counter);
            print_vector(remaining_elements);
        }
     }

     else if(is_semantic(st_top)){
        /// semantic actions execution ///
        checker_stack.pop();
        parse_semantic(st_top);

        counter++;
     }
     // top is non terminal
     else{
            // check for empty entry
        if(syntax_analyzer.is_empty_entry(st_top,input_tokens[0])){
            // error (remove from input)
            outfile_syntax << "error (remove from input)" << endl;
            has_error = true;
            input_tokens.erase(input_tokens.begin());
            tokens_values.erase(tokens_values.begin());

        }
           // check for synch
         else if(syntax_analyzer.is_synch_entry(st_top,input_tokens[0])){
             // error (synch entry pop from stack)
            outfile_syntax << "error (synch entry pop from stack)" << endl;
            checker_stack.pop();
            remaining_elements.erase(remaining_elements.begin()+counter);

         }

         else{
            vector<string> entry_prod = syntax_analyzer.get_table_entry(st_top,input_tokens[0]);
            stack_aux.push(checker_stack.top());

            checker_stack.pop();
            remaining_elements.erase(remaining_elements.begin()+counter);
            int j = 0;
            for(int i=0;i<entry_prod.size();i++){

                if(entry_prod[i]!=EMPTYSTR)
                {
                   remaining_elements.insert(remaining_elements.begin()+counter+j ,entry_prod[i]);
                   j++;
                }

            }
            for(int i=entry_prod.size()-1;i>=0;i--){
                if(entry_prod[i]!= EMPTYSTR){
                    stack_node* tempnode = new stack_node(entry_prod[i]);

                    /// we need to set inherited and synthesized attributes///

                    checker_stack.push(tempnode);

                }// output with production entry_prod
            }
            outfile_syntax << st_top << "----->";
            for(int i=0;i<entry_prod.size();i++){
                //checker_stack.push(entry_prod[i]);
               // if(! is_semantic(entry_prod[i]))
                outfile_syntax <<  entry_prod[i] << "  ";
                // output with production entry_prod
            }
            outfile_syntax << endl;

         }
         print_vector(remaining_elements);
     }


   }
   if(input_tokens.size()>1){
     // input is not finished yet
     outfile_syntax << "input mismatch the grammar" << endl;
   }
   else{
     // both the stack and input ends with $
     if(has_error){
        outfile_syntax << "input matches the grammar with errors" << endl;
     }
     else{
        outfile_syntax << "input matches the grammar" << endl;
     }
   }

}

void main_synt_analyzer()
{
    // set non terminals
    add_all_nonterminals();

    outfile_syntax.open(syntax_out_filename);
    // set the parsing table
    bool is_LL1 = syntax_analyzer.set_parsing_table();
    syntax_analyzer.print_first();
    outfile_syntax << endl;
    syntax_analyzer.print_follow();
    outfile_syntax << endl;
    // check is grammar is LL1
    if(is_LL1){
       syntax_analyzer.print_parsing_table();
       outfile_syntax << endl;
       //run_stack();
       //run_stack_nodes_test();
       run_stack_nodes_final_test();
    }

    outfile_syntax.close();
}


