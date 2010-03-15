#include <vector>
#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
#include <map>
#include "ff_dwarf.h"
#include "dwarf.h"
#include "wordid.h"
#include "tdict.h"
#include "filelib.h"

using namespace std;

void printTable(CountTable table, int count) {
  /*//cerr << "First level: " << endl;
  for (map<WordID,int*>::iterator it = table.first.begin(); it!=table.first.end(); it++) {
    //cerr << TD::Convert(it->first) << ": ";
    for (int i=0; i<count; i++) {
      //cerr << it->second[i] << " ";
    }
    //cerr << endl;
  }
  //cerr << "Second level: " << endl;
  for (map<WordID,int*>::iterator it = table.second.begin(); it!=table.second.end(); it++) {
    //cerr << TD::Convert(it->first) << ": ";
    for (int i=0; i<count; i++) {
      //cerr << it->second[i] << " ";
    }
    //cerr << endl;
  }
  //cerr << "Third level: " << endl;
  for (int i=0; i<count; i++) {
    //cerr << table.third[i] << " ";
  }
  //cerr << endl;*/
}


Dwarf::Dwarf(const std::string& param) { 
  flag_oris = false; flag_orit = false; flag_doms = false; flag_domt = false;
  SetStateSize(37*sizeof(int)); // bad hardcoding!!!   
  als = new Alignment();  
  //cerr << "here to clear" << endl;
  als->clearAls(Alignment::MAX_WORDS,Alignment::MAX_WORDS);
  istringstream iss(param);
  string w;
  while(iss >> w) {
    int equal = w.find_first_of("=");
    if (equal!=string::npos) {
      string model = w.substr(0,equal);
      string fn    = w.substr(equal+1);
      if (model == "oris") {
        //cerr << "preparing oris" << endl;
        flag_oris = readOrientation(&toris,fn,&sfw); 
        if (flag_oris) {
          oris_ = FD::Convert("OrientationSource");
          oris_bo1_ = FD::Convert("OrientationSource_BO1");
          oris_bo2_ = FD::Convert("OrientationSource_BO2");
        }
      } else if (model == "orit") {
        flag_orit = readOrientation(&torit,fn,&tfw); 
        if (flag_orit) {
          orit_ = FD::Convert("OrientationTarget");
          orit_bo1_ = FD::Convert("OrientationTarget_BO1");
          orit_bo2_ = FD::Convert("OrientationTarget_BO2");
        }
      } else if (model == "doms") {
        flag_doms = readDominance(&tdoms,fn,&sfw); 
        if (flag_doms) {
          doms_ = FD::Convert("DominanceSource");
          doms_bo1_ = FD::Convert("DominanceSource_BO1");
          doms_bo2_ = FD::Convert("DominanceSource_BO2");
        }
      } else if (model == "domt") {
        flag_domt = readDominance(&tdomt,fn,&tfw); 
        if (flag_domt) {
          domt_ = FD::Convert("DominanceTarget");
          domt_bo1_ = FD::Convert("DominanceTarget_BO1");
          domt_bo2_ = FD::Convert("DominanceTarget_BO2");
        }
      } else {
        //cerr << "DWARF doesn't understand this model: " << model << endl;
      }
    } else {
      //cerr << "DWARF doesn't need this param: " << param << endl; 
    }  
  }
  //sfw.insert(pair<WordID,int>(TD::Convert("B"),1));
  /*sfw.insert(pair<WordID,int>(TD::Convert("FW"),1));
  sfw.insert(pair<WordID,int>(TD::Convert("FW1"),1));
  sfw.insert(pair<WordID,int>(TD::Convert("FW2"),1));
  sfw.insert(pair<WordID,int>(TD::Convert("FW3"),1));
  sfw.insert(pair<WordID,int>(TD::Convert("FW4"),1));
  sfw.insert(pair<WordID,int>(TD::Convert("FW5"),1));
  sfw.insert(pair<WordID,int>(TD::Convert("FW6"),1));*/
  //sfw.insert(pair<WordID,int>(TD::Convert("X1C"),1));
  //tfw.insert(pair<WordID,int>(TD::Convert("x2"),1));
  /*tfw.insert(pair<WordID,int>(TD::Convert("FW"),1));
  tfw.insert(pair<WordID,int>(TD::Convert("FW1"),1));
  tfw.insert(pair<WordID,int>(TD::Convert("FW2"),1));
  tfw.insert(pair<WordID,int>(TD::Convert("FW3"),1));
  tfw.insert(pair<WordID,int>(TD::Convert("FW4"),1));
  tfw.insert(pair<WordID,int>(TD::Convert("FW5"),1));
  tfw.insert(pair<WordID,int>(TD::Convert("FW6"),1));*/
  //cerr << "finished initiating ff_dwarf" << endl;
}

