#include "dwarf.h"
#include "tdict.h"
#include "wordid.h"
#include <assert.h>
#include <ostream>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include "filelib.h"
#include <boost/functional/hash.hpp>
#include <tr1/unordered_map>
#include <boost/tuple/tuple.hpp>

using namespace std::tr1;
using namespace boost;
using namespace boost::tuples;

int main(int argc, char** argv) {	
	// argv[0] -> alignment
	// argv[1] -> source
	// argv[2] -> target
	// type    -> 1|2|3|4 OrientationSource|OrientationTarget|DominanceSource|DominanceTarget
	// STDIN   -> function word list 
	string line;
	map<WordID,int> fw;
	while (cin >> line) {
		//cerr << "fw:" << TD::Convert(line) << endl;
		fw.insert(pair<WordID,int>(TD::Convert(line),1));
	}
	ReadFile arf(argv[1]);
	ReadFile frf(argv[2]);
	ReadFile erf(argv[3]);
	istream& areader = *arf.stream(); 
	istream& freader = *frf.stream();
	istream& ereader = *erf.stream();
	//cerr << "0" << endl;
	string fline, eline, aline;
	map<string,int*> result;
	//cerr << "00:" << argv[0] << endl;
	int type = atoi(argv[4]);
	//cerr << "type=" << type << endl;
	int sentid=0;

	//cerr << "1" << endl;
	/*unordered_map<vector<int>,int> oris_hash;
        int key1_el[] = {1,2,3,4};
        std::vector<int> key1(key1_el,key1_el+sizeof(key1_el)/sizeof(int));
        //oris_hash.insert(pair<vector<int>,int>(key1,1234));
        int key2_el[] = {1,2,3,4};*/

        //std::vector<int> key2(key2_el,key2_el+sizeof(key2_el)/sizeof(int));

	Alignment* als = new Alignment();
	while (areader && freader && ereader) {
		als->clearAls();
		sentid++;
		//cerr << sentid << endl;
		getline(areader,aline); getline(freader,fline); getline(ereader,eline);
		//cerr << "A[" << argv[1] << "]: " << aline << endl;
		//cerr << "F[" << argv[2] << "]: " << fline << endl;
		//cerr << "E[" << argv[3] << "]: " << eline << endl;
		vector<WordID> fs,es;
		string a,f,e;
		istringstream fstream(fline);
		//fs.push_back(TD::Convert("<s>"));
		while (fstream >> f) fs.push_back(TD::Convert(f));
		//fs.push_back(TD::Convert("<\\s>"));
		//for (int i=0; i<fs.size(); i++) cerr << i << "." << TD::Convert(fs[i]) << " ";
		//cerr << endl;
		istringstream estream(eline);
		//es.push_back(TD::Convert("<s>"));
		while (estream >> e) es.push_back(TD::Convert(e));
		//es.push_back(TD::Convert("<\\s>"));
		//for (int i=0; i<es.size(); i++) cerr << i << "." << TD::Convert(es[i]) << " ";
		//cerr << endl;
		//cerr << "fs.size()=" << fs.size() << endl;
		//cerr << "es.size()=" << es.size() << endl;
		als->setF(fs); als->setE(es); als->setJ(fs.size()+2); als->setI(es.size()+2);
		istringstream astream(aline);
		while(astream >> a) {
			int hypen = a.find_first_of("-");
	                int j     = atoi(a.substr(0,hypen).c_str());
                	int i     = atoi(a.substr(hypen+1).c_str());
			als->set(j+1,i+1);
		}		
		als->set(0,0); als->set(fs.size()+1,es.size()+1);
		//cerr << *als;
		//if (strcmp(argv[4],"1")==0) { // OrientationSource
		if (type==1) { // OrientationSource
			int orir, oril;
			for (int idx=0; idx<fs.size(); idx++) {
				if (fw[fs[idx]]==1) {
					als->OrientationSource(idx+1,&oril,&orir); // transformed space
					ostringstream oss;
					oss << TD::Convert(fs[idx]) << " " << TD::Convert(als->F2EProjection(idx+1,"_SEP_"));
					string key = oss.str();
					if (result.find(key)==result.end()) {
						int* newel = new int[10];
						for (int i=0; i<10; i++) newel[i]=0;
						result.insert(pair<string,int*>(key,newel));
					}
					//cerr << idx  << ". " << key << ": " << oril << "," << orir << endl;
					if (oril>0 && oril <= 5) result[key][oril-1]+=1;
					if (orir>0 && orir <= 5) result[key][orir+4]+=1;
				}
			}
		//} else if (strcmp(argv[4],"2")==0) { // OrientationTarget
		} else if (type==2) { // OrientationTarget
                        int orir, oril;
                        for (int idx=0; idx<es.size(); idx++) {
                                if (fw[es[idx]]==1) {
					//cerr << "a" << endl;
                                        als->OrientationTarget(idx+1,&oril,&orir);
                                        ostringstream oss;
					//cerr << "2" << endl;
                                        oss << TD::Convert(es[idx]) << " " << TD::Convert(als->E2FProjection(idx+1,"_SEP_"));
					//cerr << "3" << endl;
                                        string key = oss.str();
                                        if (result.find(key)==result.end()) {
                                                int* newel = new int[10];
                                                for (int i=0; i<10; i++) newel[i]=0;
                                                result.insert(pair<string,int*>(key,newel));
                                        }
					//cerr << idx << ". " << key << ": " << oril << "," << orir << endl;
                                        if (oril>0 && oril <= 5) result[key][oril-1]+=1;
                                        if (orir>0 && orir <= 5) result[key][orir+4]+=1;
                                }
                        }	
		//} else if (strcmp(argv[4],"3")==0){
		} else if (type==3){
			int prev=-1;
			string prevf; string currf;
			string preve; string curre;
			for (int idx=0; idx<fs.size(); idx++) {
				if (fw[fs[idx]]==1) {
					currf = TD::Convert(fs[idx]); curre = TD::Convert(als->F2EProjection(idx+1,"_SEP_"));
					if (prev>=0) {
						int d = als->DominanceSource(prev+1,idx+1);
						//cerr << (prev+1) << "," << (idx+1) << " d=" << d << endl;
						ostringstream oss;
						oss<<prevf<<" "<<currf<<" "<<preve<<" "<<curre;
						string key(oss.str());
						if (result.find(key)==result.end()) {
							int* newel = new int[4];
							for (int i=0; i<4; i++) newel[i]=0;
							result.insert(pair<string,int*>(key,newel));
						}
						//cerr << "key=" << key << endl;
						result[key][d]+=1;
					}
					prevf = currf; preve = curre; prev  = idx;
					//cerr << "prevf=" << prevf << ", preve=" << preve << ", prev=" << idx << endl;
				}
			}	
		//} else if (strcmp(argv[4],"4")==0) { // DominanceTarget
		} else if (type==4) { // DominanceTarget
                        int prev=-1;
                        string prevf; string currf;
                        string preve; string curre;
                        for (int idx=0; idx<es.size(); idx++) {
                                if (fw[es[idx]]==1) {
                                        curre = TD::Convert(es[idx]); currf = TD::Convert(als->E2FProjection(idx+1,"_SEP_"));
                                        if (prev>=0) {
                                                int d = als->DominanceTarget(prev+1,idx+1);
						//cerr << (prev+1) << "," << (idx+1) << " d=" << d << endl;
                                                ostringstream oss;
                                                oss<<preve<<" "<<curre<<" "<<prevf<<" "<<currf;
                                                string key(oss.str());
                                                if (result.find(key)==result.end()) {
                                                        int* newel = new int[4];
                                                        for (int i=0; i<4; i++) newel[i]=0;
                                                        result.insert(pair<string,int*>(key,newel));
                                                }
                                                //cerr << "key=" << key << endl;
                                                result[key][d]+=1;
                                        }
                                        prevf = currf; preve = curre; prev  = idx;
                                        //cerr << "prevf=" << prevf << ", preve=" << preve << ", prev=" << idx << endl;
                                }
                        }

		}
	}			
	for (map<string,int*>::iterator it = result.begin(); it != result.end(); ++it) {
		int* el = it->second;	
		cout << it->first;
		int max=(type>2)?4:10;
		for (int i=0; i<max; i++) cout << " " << el[i];
		cout << endl;
	}
	
}