void Dwarf::TraversalFeaturesImpl(const SentenceMetadata& smeta,
                                     const Hypergraph::Edge& edge,
                                     const std::vector<const void*>& ant_contexts,
                                     SparseVector<double>* features,
                                     SparseVector<double>* estimated_features,
                                     void* context) const {
  double cost, bonus, bo1, bo2, bo1_bonus, bo2_bonus;
  //cerr << "TraversalFeaturesImpl" << endl;
  TRule r = *edge.rule_;
  //cerr << "F: ";
  for (int i=0; i<r.f_.size(); i++) { 
    if (r.f_[i]<0) {
      //cerr << "NT "; 
    } else {
      //cerr << TD::Convert(r.f_[i]) << " ";
    }
  } 
  //cerr << endl;
  //cerr << "E: ";
  for (int i=0; i<r.e_.size(); i++) {
    if (r.e_[i]<=0) {
      //cerr << "NT ";
    } else {
      //cerr << TD::Convert(r.e_[i]) << " ";
    }
  }
  //cerr << endl;
  //cerr << "A: ";
  for (int i=0; i<r.a_.size()/2; i++) {
	//cerr << r.a_[2*i] << "-" << r.a_[2*i+1] << " ";
  }
  //cerr << endl;
  bool nofw = als->prepare(*edge.rule_, ant_contexts, sfw, tfw);
  //cerr << "nofw = " << nofw << endl;
  if (flag_oris) {
    cost=0; bonus=0; bo1=0; bo2=0; bo1_bonus=0; bo2_bonus=0;
    if (!nofw) als->computeOrientationSource(toris,&cost,&bonus,&bo1,&bo1_bonus,&bo2,&bo2_bonus); 
    //cerr << "oris!!!cost: " << cost << " ,bonus:" << bonus << endl; 
    //cerr << "bo1="<< bo1<<", bo1_bonus=" << bo1_bonus << ", bo2=" << bo2 << ", bo2_bonus=" << bo2_bonus << endl;
    features->set_value(oris_,cost); features->set_value(oris_bo1_,bo1); features->set_value(oris_bo2_,bo2);
    //estimated_features->set_value(oris_,bonus); 
    //estimated_features->set_value(oris_bo1_,bo1_bonus); 
    //estimated_features->set_value(oris_bo2_,bo2_bonus);
    //cerr << "here" << endl;
  }
  if (flag_doms) {
    cost=0; bonus=0; bo1=0; bo2=0; bo1_bonus=0; bo2_bonus=0;
    //cerr << "<cost: " << cost << " ,doms_bo1_:" << bo1 << " ,doms_bo2_:" << bo2 << endl; 
    if (!nofw) als->computeDominanceSource(tdoms,&cost,&bonus,&bo1,&bo1_bonus,&bo2,&bo2_bonus); // the bonus will be zero
    //cerr << ">cost: " << cost << " ,doms_bo1_:" << bo1 << " ,doms_bo2_:" << bo2 << endl; 
    features->set_value(doms_,cost); features->set_value(doms_bo1_,bo1); features->set_value(doms_bo2_,bo2);
    //cerr << ">>cost: " << cost << " ,doms_bo1_:" << bo1 << " ,doms_bo2_:" << bo2 << endl; 
                /*estimated_features->set_value(doms_,cost);
                estimated_features->set_value(doms_bo1_,bo1);
                estimated_features->set_value(doms_bo2_,bo2);*/
  }
  if (flag_orit) {
    cost=0; bonus=0; bo1=0; bo2=0; bo1_bonus=0; bo2_bonus=0;
    if (!nofw) als->computeOrientationTarget(torit,&cost,&bonus,&bo1,&bo1_bonus,&bo2,&bo2_bonus); 
    features->set_value(orit_,cost); features->set_value(orit_bo1_,bo1); features->set_value(orit_bo2_,bo2);
                estimated_features->set_value(orit_,bonus);
                estimated_features->set_value(orit_bo1_,bo1_bonus);
                estimated_features->set_value(orit_bo2_,bo2_bonus);
  }
  if (flag_domt) {
    cost=0; bonus=0; bo1=0; bo2=0; bo1_bonus=0; bo2_bonus=0;
    if (!nofw) als->computeDominanceTarget(tdomt,&cost,&bonus,&bo1,&bo1_bonus,&bo2,&bo2_bonus); // the bonus will be zero
    features->set_value(domt_,cost); features->set_value(domt_bo1_,bo1); features->set_value(domt_bo2_,bo2);
                /*estimated_features->set_value(domt_,cost);
                estimated_features->set_value(domt_bo1_,bo1);
                estimated_features->set_value(domt_bo2_,bo2);*/
  }
  int* vcontext = reinterpret_cast<int *>(context);
  if (!nofw) {
    als->SetSourceBorderingFW();
    als->SetTargetBorderingFW();
    als->simplify(vcontext);  
  } else {
    als->simplify_nofw(vcontext);
  }
  //cerr << "state = ";
  for (int idx=0; idx<37; idx++) {
    //cerr << idx << "." << vcontext[idx] << " ";
  }
  //cerr << endl;
}

bool Dwarf::readOrientation(CountTable* table, std::string filename, std::map<WordID,int> *fw) {
  // the input format is
  // source target 0 1 2 3 4 0 1 2 3 4
  // 0 -> MA, 1 -> RA, 2 -> MG, 3 -> RG, 4 -> NO_NEIGHBOR
  // first 01234 corresponds to the left neighbor, the second 01234 corresponds to the right neighbor
  // append 2 more at the end as the total
  
  // word tokenizer 
  // check whether the file exists or not, return false if not

  ReadFile rf(filename);  
  istream& in = *rf.stream();
        table->third = new int[12];
  for (int i=0; i<12; i++) table->third[i]=0;
  while (in) {
    string line, word;
    WordID key1_id, key2_id; 
    getline(in,line);
    if (line=="") break;
    istringstream tokenizer(line);
    ostringstream key;
    tokenizer >> word; 
    if (fw->find(TD::Convert(word))==fw->end()) fw->insert(pair<WordID,int>(TD::Convert(word),1));
    key2_id = TD::Convert(word);
    key << word << " "; tokenizer >> word; key << word;
    key1_id = TD::Convert(key.str());
    int* element = new int[12];
    element[5] = 0;
    for (int i=0; i<5; i++) {
      element[i] = 0;
      if (tokenizer >> word) element[i] = atoi(word.c_str());
      element[5] += element[i];
    }
    element[11] = 0;
    for (int i=6; i<11; i++) {
      element[i] = 0;
      if (tokenizer >> word) element[i] = atoi(word.c_str());
      element[11] += element[i];
    }
    table->first.insert(pair<WordID,int*>(key1_id,element));
    if (table->second.find(key2_id)!=table->second.end()) {  
      for (int i=0; i<12; i++) table->second[key2_id][i]+=element[i];
    } else {
      int* el2 = new int[12];
      for (int i=0; i<12; i++) el2[i] = element[i];
      table->second.insert(pair<WordID,int*>(key2_id,el2));
    }
    for (int i=0; i<12; i++) table->third[i] += element[i];
  }  
  //printTable(*table,12);
  return true;    
}

bool Dwarf::readDominance(CountTable* table, std::string filename, std::map<WordID,int>* fw) {
  // the input format is 
  // source1 source2 target1 target2 0 1 2 3
  // 0 -> dontcase 1->leftfirst 2->rightfirst 3->neither 
  ReadFile rf(filename);
  istream& in = *rf.stream();
  table->third = new int[5];
  for (int i=0; i<5; i++) table->third[i]=0;
  while (in) {
    string line, word;
    getline(in,line);
    if (line=="") break;
    ostringstream key; 
    WordID key1_id, key2_id;
    istringstream tokenizer(line);
    tokenizer >> word; 
    if (fw->find(TD::Convert(word))==fw->end()) fw->insert(pair<WordID,int>(TD::Convert(word),1));
    key << word << " "; tokenizer >> word; 
    if (fw->find(TD::Convert(word))==fw->end()) fw->insert(pair<WordID,int>(TD::Convert(word),1));
    key << word; // take two words as first key
    key2_id = TD::Convert(key.str());
    tokenizer >> word; key << " " << word << " "; tokenizer >> word; key << word;
    key1_id = TD::Convert(key.str());
    int* element = new int[5];
    element[4]=0;
    for (int i=0; i<4; i++) {
      element[i]  = 0;
      if (tokenizer >> word) element[i] = atoi(word.c_str());
      element[4]+=element[i];
    }
    table->first.insert(pair<WordID,int*>(key1_id,element));
    if (table->second.find(key2_id)!=table->second.end()) { 
      for (int i=0; i<5; i++) table->second[key2_id][i]+=element[i];
    } else {
      int* el2 = new int[5]; 
      for (int i=0; i<5; i++) el2[i]=element[i];
      table->second.insert(pair<WordID,int*>(key2_id,el2));
    }
    for (int i=0; i<5; i++) table->third[i] += element[i];
  }
  //printTable(*table,5);
  return true;    
}
