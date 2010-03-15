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
#include <boost/functional/hash.hpp>
#include <tr1/unordered_map>
#include <boost/tuple/tuple.hpp>

using namespace std;
using namespace std::tr1;
using namespace boost::tuples;
using namespace boost;

Alignment::Alignment() {
  //unordered_map<std::vector<WordID>,int> XX;
  _I=0;
  _J=0;
  SourceFWAntsIdxs = new int*[MAX_ARITY];
  TargetFWAntsIdxs = new int*[MAX_ARITY];
  SourceAntsIdxs = new int*[MAX_ARITY];
  TargetAntsIdxs = new int*[MAX_ARITY];
  AntsAl = new int*[MAX_ARITY];
  for (int idx=0; idx<MAX_ARITY; idx++) {
    int*tmp = new  int[40];
    SourceAntsIdxs[idx] = tmp;
    int *tmp1 = new int[40];
    SourceFWAntsIdxs[idx] = tmp1;
  }
  TargetAntsIdxs = new int*[MAX_ARITY];
  for (int idx=0; idx<MAX_ARITY; idx++) {
    int *tmp = new int[40];
    TargetAntsIdxs[idx] = tmp;
    int *tmp1 = new int[40];
    TargetFWAntsIdxs[idx] = tmp1;
  }
  for (int idx=0; idx<MAX_ARITY; idx++) {
    int *tmp = new int[40];
    AntsAl[idx] = tmp;
  }    
  for (int j=0; j<MAX_WORDS; j++) 
    for (int i=0; i<MAX_WORDS; i++) _matrix[j][i]=false; 
  for (int j=0; j<MAX_WORDS; j++) {
    _tSpan[j][0]=MINIMUM_INIT;
    _sSpan[j][1]=MAXIMUM_INIT;
  }
  for (int i=0; i<MAX_WORDS; i++) {
    _sSpan[i][0]=MINIMUM_INIT;
    _sSpan[i][1]=MAXIMUM_INIT;
  }
}

void Alignment::set(int j,int i) {
// create a link between j and i, update their corresponding span accordingly
  //assert(0<=j && j<MAX_WORDS);
  //assert(0<=i && i<MAX_WORDS);
  if (0<=j && j<MAX_WORDS && 0<=i && i<MAX_WORDS) {
    _matrix[j][i] = true;
    //cerr << "before" << endl;
    //cerr << "tSpan[j]=" << _tSpan[j][0] << "," << _tSpan[j][1] << endl; 
    //cerr << "sSpan[i]=" << _sSpan[i][0] << "," << _sSpan[i][1] << endl; 
    //cerr << "after" << endl;
    if (i<_tSpan[j][0]) _tSpan[j][0]=i;
    if (_tSpan[j][1]<i) _tSpan[j][1]=i;
    if (j<_sSpan[i][0]) _sSpan[i][0]=j;
    if (_sSpan[i][1]<j) _sSpan[i][1]=j;
    //cerr << "tSpan[j]=" << _tSpan[j][0] << "," << _tSpan[j][1] << endl; 
    //cerr << "sSpan[i]=" << _sSpan[i][0] << "," << _sSpan[i][1] << endl; 
  /*} else {
    if (j<0 && i>=0 && i<MAX_WORDS) {
      _sSpan[i][0] = MINIMUM_INIT; _sSpan[i][1] = MAXIMUM_INIT;
    }
    if (i<0 && j>=0 && j<MAX_WORDS) {
      _tSpan[j][0] = MINIMUM_INIT; _tSpan[j][1] = MAXIMUM_INIT;
    } */
  }
  if (j+1>_J) _J=j+1;
  if (i+1>_I) _I=i+1;
  //cerr << "J=" << _J << " I=" << _I << endl;
  //cerr << j << "-" << i << endl;
}

void Alignment::reset(int j,int i) { //probably won't be used, since the alignment is not dynamic
// remove the link between j and i, update their corresponding span accordingly
  assert(0<=j && j<MAX_WORDS);
  assert(0<=i && i<MAX_WORDS);
  _matrix[j][i] = false;
  if (j==_sSpan[i][0] || j==_sSpan[i][1]) {
    int min=MINIMUM_INIT;
    int max=MAXIMUM_INIT;
    for (int idx=_sSpan[i][0]; idx<=_sSpan[i][1]; idx++) {
      if (_matrix[idx][i]) {
        if (idx<min) min=idx;
        if (idx>max) max=idx;
      }
    }
    _sSpan[i][0]=min;
    _sSpan[i][1]=max;
  }
  if (i==_tSpan[j][0] || j==_tSpan[j][i]) {
    int min=MINIMUM_INIT;
    int max=MAXIMUM_INIT;
    for (int idx=_tSpan[i][0]; idx<=_tSpan[i][1]; idx++) {
      if (_matrix[j][idx]) {
        if (idx<min) min=idx;
        if (idx>min) max=idx;
      }
    }
    _tSpan[j][0]=min;
    _tSpan[j][1]=max;
  }
}

int Alignment::targetOf(int j, int start) {
  assert(j>=0);
  if (start==-1) start = _tSpan[j][0];
  if (_tSpan[j][0]==MINIMUM_INIT) return -1;
  for (int idx=start; idx<=_tSpan[j][1]; idx++) {
    if (_matrix[j][idx]) return idx;
  }
  return -1;
}

int Alignment::sourceOf(int i, int start) {
  assert(i>=0);
  if (start==-1) start = _sSpan[i][0];
  //cerr << "sourceOf(" << i << "),";
  //cerr << "start = " << start << endl;
  if (_sSpan[i][0]==MINIMUM_INIT) return -1;
  for (int idx=start; idx<=_sSpan[i][1]; idx++) {
    //cerr << "o" << idx << endl;
    if (_matrix[idx][i]) return idx;
  }
  //cerr << "p" << endl;
  return -1;
}

void Alignment::clearAls(int prevJ, int prevI) {
  //cerr << "prevJ=" << prevJ << ", prevI=" << prevI << endl;
  for (int j=0; j<=prevJ; j++) {
    for (int i=0; i<prevI; i++) {
      _matrix[j][i]=false;
    }
  }
  for (int j=0; j<=prevJ; j++) {
    _tSpan[j][0] = MINIMUM_INIT;
    _tSpan[j][1] = MAXIMUM_INIT;
  }
  for (int i=0; i<=prevI; i++) {
    _sSpan[i][0] = MINIMUM_INIT;
    _sSpan[i][1] = MAXIMUM_INIT;
  }
  _J=0;
  _I=0;
}

int Alignment::DominanceSource(int fw1, int fw2) {
  // 0 -> neither, 1 -> leftFirst, 2 -> rightFirst, 3 -> dontCare
  //cerr << "DominanceSource(" << fw1 << "," << fw2 << ")" << endl;
  int dom = 0;
  curr_al.push_back(fw1); curr_al.push_back(fw2);
  if (doms_hash.find(curr_al)==doms_hash.end()) {
    int* block = blockSource(fw1,fw2);
    //cerr << block[0] << "," << block[1] << "," << block[2] << "," << block[3] << endl;
    if (block[0]==fw1) dom+=1;
    if (block[1]==fw2) dom+=2;
    delete block;
    doms_hash.insert(pair<vector<int>,int>(curr_al,dom));
  } else {
    dom = doms_hash[curr_al];
  }
  curr_al.pop_back(); curr_al.pop_back();
  return dom;
}

int Alignment::DominanceTarget(int fw1, int fw2) {
  int dom = 0;
  curr_al.push_back(fw1); curr_al.push_back(fw2);
  if (domt_hash.find(curr_al)==domt_hash.end()) {
    int* block = blockTarget(fw1,fw2);
    if (block[2]==fw1) dom+=1;
    if (block[3]==fw2) dom+=2;
    delete block;
    domt_hash.insert(pair<vector<int>,int>(curr_al,dom));
  } else {
    dom = domt_hash[curr_al];
  }
  curr_al.pop_back(); curr_al.pop_back();
  return dom;
}

void Alignment::OrientationSource(int fw, int* oril, int* orir, bool Lcompute, bool Rcompute) {
  // Left Neighbor
  // 1 -> MA, 2 -> RA, 3 -> MG, 4 -> RG, 5 -> Other
  //cerr << "OrientationSource(" << fw << ")" << endl;
  if (!Lcompute && !Rcompute) return;
  curr_al.push_back(fw);
  *oril=0;
  *orir=0;
  int lr=0;
  if (oris_hash.find(curr_al)==oris_hash.end()) {
    //cerr << "oris_hash miss" << endl;
    // Find first aligned word N0 to the left of fw
    int fw0 = fw;
    int fw1 = fw;
    int N0=fw-1;
    while (N0>=0) {
      if (minTSpan(N0)!=MINIMUM_INIT) break;
      N0--;
    }
    int N1=fw+1;
    while (N1<_J) {
      if (minTSpan(N1)!=MINIMUM_INIT) break;
      N1++;
    }
    if (minTSpan(fw)==MINIMUM_INIT) {
      fw0 = N1; fw1 = N0;
      //cerr << "minTSpan(fw)==MINIMUM_INIT, thus fw0=" << fw0 << ", fw1=" << fw1 << endl;
    }
    //cerr << "fw0=" << fw0 << ", fw1=" << fw1 << ", N0=" << N0 << ", N1=" << N1 << endl;
    if (maxTSpan(N0)<minTSpan(fw0) || maxTSpan(fw0)<minTSpan(N0)) {
            //cerr << "N0=" << minTSpan(N0) << "-" << maxTSpan(N0);
      //cerr << "fw=" << minTSpan(fw0) << "-" << maxTSpan(fw0) << endl;
      int *block = blockTarget(minTSpan(N0),maxTSpan(N0));
      if (block[0]<=fw0 && fw0<=block[1]) *oril=5;
      delete block;
      if (*oril==0) {
        block = blockTarget(minTSpan(fw0),maxTSpan(fw0));
        if (block[0]<=N0 && N0<=block[1]) *oril=5;
        delete block;
      }
      if (*oril==0) {
        if (maxTSpan(N0)<minTSpan(fw0)) {// if N0 is monotone
          *oril=1;
          block = blockTarget(maxTSpan(N0),minTSpan(fw0)-1);
          if (block[0] <= fw0 && fw0 <= block[1]) *oril+=2;
        } else { //if (maxTSpan(fw0)<minTSpan(N0)) { // if NO is non-monotone
          *oril=2;
          block = blockTarget(maxTSpan(fw0)+1,minTSpan(N0));
          if (block[0] <= fw0 && fw0 <= block[1]) *oril+=2;
        }
        delete block;
      }
    } else {
      *oril=5;
    }
    //cerr << "oril =" << *oril << endl;
    // Right neighbor
    if (maxTSpan(N1)<minTSpan(fw1) || maxTSpan(fw1)<minTSpan(N1)) {
      int* block = blockTarget(minTSpan(N1),maxTSpan(N1));
      if (block[0]<=fw1 && fw1<=block[2]) *orir=5;
      delete block;
      if (*orir==0) {
        block = blockTarget(minTSpan(fw1),maxTSpan(fw1));
        if (block[0]<=N1 && N1 <=block[1]) *orir=5;
        delete block;
      }
      //cerr << "N1=" << minTSpan(N1) << "-" << maxTSpan(N1);
      //cerr << "fw1=" << minTSpan(fw1) << "-" << maxTSpan(fw1) << endl;
      if (*orir==0) {
        if (maxTSpan(fw1)<minTSpan(N1)) { // if N1 is monotone
          *orir = 1;
          block = blockTarget(maxTSpan(fw1)+1,minTSpan(N1));
          if (block[0] <= fw1 && fw1 <= block[1]) *orir+=2;
          delete block;
        } else {// if (maxTSpan(N1)<minTSpan(fw1)) { // if N1 is non-monotone
          *orir = 2;
          block = blockTarget(maxTSpan(N1),minTSpan(fw1)-1);
          if (block[0] <= fw1 && fw1 <= block[1]) *orir+=2;
          delete block;
        }
      }
    } else {
      *orir = 5;
    }
    //cerr << "orir =" << *orir << endl;
    lr = link(*oril,*orir);
    oris_hash.insert(pair<vector<int>,int>(curr_al,lr));  
  } else {
    //cerr << "oris_hash hit" << endl;
    lr = oris_hash[curr_al];  
  }
  if (Lcompute) *oril = source(lr);
  if (Rcompute) *orir = target(lr);
  //cerr << "oris_hash returns L=" << *oril << ", R=" << *orir << endl; 
  curr_al.pop_back();
}

void Alignment::OrientationTarget(int fw, int*oril, int*orir, bool Lcompute, bool Rcompute) {
  //cerr << "OrientationTarget " << fw << endl;
        // Left Neighbor
  if (!Lcompute && !Rcompute) return;
  *oril=0;
  *orir=0;
  curr_al.push_back(fw);
  int lr = 0;
  if (orit_hash.find(curr_al)==orit_hash.end()) {
    // Find first aligned word N0 to the left of fw
    int N0=fw-1;
    while (N0>=0) {
      if (minSSpan(N0)!=MINIMUM_INIT) break;
      N0--;
    }
    int N1=fw+1;
    while (N1<_I) {
      if (minSSpan(N1)!=MINIMUM_INIT) break;
      N1++;
    }
    int fw0 = fw; int fw1 = fw;
    if (minSSpan(fw)==MINIMUM_INIT) {
      fw0=N1; fw1=N0;
    }
    //cerr << "fw0:" << fw0 << ", fw1:" << fw1 << ", N0:" << N0 << ", N1:" << N1 << endl;
    //cerr << "minSSpan(N0)=" << minSSpan(N0) << " maxSSpan(N0)=" << maxSSpan(N0) << " minSSpan(fw0)="<< minSSpan(fw0) << " maxSSpan(fw0)=" << maxSSpan(fw0) << endl;
    //cerr << "minSSpan(fw1)=" << minSSpan(fw1) << " maxSSpan(fw1)=" << maxSSpan(fw1) << " minSSpan(N1)="<< minSSpan(N1) << " maxSSpan(N1)=" << maxSSpan(N1) << endl;
    if (maxSSpan(N0)<minSSpan(fw0) || maxSSpan(fw0)<minSSpan(N0)) {
      int *block = blockSource(minSSpan(N0),maxSSpan(N0));
      if (block[2]<=fw0 && fw0<=block[3])  //source span of fw0 subsumes NO's or the other way around
        *oril=5;
      delete block;
      if (*oril==0) {
        block = blockSource(minSSpan(fw0), maxSSpan(fw0));
        if (block[2] <= N0 && N0 <= block[3]) *oril=5;
        delete block;
      }
      if (*oril==0) {  
        if (maxSSpan(N0)<minSSpan(fw0)) {// if N0 is monotone
          //cerr << "maxSSpan(N0)="<< maxSSpan(N0) << "<minSSpan(fw0)=" << minSSpan(fw0) << endl;
          *oril=1;
          block = blockSource(maxSSpan(N0),minSSpan(fw0)-1);
          //cerr << block[0] << "," << block[1] << "," << block[2] << "," << block[3] << endl;
          if (block[2] <= fw0 && fw0 <= block[3]) *oril+=2;
        } else { // (maxSSpan(fw0)<minSSpan(N0)) // if NO is non-monotone
          //cerr << "maxSSpan(fw0)=" << maxSSpan(fw0) << "<minSSpan(N0)=" << minSSpan(N0) << endl;
          *oril=2;
          int *block = blockSource(maxSSpan(fw0)+1,minSSpan(N0));
          if (block[2] <= fw0 && fw0 <= block[3]) *oril+=2;
        }
        delete block;
      }
    } else { //source span of fw0 subsumes NO's or the other way around
      *oril=5;
    }
    //cerr << "oril = " << *oril << endl;
    // Right Neighbor
    if (maxSSpan(N1)<minSSpan(fw1) || maxSSpan(fw1)<minSSpan(N1)) {
      int *block = blockSource(minSSpan(N1),maxSSpan(N1));
      if (block[2]<=fw1 && fw1<=block[3]) *orir=5; 
      delete block;
      if (*orir==0) {
        block = blockSource(minSSpan(fw1),maxSSpan(fw1));
        if (block[2] <= N1 && N1 <= block[3]) *orir=5;
        delete block;
      }
      if (*orir==0) {
        if (maxSSpan(fw1)<minSSpan(N1)) { // if N1 is monotone
          *orir=1;
          block = blockSource(maxSSpan(fw1)+1,minSSpan(N1));
          if (block[2] <= fw1 && fw1 <= block[3]) *orir+=2;
        } else { //if (maxSSpan(N1)<minSSpan(fw1)) { // if N1 is non-monotone
          *orir=2;
          block = blockSource(maxSSpan(N1),minSSpan(fw1)-1);
          if (block[2] <= fw1 && fw1 <= block[3]) *orir+=2;
        }
        delete block;
      }
    } else {
      *orir=5;
    }
    //cerr << "orir = " << *orir << endl;
    lr = link(*oril,*orir);
    orit_hash.insert(pair<vector<int>,int>(curr_al,lr));
  } else {
    lr = orit_hash[curr_al];
  }
  if (Lcompute) *oril=source(lr);
  if (Rcompute) *orir=target(lr);
  curr_al.pop_back();
}

int* Alignment::blockSource(int idx1, int idx2) {
  //cerr << "blockSource ("<<idx1<<","<<idx2<<")"<<endl;
  int *curr = new int[4];
  curr[0]=idx1; curr[1]=idx2; curr[2]=MINIMUM_INIT; curr[3]=MAXIMUM_INIT;
  for (int j=curr[0]; j<=curr[1]; j++) {
    curr[2] = least(curr[2],_tSpan[j][0]);
    curr[3] =  most(curr[3],_tSpan[j][1]);
  }
  int next[4];
  next[0]=curr[0]; next[1]=curr[1];
  for (int i=curr[2]; i<=curr[3]; i++) {
    next[0] = least(next[0],_sSpan[i][0]);
    next[1] =  most(next[1],_sSpan[i][1]);
  } 
  next[2] = curr[2]; next[3]= curr[3];
  //cerr << "0curr" << curr[0] << "," << curr[1] << "," << curr[2] << "," << curr[3] << endl;
  //cerr << "0next" << next[0] << "," << next[1] << "," << next[2] << "," << next[3] << endl;
  int idx=1;
  do {
    // update the current
    for (int j=next[0]; j<curr[0]; j++) {
      curr[2] = least(curr[2],_tSpan[j][0]);
      curr[3] =  most(curr[3],_tSpan[j][1]);
    }
    for (int j=curr[1]+1; j<=next[1]; j++) {
      curr[2] = least(curr[2],_tSpan[j][0]);
      curr[3] =  most(curr[3],_tSpan[j][1]);
    }
    curr[0] = next[0]; curr[1] = next[1]; 
    //cerr << idx << "curr" << curr[0] << "," << curr[1] << "," << curr[2] << "," << curr[3] << endl;
    if (curr[2]==next[2] && curr[3]==next[3]) break;
    // prepare for the next 
    for (int i=curr[2]; i<next[2]; i++) {
      next[0]= least(next[0],_sSpan[i][0]);
      next[1]=  most(next[1],_sSpan[i][1]);
    }
    for (int i=next[3]+1; i<=curr[3]; i++) {
      next[0] = least(next[0],_sSpan[i][0]);
      next[1] =  most(next[1],_sSpan[i][1]);
    }
    next[2] = curr[2]; next[3]= curr[3];
    //cerr << idx << "next" << next[0] << "," << next[1] << "," << next[2] << "," << next[3] << endl;
    idx++;
  } while(1);
  //cerr << "return curr" << curr[0] << "," << curr[1] << "," << curr[2] << "," << curr[3] << endl;
  return curr;
}

int* Alignment::blockTarget(int idx1, int idx2) {
  int *curr = new int[4];
  //cerr << "blockTarget (" << idx1 << "," << idx2 << ")" << endl;
  curr[0]=MINIMUM_INIT; curr[1]=MAXIMUM_INIT; curr[2]=idx1; curr[3]=idx2;
        for (int i=curr[2]; i<=curr[3]; i++) {
                curr[0] = least(curr[0],_sSpan[i][0]);
                curr[1] =  most(curr[1],_sSpan[i][1]);
        }
        int next[4];
        next[2]=curr[2]; next[3]=curr[3];
        for (int j=curr[0]; j<=curr[1]; j++) {
                next[2] = least(next[2],_tSpan[j][0]);
                next[3] =  most(next[3],_tSpan[j][1]);
        }
        next[0] = curr[0]; next[1]= curr[1];
        //cerr << "0curr" << curr[0] << "," << curr[1] << "," << curr[2] << "," << curr[3] << endl;
        //cerr << "0next" << next[0] << "," << next[1] << "," << next[2] << "," << next[3] << endl;
        int idx=1;
        do {
                // update the current
                for (int i=next[2]; i<curr[2]; i++) {
                        curr[0] = least(curr[0],_sSpan[i][0]);
                        curr[1] =  most(curr[1],_sSpan[i][1]);
                }
                for (int i=curr[3]+1; i<=next[3]; i++) {
                        curr[0] = least(curr[0],_sSpan[i][0]);
                        curr[1] =  most(curr[1],_sSpan[i][1]);
                }
                curr[2] = next[2]; curr[3] = next[3];
                //cerr << idx << "curr" << curr[0] << "," << curr[1] << "," << curr[2] << "," << curr[3] << endl;
                if (curr[0]==next[0] && curr[1]==next[1]) break;
                // prepare for the next
                for (int j=curr[0]; j<next[0]; j++) {
                        next[2]= least(next[2],_tSpan[j][0]);
                        next[3]=  most(next[3],_tSpan[j][1]);
                }
                for (int j=next[1]+1; j<=curr[1]; j++) {
                        next[2] = least(next[2],_tSpan[j][0]);
                        next[3] =  most(next[3],_tSpan[j][1]);
                }
                next[0] = curr[0]; next[1]= curr[1];
                //cerr << idx << "next" << next[0] << "," << next[1] << "," << next[2] << "," << next[3] << endl;
                idx++;
        } while(1);
  //cerr << "return curr" << curr[0] << "," << curr[1] << "," << curr[2] << "," << curr[3] << endl;
  return curr;
}

int Alignment::firstSourceAligned(int start) {
  for (int j=start; j<_J; j++) 
    if (_tSpan[j][0]!=MINIMUM_INIT) return j;
  return -1;
}

int Alignment::lastSourceAligned(int end) {
  for (int j=end; j>=0; j--)
    if (_tSpan[j][0]!=MINIMUM_INIT) return j;
  return -1;
}

int Alignment::firstTargetAligned(int start) {
  for (int i=start; i<_I; i++) 
    if (_sSpan[i][0]!=MINIMUM_INIT) return i;
  return -1;
}

int Alignment::lastTargetAligned(int end) {
  for (int i=end; i>=0; i--) 
    if (_sSpan[i][0]!=MINIMUM_INIT) return i;
  return -1;
}

void Alignment::SetSourceBorderingFW() {
  if (SourceFWIdxs[0]>2) {
    //int firstAligned  = firstSourceAligned(1); // disregard start phrase boundary
    //int lastAligned   = lastSourceAligned(_J-2); // disregard end   phrase boundary 
    //cerr << "firstAligned="<< fas << ", lastAligned="<<las<<endl;
    int firstCut = 1;
    for (int j=2; j<=SourceFWIdxs[0]; j++) {
      if (SourceFWIdxs[3*j-2]>fas) break;
      firstCut=j;
    }
    //cerr << "firstCut = " << firstCut;
    int lastCut  = SourceFWIdxs[0];
    for (int j=SourceFWIdxs[0]-1; j>=0; j--) {
      if (SourceFWIdxs[3*j-2]<las) break;
      lastCut=j;
    }
    //cerr << ", lastCut = " << lastCut << endl;
    if (firstCut>=lastCut) return;
    int delta = 0;
    for (int j=lastCut; j<=SourceFWIdxs[0]; j++) {
      delta++;
      SourceFWIdxs[3*(firstCut+delta)-2]=SourceFWIdxs[3*j-2];
      SourceFWIdxs[3*(firstCut+delta)-1]=SourceFWIdxs[3*j-1];
      SourceFWIdxs[3*(firstCut+delta)]  =SourceFWIdxs[3*j];
    }
    SourceFWIdxs[0]=firstCut+delta;
  }
}

void Alignment::SetTargetBorderingFW() {
  //cerr << "SetTargetBorderingFW()" << endl;
  if (TargetFWIdxs[0]>2) {
    //cerr << "firstAligned="<< fat << ", lastAligned="<<lat<<endl;
                int firstCut = 1;
                for (int j=2; j<=TargetFWIdxs[0]; j++) {
                        if (TargetFWIdxs[3*j-2]>fat) break;
                        firstCut=j;
                }
    //cerr << "firstCut = " << firstCut;
                int lastCut  = TargetFWIdxs[0];
                for (int j=TargetFWIdxs[0]-1; j>=0; j--) {
                        if (TargetFWIdxs[3*j-2]<lat) break;
                        lastCut=j;
                }
    //cerr << ", lastCut = " << lastCut << endl;
    if (firstCut>=lastCut) return;
                int delta = 0;
                for (int j=lastCut; j<=TargetFWIdxs[0]; j++) {
      delta++;
                        TargetFWIdxs[3*(firstCut+delta)-2]=TargetFWIdxs[3*j-2];
                        TargetFWIdxs[3*(firstCut+delta)-1]=TargetFWIdxs[3*j-1];
                        TargetFWIdxs[3*(firstCut+delta)]  =TargetFWIdxs[3*j];
      //cerr << "moving from " << j << " to " << (firstCut+delta) << endl;
                }
    //cerr << "TargetFWIdxs[0] = " << TargetFWIdxs[0] << endl;
    TargetFWIdxs[0]=firstCut+delta;
    //cerr << "TargetFWIdxs[0] = " << TargetFWIdxs[0] << endl;
  }
}

void Alignment::fillFWIdxs(int* state, int fas, int las, int fat, int lat) {
  //cerr << "fillFWIdxs ("<< fas <<","<< las<<"," << fat <<"," << lat << ")" << endl;
  if (fas==las) las+=1;
  if (fat==lat) lat+=1;
  //cerr << "adjusted fillFWIdxs ("<< fas <<","<< las<<"," << fat <<"," << lat << ")" << endl;
  for (int idx=0; idx<12; idx++) state[idx]=-1;
  if (SourceFWIdxs[0]<=2) {
    if (SourceFWIdxs[0]>=1) {state[0]=SourceFWIdxs[1]; state[1]=SourceFWIdxs[2]; state[2]=SourceFWIdxs[3];}
    if (SourceFWIdxs[0]==2) {state[3]=SourceFWIdxs[4]; state[4]=SourceFWIdxs[5]; state[5]=SourceFWIdxs[6];}
  } else {
    if (SourceFWIdxs[1]>fas) {
      state[0]=SourceFWIdxs[1]; state[1]=SourceFWIdxs[2]; state[2]=SourceFWIdxs[3];
    } else {
      ostringstream issf; ostringstream isse;
      for (int idx=1; idx<=SourceFWIdxs[0]; idx++) {
        if (SourceFWIdxs[3*idx-2]>las) break;
        if (idx>1) { issf << " "; isse << " ";};
        issf << TD::Convert(SourceFWIdxs[3*idx-1]);
        isse << TD::Convert(SourceFWIdxs[3*idx]);
        state[0]=SourceFWIdxs[3*idx-2];
        //cerr << "setting state[0] to " << state[0] << endl;
        if (state[0]>=fas) break;
      }
      if (state[0]>=0) {
        state[1]=TD::Convert(issf.str())*-1; state[2]=TD::Convert(isse.str()); //multiplying source with -1 as marker
        //cerr << "First Source FW: " << issf.str() << "," << isse.str() << endl;
      }
    }
    if (SourceFWIdxs[SourceFWIdxs[0]*3-2]==las) {
      state[3]=SourceFWIdxs[SourceFWIdxs[0]*3-2];
      state[4]=SourceFWIdxs[SourceFWIdxs[0]*3-1]; 
      state[5]=SourceFWIdxs[SourceFWIdxs[0]*3];
    } else {
                        int lastCut = SourceFWIdxs[0];
      for (int j=lastCut-1; j>=state[0]+1; j--) {
        if (SourceFWIdxs[3*j-2]==state[0]) break;
        if (SourceFWIdxs[3*j-2]<las) break;
        lastCut=j;
      }
      state[3]=SourceFWIdxs[3*lastCut-2];
      ostringstream issf; ostringstream isse;
      for (int idx=lastCut; idx<=SourceFWIdxs[0]; idx++) {
        if (idx>lastCut) { issf << " "; isse << " ";};
        issf << TD::Convert(SourceFWIdxs[3*idx-1]);
        isse << TD::Convert(SourceFWIdxs[3*idx]);
      }
      if (state[3]>=0) {
        //cerr << "Last Source FW: " << issf.str() << "," << isse.str() << endl;
        state[4]=TD::Convert(issf.str())*-1; state[5]=TD::Convert(isse.str()); 
      }
      //multiplying source with -1 as compound marker
    }
  }
  if (TargetFWIdxs[0]<=2) {
                if (TargetFWIdxs[0]>=1) {state[6]=TargetFWIdxs[1]; state[7]=TargetFWIdxs[2]; state[8]=TargetFWIdxs[3];}
                if (TargetFWIdxs[0]==2) {state[9]=TargetFWIdxs[4]; state[10]=TargetFWIdxs[5]; state[11]=TargetFWIdxs[6];}
        } else {
                if (TargetFWIdxs[1]>fat) { //shouldn't come here if SetTargetBorderingFW is invoked
                        state[6]=TargetFWIdxs[1]; state[7]=TargetFWIdxs[2]; state[8]=TargetFWIdxs[3];
                } else {
                        ostringstream issf; ostringstream isse;
                        for (int idx=1; idx<=TargetFWIdxs[0]; idx++) {
                                if (TargetFWIdxs[3*idx-2]>fat) break;
                                if (idx>1) { issf << " "; isse << " ";};
                                issf << TD::Convert(TargetFWIdxs[3*idx-1]);
                                isse << TD::Convert(TargetFWIdxs[3*idx]);
                                state[6]=TargetFWIdxs[3*idx-2];
                        }
      //cerr << "First Target FW: " << issf.str() << "," << isse.str() << endl;
                        state[7]=TD::Convert(issf.str()); state[8]=TD::Convert(isse.str())*-1; 
      //multiplying target with -1 as compound marker
                }
                if (TargetFWIdxs[TargetFWIdxs[0]*3-2]==lat) {
                        state[9]=TargetFWIdxs[TargetFWIdxs[0]*3-2];
                        state[10]=TargetFWIdxs[TargetFWIdxs[0]*3-1];
                        state[11]=TargetFWIdxs[TargetFWIdxs[0]*3];
                } else {
                        int lastCut = TargetFWIdxs[0];
                        for (int j=lastCut-1; j>=1; j--) {
        //cerr << TargetFWIdxs[3*j-2] << endl;
        if (TargetFWIdxs[3*j-2]<=state[9]) break;
                                if (TargetFWIdxs[3*j-2]<lat) break;
                                lastCut=j;
      }
                        state[9]=TargetFWIdxs[3*lastCut-2];
      //cerr << "lastCut " << lastCut << endl;
                        ostringstream issf; ostringstream isse;
                        for (int idx=lastCut; idx<=TargetFWIdxs[0]; idx++) {
                                if (idx>lastCut) { issf << " "; isse << " ";};
                                issf << TD::Convert(TargetFWIdxs[3*idx-1]);
                                isse << TD::Convert(TargetFWIdxs[3*idx]);
                        }
      //cerr << "Last Target FW: " << issf.str() << "," << isse.str() << endl;
                        state[10]=TD::Convert(issf.str()); state[11]=TD::Convert(isse.str())*-1; 
      //multiplying target with -1 as compound marker
                }
  }

}

void Alignment::SplitIfViolateDanglingTargetFWIdxs(vector<int *>*blocks, int* block, vector<int>danglings) {
  //cerr << "SplitIfViolateDanglingTargetFWIdxs[" << block[0] << "," << block[1] << "," << block[2] << "," << block[3] << "]" << endl;
  if (danglings.size()==0) {
    //cerr << "4cpushing " << block[0] << "," << block[1] << "," << block[2] << "," << block[3] << endl;
    blocks->push_back(block); return; 
  }
  int currIdx = block[2];
  int i_dangling = 0;
  while (block[2]>danglings[i_dangling]) 
    if (++i_dangling >= danglings.size()) break;
  if (i_dangling>=danglings.size()) {
    //cerr << "4apushing " << block[0] << "," << block[1] << "," << block[2] << "," << block[3] << endl;
    blocks->push_back(block); return;}
  if (block[3]<danglings[i_dangling]) {
    //cerr << "4bpushing " << block[0] << "," << block[1] << "," << block[2] << "," << block[3] << endl;
    blocks->push_back(block); return;}
  //cerr << "i_dangling = " << i_dangling << endl;
  int anchorIdx = danglings[i_dangling];  
  //cerr << "anchorIdx = " << anchorIdx << endl;
  do {
    while(currIdx<anchorIdx) {
      bool isMoved = false;
      for (int idx=anchorIdx-1; idx>=currIdx; idx--) {
        int *nublock = blockTarget(currIdx,idx);
        if (nublock[2]==currIdx && nublock[3]==idx) {
          if (nublock[1]!=MINIMUM_INIT) {
            blocks->push_back(nublock);
            //cerr << "4pushing " << nublock[0] << "," << nublock[1] << "," << nublock[2] << "," << nublock[3] << endl;
          }
          isMoved = true;
          currIdx=idx+1; break;
        }
        delete nublock;
      }
      if (!isMoved) {
        int source = sourceOf(currIdx);
        while (source>=0) {
          if (source >= block[0]) {
            int* nublock = new int[4];
            nublock[0]=source; nublock[1]=source; nublock[2]=currIdx; nublock[3]=currIdx;
            //cerr << "6pushing " << nublock[0] << "," << nublock[1] << "," << nublock[2] << "," << nublock[3] << endl;
            blocks->push_back(nublock);
          }
          source = sourceOf(currIdx,source+1);
        }
        currIdx++;
      }
    }
    currIdx=anchorIdx+1;
    anchorIdx=block[3];
    if (i_dangling+1<danglings.size()) anchorIdx=danglings[++i_dangling];
  } while(currIdx<=block[3]);
}

void Alignment::simplify(int* ret) {
  // the idea is to create blocks of maximal consistent alignment in between a pair of function words
  // exceptional cases include: one to non-contiguous many (or vice versa) -> treat this as one alignment each
  // record all function word indexes first, important because it may be unaligned
  //cerr << "begin simplify" << endl;
  reset(0,0); reset(_J-1,_I-1); // remove the phrase boundary alignments, NEED TO CHECK AGAIN !!!
  if (SourceFWIdxs[0]+TargetFWIdxs[0]==0) { // return singleton
    //cerr << "no function words" << endl;
    for (int idx=0; idx<12; idx++) ret[idx]=-1;
    ret[12]=1; ret[13]=0; ret[14]=0; // 0-0
    fillFWIdxs(ret,0,0,0,0);  
    return;
  }
  // create key for simplify_hash
  // #al, al1, ... al2, #sfw, sfw1, ... , #tfw, tfw1, ...
  curr_al.insert(curr_al.begin(),curr_al.size());
  curr_al.push_back(SourceFWIdxs[0]);
  for (int i=1; i<=SourceFWIdxs[0]; i++) curr_al.push_back(SourceFWIdxs[3*i-2]);
  curr_al.push_back(TargetFWIdxs[0]);
  for (int i=1; i<=TargetFWIdxs[0]; i++) curr_al.push_back(TargetFWIdxs[3*i-2]);
  vector<int> el;
  if (simplify_hash.find(curr_al)==simplify_hash.end()) {
    //cerr << "SourceFWIdxs:" << SourceFWIdxs[0] << endl;
    for (int i=1; i<=SourceFWIdxs[0]; i++) { 
      //cerr << SourceFWIdxs[3*i-2] << "," <<  SourceFWIdxs[3*i-1] << "," << SourceFWIdxs[3*i] << endl;  
    }
    //cerr << "TargetFWIdxs:" << TargetFWIdxs[0] << endl;
    for (int i=1; i<=TargetFWIdxs[0]; i++) { 
      //cerr << TargetFWIdxs[3*i-2] << "," <<  TargetFWIdxs[3*i-1] << "," << TargetFWIdxs[3*i] << endl;  
    }

    vector< int* > blocks; // each element contains s1,s2,t1,t2
    int currIdx = 1; // start from 1 to avoid considering phrase start
    std::set<int> FWIdxs;
    std::vector<int> DanglingTargetFWIdxs;
    for (int i=1; i<= SourceFWIdxs[0]; i++) FWIdxs.insert(SourceFWIdxs[3*i-2]);
    for (int i=1; i<= TargetFWIdxs[0]; i++) {
      int source = sourceOf(TargetFWIdxs[3*i-2]);
      if (source>=0) {
        do {
          FWIdxs.insert(source);
          source = sourceOf(TargetFWIdxs[3*i-2],source+1);
        } while(source >=0);
      } else {
        int *block = new int[4];
        block[0]=-1; block[1]=-1; block[2]=TargetFWIdxs[3*i-2]; block[3]=TargetFWIdxs[3*i-2];
        //cerr << "5pushing dangling ["<<block[0]<<","<<block[1]<<","<< block[2] << "," << block[3] << "]" << endl;
        blocks.push_back(block);
        DanglingTargetFWIdxs.push_back(TargetFWIdxs[3*i-2]);  
      }
    }
    for (std::set<int>::const_iterator iter=FWIdxs.begin(); iter!=FWIdxs.end(); iter++) { 
      //cerr << "FWIdxs=" << *iter << endl;
    }
    std::set<int>::const_iterator currFWIdx  = FWIdxs.begin();
    if (currFWIdx == FWIdxs.end()) {
      int block[] = {1,_J-2,1,_I-2}; // no need to consider phrase boundaries
      SplitIfViolateDanglingTargetFWIdxs(&blocks,block,DanglingTargetFWIdxs);
    } else {
      int anchorIdx = *currFWIdx; // also used to denote _J+1
      do {
        // add alignments whose source from currIdx to currFWIdx-1
        while (currIdx<anchorIdx) {
          bool isMoved = false;
          //cerr << "anchorIdx = " << anchorIdx << ", currIdx = " << currIdx << endl;
          for (int idx=anchorIdx-1; idx>=currIdx; idx--) {
            int* block = blockSource(currIdx,idx);
            if (block[0]==currIdx&&block[1]==idx) {
              if (block[2]!=MINIMUM_INIT) { // must be aligned
                SplitIfViolateDanglingTargetFWIdxs(&blocks,block,DanglingTargetFWIdxs); 
                //cerr << "0pushing :" << block[0]<<","<<block[1]<<","<<block[2]<<","<<block[3]<<endl; 
              }
              currIdx = idx+1; isMoved = true;
              //delete block;
              break;
            }     
            delete block;
          }
          //cerr << "1currIdx = " << currIdx << endl;
          if (!isMoved) {
            int target = targetOf(currIdx);
            while (target>=0) {
              //cerr << "target = " << target << endl;
              int* block = new int[4];
              block[0]=currIdx; block[1]=currIdx; block[2]=target; block[3]=target;
              //cerr << "1pushing :" << block[0]<<","<<block[1]<<","<<block[2]<<","<<block[3]<<endl; 
              blocks.push_back(block);
              target = targetOf(currIdx,target+1);
              //delete block;
            }
            currIdx++;
          }
          //cerr << "2currIdx = " << currIdx << endl;
        }    
    
        //cerr << "3currIdx = " << currIdx << endl;
        // add function word alignments (anchorIdx)
        if (anchorIdx==getJ()) break;
        //cerr << "anchorIdx = "<< anchorIdx << endl;
        int target = targetOf(anchorIdx);
        //cerr << "target of anchorIdx = " << target << endl;
        do {
          int* block = new int[4];
          block[0]=anchorIdx; block[1]=anchorIdx; block[2]=target; block[3]=target;
          //cerr << "2pushing :" << block[0]<<","<<block[1]<<","<<block[2]<<","<<block[3]<<endl; 
          blocks.push_back(block);
          //delete block;
          if (target>=0) target = targetOf(anchorIdx,target+1);
        } while (target>=0);
        // advance indexes
        currIdx   = anchorIdx+1;
        anchorIdx = getJ()-1; // was minus 2
        if (++currFWIdx!=FWIdxs.end()) anchorIdx = *currFWIdx;
      } while (currIdx<=getJ()-2);
    }  

    //cerr << "simplify["<< getJ() << ","<< getI()<< "]" << endl;
    vector<int> source_block_mapper(getJ(),-1);
    vector<int> target_block_mapper(getI(),-1);
    for (int i = 0; i<blocks.size(); i++) {
      if (blocks[i][0]>=0) source_block_mapper[blocks[i][0]]=1;
      if (blocks[i][2]>=0) target_block_mapper[blocks[i][2]]=1; 
      //cerr << "block[" << i << "]" <<blocks[i][0] << "," << blocks[i][1] << "," << blocks[i][2] << "," << blocks[i][3] << " ";
    }
    //cerr << endl;
    int curr = 1;
    //cerr << "source_block_mapper " << source_block_mapper.size() << endl;
    int prev = -1;
    for (int idx=0; idx<source_block_mapper.size(); idx++) {
      if (source_block_mapper[idx]>0) {
        source_block_mapper[idx]=curr++;
        prev = curr;
      } else {
        source_block_mapper[idx]=prev;
      }
      //cerr << source_block_mapper[idx] << " ";
    }
    //cerr << endl;
    curr = 1;
    //cerr << "target_block_mapper" << endl;
    for (int idx=0; idx<target_block_mapper.size(); idx++) {
      if (target_block_mapper[idx]>0) {
        target_block_mapper[idx]=curr++;
        prev = curr;
      } else {
        target_block_mapper[idx]=prev;
      }
      //cerr << target_block_mapper[idx] << " ";
    }
    //cerr << endl;
    
    assert(blocks.size()<=24);
    //cerr << "el starts with " << el.size() << endl;
    //cerr << "result:" << endl;
    for (int i = 0; i<blocks.size(); i++) {
      if (blocks[i][2]<0 || blocks[i][0]<0) continue;
      int source = source_block_mapper[blocks[i][0]]-1;
      int target = target_block_mapper[blocks[i][2]]-1;
      el.push_back(link(source,target));
      //cerr << source << "-" << target << " ";
    }
    el.insert(el.begin(),el.size());
    //cerr << endl;
    //cerr << "the content of el after al: ";
    for (int ii=0; ii<el.size(); ii++) {
      //cerr << ii << "." << el[ii] << " ";
    }
    //cerr << endl;
    el.push_back(SourceFWIdxs[0]);
    for (int idx=1; idx<=SourceFWIdxs[0]; idx++) {
      //cerr << "SourceFWIdxs[" << (3*idx-2) << "] from " << SourceFWIdxs[3*idx-2];
      //SourceFWIdxs[3*idx-2] = source_block_mapper[SourceFWIdxs[3*idx-2]]-1;
      el.push_back(source_block_mapper[SourceFWIdxs[3*idx-2]]-1);
      //cerr << " to " << SourceFWIdxs[3*idx-2];
      //cerr << "," << SourceFWIdxs[3*idx-1] << "," << SourceFWIdxs[3*idx] << endl;
    }
    el.push_back(TargetFWIdxs[0]);
    for (int idx=1; idx<=TargetFWIdxs[0]; idx++) {
      //cerr << "TargetFWIdxs[" << (3*idx-2) << "] from " << TargetFWIdxs[3*idx-2];
      //TargetFWIdxs[3*idx-2] = target_block_mapper[TargetFWIdxs[3*idx-2]]-1;
      el.push_back(target_block_mapper[TargetFWIdxs[3*idx-2]]-1);
      //cerr << " to " << TargetFWIdxs[3*idx-2];
      //cerr << "," <<  TargetFWIdxs[3*idx-1] << "," <<  TargetFWIdxs[3*idx] << endl;
    }
    el.push_back(source_block_mapper[fas]-1); 
    el.push_back(source_block_mapper[las]-1);
    el.push_back(target_block_mapper[fat]-1);
    el.push_back(target_block_mapper[lat]-1);
    //cerr << fas << ", " << las << "< " << fat << ", " << lat << endl;
    //cerr << "insert key:el = ";
    for (int ii=0; ii<curr_al.size(); ii++) {
      //cerr << ii << "." << curr_al[ii] << " ";
    }
    for (int ii=0; ii<el.size(); ii++) {
      //cerr << ii << "." << el[ii] << " ";
    }
    //cerr << endl;
    simplify_hash.insert(pair<vector<int>, vector<int> > (curr_al,el));
  } else {
    //cout << "hit2" << endl;
    el = simplify_hash[curr_al];
  }
  //cerr << "pull key:el = ";
  for (int ii=0; ii<curr_al.size(); ii++) { 
	//cerr << ii << "." << curr_al[ii] << " ";
  }
  for (int ii=0; ii<el.size(); ii++) {
	//cerr << ii << "." << el[ii] << " ";
  }
  //cerr << endl;
  ret[12] = el[0];
  for (int i=1; i<=el[0]; i++) ret[12+i] = el[i];
  int istart = el[0]+1;
  assert(el[istart]==SourceFWIdxs[0]);
  for (int i=1; i<=el[istart]; i++) SourceFWIdxs[3*i-2]=el[istart+i];
  istart += el[istart]+1;
  assert(el[istart]==TargetFWIdxs[0]);
  for (int i=1; i<=el[istart]; i++) TargetFWIdxs[3*i-2]=el[istart+i];
  istart += el[istart]+1;
  //fillFWIdxs(ret,source_block_mapper[fas]-1,source_block_mapper[las]-1,target_block_mapper[fat]-1,target_block_mapper[lat]-1);
  fillFWIdxs(ret,el[istart],el[istart+1],el[istart+2],el[istart+3]);
  //cerr << "it all ends" << endl;  
}

void Alignment::simplify_nofw(int* ret) {
  for (int i=0; i<12; i++) ret[i]=-1;  
  ret[12]=1; ret[13]=0; 
}

void Alignment::sort(int* num) {
  if (num[0]>1) quickSort(num,1,num[0]);
}

void Alignment::quickSort(int arr[], int left, int right) {
      int i = left, j = right;
      int tmp1,tmp2,tmp3;
      int mid = (left + right) / 2;
      //cerr << "quickSort " << i << "," << mid << "," << j << endl;
      int pivot = arr[3*mid-2];
 
      /* partition */
      while (i <= j) {
            while (arr[3*i-2] < pivot)
                  i++;
            while (arr[3*j-2] > pivot)
                  j--;
            if (i <= j) {
                  tmp1 = arr[3*i-2]; tmp2 = arr[3*i-1]; tmp3 = arr[3*i];
                  arr[3*i-2] = arr[3*j-2]; arr[3*i-1] = arr[3*j-1]; arr[3*i] = arr[3*j];
                  arr[3*j-2] = tmp1; arr[3*j-1] = tmp2; arr[3*j] = tmp3;
                  i++;
                  j--;
            }
      };
 
      /* recursion */
      if (left < j)
            quickSort(arr, left, j);
      if (i < right)
            quickSort(arr, i, right);
}

void Alignment::ScoreOrientation(CountTable table, int offset, int ori, WordID cond1, WordID cond2, bool isBonus, double *cost, double *bonus, double *bo1, double *bo1_bonus, double *bo2, double *bo2_bonus) {
  //cerr << "ScoreOrientation:" << cond1 << "," << cond2 << endl;
  //cerr << "isBonus:" << isBonus << endl;
  std::string key1_part1(TD::Convert(cond1));
  std::string key1_part2(TD::Convert(cond2));
  std::string key = key1_part1 + " " + key1_part2;
  WordID key1_id = TD::Convert(key);
  double c1=0; double t1=0; double c2=0; double t2=0; double c3=0; double t3=0;
  if (table.first.find(key1_id)!=table.first.end()) {
    c1 = table.first[key1_id][offset+ori-1];
    t1 = table.first[key1_id][offset+5];
  } else {
    if (isBonus) {
      *bo1_bonus+=1;
      //cerr << "adding bo1_bonus[1], new val=" << *bo1_bonus << endl;
    } else {
      *bo1+=1;
      //cerr << "adding bo1[1], new val=" << *bo1 << endl;
    }
  }
  if (table.second.find(cond1)!=table.second.end()) {
    c2 = table.second[cond1][offset+ori-1];
    t2 = table.second[cond1][offset+5];
  } else {
    if (isBonus) {
      *bo2_bonus+=1;
      //cerr << "adding bo2_bonus[1], new val=" << *bo2_bonus << endl;
    } else {
      *bo2+=1;
      //cerr << "adding bo2[1], new val=" << *bo2 << endl;
    }
  }
  c3 = table.third[offset+ori-1]; t3 = table.third[offset+5];
  //cerr << c1 << "/" << t1 << " " << c2 << "/" << t2 << " " << c3 << "/" << t3 << endl;
  //cerr << "alpha = " << alpha1 << "," << alpha2 << endl;
  double prob = log(( c1 + alpha1 * ((c2 + alpha2 * (c3/t3))/(t2 + alpha2))) / (t1 + alpha1)); 
  if (isBonus) {
    *bonus += prob;
    //cerr << "adding bonus ["<< prob << "], new val=" << *bonus << endl;
  } else {
    *cost += prob;
    //cerr << "adding cost [" << prob << "], new val=" << *cost << endl;
  }
}

void Alignment::ScoreOrientationLeft(CountTable table, int ori, WordID cond1, WordID cond2, bool isBonus, double *cost, double *bonus, double *bo1, double *bo1_bonus, double *bo2, double *bo2_bonus) {
  //cerr << "ScoreOrientationLeft" << endl;
  ScoreOrientation(table,0,ori,cond1,cond2,isBonus,cost,bonus,bo1,bo1_bonus,bo2,bo2_bonus);
}

void Alignment::ScoreOrientationRight(CountTable table, int ori, WordID cond1, WordID cond2, bool isBonus, double *cost, double *bonus, double *bo1, double *bo1_bonus, double *bo2, double *bo2_bonus) {
  //cerr << "ScoreOrientationRight" << endl;
  ScoreOrientation(table,6,ori,cond1,cond2,isBonus,cost,bonus,bo1,bo1_bonus,bo2,bo2_bonus);
}


void Alignment::computeOrientationSource(CountTable table, double *cost, double *bonus, double *bo1, double *bo1_bonus, double *bo2, double *bo2_bonus) {
  //cerr << "computeOrientationSource" << endl;
  int oril, orir;
  for (int idx=1; idx<=SourceFWRuleIdxs[0]; idx++) {
    //cerr << "considering SourceFWRuleIdxs[" << idx << "]: " << SourceFWRuleIdxs[3*idx-2] << endl; 
    OrientationSource(SourceFWRuleIdxs[3*idx-2],&oril,&orir);
    bool isBonus = false;
    if (SourceFWRuleIdxs[3*idx-2]<=fas) isBonus=true;
    if (!isBonus)
      if (minTSpan(SourceFWRuleIdxs[3*idx-2])==MINIMUM_INIT && las<=SourceFWRuleIdxs[3*idx-2]) isBonus=true;
        ScoreOrientationLeft(table,oril,SourceFWRuleIdxs[3*idx],SourceFWRuleIdxs[3*idx-1],
                             isBonus,cost,bonus,bo1,bo1_bonus,bo2,bo2_bonus); 
    isBonus = false;
    if (las<=SourceFWRuleIdxs[3*idx-2]) isBonus=true;
    if (!isBonus)
      if (minTSpan(SourceFWRuleIdxs[3*idx-2])==MINIMUM_INIT && SourceFWRuleIdxs[3*idx-2]<=fas) isBonus=true;
        ScoreOrientationRight(table,orir,SourceFWRuleIdxs[3*idx],SourceFWRuleIdxs[3*idx-1],
                             isBonus,cost,bonus,bo1,bo1_bonus,bo2,bo2_bonus);
  }
  for (int i_ant=0; i_ant<_Arity; i_ant++) {
    for (int idx=1; idx<=SourceFWAntsIdxs[i_ant][0]; idx++) {
      //cerr << "considering SourceFWAntsIdxs[" << i_ant << "][" << idx << "]: " << SourceFWAntsIdxs[i_ant][3*idx-2] << endl;
      int antfas = firstSourceAligned(SourceAntsIdxs[i_ant][1]);
      int antlas = lastSourceAligned(SourceAntsIdxs[i_ant][SourceAntsIdxs[i_ant][0]]);
      bool aligned = (minTSpan(SourceFWAntsIdxs[i_ant][3*idx-2])!=MINIMUM_INIT);
      //cerr << "idx=" << SourceFWAntsIdxs[i_ant][3*idx-2] << ", antfas=" << antfas << ", antlas=" << antlas << ", aligned=" << aligned << endl;
      bool Lcompute = true;bool Rcompute = true;
      if ((aligned && antfas<SourceFWAntsIdxs[i_ant][3*idx-2]) ||
          (!aligned && antfas < SourceFWAntsIdxs[i_ant][3*idx-2] && SourceFWAntsIdxs[i_ant][3*idx-2] < antlas)) 
          Lcompute=false; 
      if ((aligned && SourceFWAntsIdxs[i_ant][3*idx-2]<antlas) ||
          (!aligned && antfas < SourceFWAntsIdxs[i_ant][3*idx-2] && SourceFWAntsIdxs[i_ant][3*idx-2] < antlas)) 
          Rcompute=false;
      //cerr << "Lcompute=" << Lcompute << ",Rcompute=" << Rcompute << endl;
      if (!Lcompute && !Rcompute) continue;
      OrientationSource(SourceFWAntsIdxs[i_ant][3*idx-2],&oril,&orir,Lcompute, Rcompute);
      bool isBonus = false;
      if (Lcompute) {
        if (SourceFWAntsIdxs[i_ant][3*idx-2]<=fas) isBonus = true;
        if (!isBonus) 
          if (!aligned && las<=SourceFWAntsIdxs[i_ant][3*idx-2]) isBonus=true;
          ScoreOrientationLeft(table,oril,SourceFWAntsIdxs[i_ant][3*idx],SourceFWAntsIdxs[i_ant][3*idx-1],
                               isBonus,cost,bonus,bo1,bo1_bonus,bo2,bo2_bonus); 
      } else {
        //cerr << "Skipping Left" << endl;
      }
      isBonus = false;
      if (Rcompute) {
        if (las<=SourceFWAntsIdxs[i_ant][3*idx-2]) isBonus = true;
        if (!isBonus)
          if (!aligned && SourceFWAntsIdxs[i_ant][3*idx-2]<=fas) isBonus=true;
            ScoreOrientationRight(table,orir,SourceFWAntsIdxs[i_ant][3*idx],SourceFWAntsIdxs[i_ant][3*idx-1],
                                  isBonus,cost,bonus,bo1,bo1_bonus,bo2,bo2_bonus);
      } else {
        //cerr << "Skipping Right" << endl;
      }
    }
  }
  //cerr << "END of computeOrientationSource" << endl;  
}

void Alignment::computeOrientationTarget(CountTable table, double *cost, double *bonus, double *bo1, double *bo1_bonus, double *bo2, double *bo2_bonus) {
  //cerr << "computeOrientationTarget" << endl;
  int oril, orir;
  for (int idx=1; idx<=TargetFWRuleIdxs[0]; idx++) {
    //cerr << "considering TargetFWRuleIdxs[" << idx << "]: " << TargetFWRuleIdxs[3*idx-2] << endl;
    OrientationTarget(TargetFWRuleIdxs[3*idx-2],&oril,&orir);
    // the second and the third parameters of ScoreOrientationLeft must be e and f (not f and then e) 
    bool isBonus = false;
    if (TargetFWRuleIdxs[3*idx-2]<=fat) isBonus = true;
    if (!isBonus)
      if (minSSpan(TargetFWRuleIdxs[3*idx-2])==MINIMUM_INIT && lat<=TargetFWRuleIdxs[3*idx-2]) isBonus = true;
    //cerr << "fat=" << fat << ", lat=" << lat << ", idx=" << TargetFWRuleIdxs[3*idx-2] << endl;
    ScoreOrientationLeft(table,oril,TargetFWRuleIdxs[3*idx],TargetFWRuleIdxs[3*idx-1],
                         isBonus,cost,bonus,bo1,bo1_bonus,bo2,bo2_bonus); 
    isBonus = false;
    if (lat<=TargetFWRuleIdxs[3*idx-2]) isBonus = true;
    if (!isBonus)
      if (minSSpan(TargetFWRuleIdxs[3*idx-2])==MINIMUM_INIT && TargetFWRuleIdxs[3*idx-2]<=fat) isBonus=true;
    //cerr << "fat=" << fat << ", lat=" << lat << ", idx=" << TargetFWRuleIdxs[3*idx-2] << endl;
    ScoreOrientationRight(table,orir,TargetFWRuleIdxs[3*idx],TargetFWRuleIdxs[3*idx-1],
                          isBonus,cost,bonus,bo1,bo1_bonus,bo2,bo2_bonus);
  }

  for (int i_ant=0; i_ant<_Arity; i_ant++) {
    for (int idx=1; idx<=TargetFWAntsIdxs[i_ant][0]; idx++) {
      //cerr << "considering TargetFWAntsIdxs[" << i_ant << "][" << idx << "]: " << TargetFWAntsIdxs[i_ant][3*idx-2] << endl; 
      int antfat = firstTargetAligned(TargetAntsIdxs[i_ant][1]);
      int antlat = lastTargetAligned(TargetAntsIdxs[i_ant][TargetAntsIdxs[i_ant][0]]);
      int aligned = (minSSpan( TargetFWAntsIdxs[i_ant][3*idx-2])!=MINIMUM_INIT);
      //cerr << "idx=" <<  TargetFWAntsIdxs[i_ant][3*idx-2] << ", antfat=" << antfat << ", antlat=" <<antlat <<", aligned="<<aligned<<endl;
      bool Lcompute = true; bool Rcompute = true;
      if ((aligned && antfat<TargetFWAntsIdxs[i_ant][3*idx-2]) ||
          (!aligned && antfat < TargetFWAntsIdxs[i_ant][3*idx-2] && TargetFWAntsIdxs[i_ant][3*idx-2] < antlat)) 
          Lcompute=false;
      if ((aligned && TargetFWAntsIdxs[i_ant][3*idx-2]<antlat) ||
          (!aligned && antfat < TargetFWAntsIdxs[i_ant][3*idx-2] && TargetFWAntsIdxs[i_ant][3*idx-2] < antlat)) 
          Rcompute=false;
      //cerr << "Lcompute=" << Lcompute << ",Rcompute=" << Rcompute << endl;
      if (!Lcompute && !Rcompute) continue;
      bool isBonus = false;
      OrientationTarget(TargetFWAntsIdxs[i_ant][3*idx-2],&oril,&orir, Lcompute, Rcompute);
      if (Lcompute) {
        if (TargetFWAntsIdxs[i_ant][3*idx-2]<=fat) isBonus=true;
        if (!isBonus) 
          if (!aligned && lat<=TargetFWAntsIdxs[i_ant][3*idx-2]) isBonus=true;
          ScoreOrientationLeft(table,oril,TargetFWAntsIdxs[i_ant][3*idx],TargetFWAntsIdxs[i_ant][3*idx-1],
                               isBonus,cost,bonus,bo1,bo1_bonus,bo2,bo2_bonus); 
      }
      isBonus = false;
      if (Rcompute) {
        if (lat<=TargetFWAntsIdxs[i_ant][3*idx-2]) isBonus=true;
        if (!isBonus)
          if (!aligned && TargetFWAntsIdxs[i_ant][3*idx-2]<=fat) isBonus=true;
          ScoreOrientationRight(table,orir,TargetFWAntsIdxs[i_ant][3*idx],TargetFWAntsIdxs[i_ant][3*idx-1],
                                isBonus,cost,bonus,bo1,bo1_bonus,bo2,bo2_bonus);
      }
    }
  }
  //cerr << "END of computeOrientationTarget" << endl;  
}

bool Alignment::MemberOf(int* FWIdxs, int pos1, int pos2) {
  /*if (FWIdxs[0]<=1) return false;
  if (pos2<=FWIdxs[1]) return false;
  if (FWIdxs[3*FWIdxs[0]-2]<=pos1) return false;*/
  for (int idx=2; idx<=FWIdxs[0]; idx++) 
    if (FWIdxs[3*(idx-1)-2]==pos1 && FWIdxs[3*idx-2]==pos2) return true;
  return false;
}

void Alignment::computeDominanceSource(CountTable table, double *cost, double *bonus, double *bo1, double *bo1_bonus, double *bo2, double *bo2_bonus) {
  // no bonus yet
  //cerr << "computeDominanceSource" << endl;
  //cerr << "cost="  << *cost << ", bo1=" << *bo1 << ", bo2=" << *bo2 << endl;
  for (int idx=2; idx<=SourceFWIdxs[0]; idx++) {
    //cerr << "PrevSourceFWIdxs :" << SourceFWIdxs[3*(idx-1)-2] << "," << SourceFWIdxs[3*(idx-1)-1] << "," << SourceFWIdxs[3*(idx-1)] << endl;
    //cerr << "CurrSourceFWIdxs :" << SourceFWIdxs[3*(idx)-2] << "," << SourceFWIdxs[3*(idx)-1] << "," << SourceFWIdxs[3*(idx)] << endl;
    bool compute = true;
    for (int i_ant=0; i_ant<_Arity && compute; i_ant++) {
      if (MemberOf(SourceFWAntsIdxs[i_ant],SourceFWIdxs[3*(idx-1)-2],SourceFWIdxs[3*(idx)-2])) {
        //cerr << "Skipping, they have been calculated in the  " << (i_ant+1) << "-th branch" << endl;
        compute=false;
      }
    }
    if (compute) {
      int dom = DominanceSource(SourceFWIdxs[3*(idx-1)-2],SourceFWIdxs[3*idx-2]);
      //cerr << "dom = " << dom << endl;
      ScoreDominance(table,dom,SourceFWIdxs[3*(idx-1)-1],SourceFWIdxs[3*idx-1],SourceFWIdxs[3*(idx-1)],SourceFWIdxs[3*idx],
              cost,bonus,bo1,bo1_bonus,bo2,bo2_bonus);
    }
  }
  //cerr << "END of computeDominanceSource" << endl;
}

void Alignment::computeDominanceTarget(CountTable table, double *cost, double *bonus, double *bo1, double *bo1_bonus, double *bo2, double *bo2_bonus) {
   //cerr << "computeDominanceTarget" << endl;
  for (int idx=2; idx<=TargetFWIdxs[0]; idx++) {
    //cerr << "PrevTargetFWIdxs :" << TargetFWIdxs[3*(idx-1)-2] << "," << TargetFWIdxs[3*(idx-1)-1] << "," <<TargetFWIdxs[3*(idx-1)] << endl;
    //cerr << "CurrTargetFWIdxs :" << TargetFWIdxs[3*(idx)-2] << "," << TargetFWIdxs[3*(idx)-1] << "," <<TargetFWIdxs[3*(idx)] << endl;
    bool compute = true;
    for (int i_ant=0; i_ant <_Arity && compute; i_ant++) {
      if (MemberOf(TargetFWAntsIdxs[i_ant],TargetFWIdxs[3*(idx-1)-2],TargetFWIdxs[3*idx-2])) {
        //cerr << "Skipping, they have been calculated in the " << (i_ant+1) << "-th branch" << endl;
        compute = false;
      }
    }
    if (compute) {
      int dom = DominanceTarget(TargetFWIdxs[3*(idx-1)-2],TargetFWIdxs[3*idx-2]);
      //cerr << (3*(idx-1)) << "," << (3*idx) << "," << (3*(idx-1)-1) << "," << (3*idx-1) << endl;
      //cerr << "dom target = " << dom << endl;
      ScoreDominance(table,dom,TargetFWIdxs[3*(idx-1)],TargetFWIdxs[3*idx],TargetFWIdxs[3*(idx-1)-1],TargetFWIdxs[3*idx-1],
              cost,bonus,bo1,bo1_bonus,bo2,bo2_bonus);
    }
  }
  //cerr << "END of computeDominanceTarget" << endl;
}

void Alignment::ScoreDominance(CountTable table, int dom, WordID source1, WordID source2, WordID target1, WordID target2, double *cost, double *bonus, double *bo1, double *bo1_bonus, double *bo2, double *bo2_bonus) {
  //cerr << "ScoreDominance(source1=" << source1 << ",source2=" << source2 << ",target1=" << target1 << ",target2=" << target2 <<endl;
  //cerr << "cost=" << *cost << ", bo1=" << *bo1 << ", bo2=" << *bo2 << endl;
  string key1_part1(TD::Convert(source1));
  string key1_part2(TD::Convert(source2));
  string key = key1_part1 + " " + key1_part2;
  WordID key2_id = TD::Convert(key);
  string key1_part3(TD::Convert(target1));
  string key1_part4(TD::Convert(target2));
  key += " "+key1_part3+" "+key1_part4;
  WordID key1_id = TD::Convert(key);
  //cerr << "ScoreDominance(key1=" << TD::Convert(key1_id) << ", key2=" << TD::Convert(key2_id) << endl;
        double c1=0; 
  double t1=0; 
  double c2=0; 
  double t2=0; 
  double c3=0; 
  double t3=0;
        if (table.first.find(key1_id)!=table.first.end()) {
                c1 = table.first[key1_id][dom];
                t1 = table.first[key1_id][4];
        } else {
                *bo1+=1;
    //cerr << "adding bo1[1], new val=" << *bo1 << endl;
        }
        if (table.second.find(key2_id)!=table.second.end()) {
                c2 = table.second[key2_id][dom];
                t2 = table.second[key2_id][4];
        } else {
                *bo2+=1;
    //cerr << "adding bo2[1], new val=" << *bo2 << endl;
        }
        c3 = table.third[dom]; t3 = table.third[4];
  //cerr << "ScoreDominance(" << c1 <<","<< t1 << " " << c2 << "," << t2 << " " << c3 << "," << t3 << ")" << endl;
        double prob = log(( c1 + alpha3 * ((c2 + alpha4 * (c3/t3))/(t2 + alpha4))) / (t1 + alpha3));
  *cost += prob;
  //cerr << "adding cost [" << prob << "], new val=" << *cost << endl;
}

WordID Alignment::F2EProjection(int idx, string delimiter) {
  //cerr << "F2EProjection(" << idx << ")" << endl;
  ostringstream projection(ostringstream::out);
  int e = targetOf(idx);
  if (e<0) {
    //cerr << "projection = NULL" << endl;
    return TD::Convert("NULL");
  } else {
    bool firstTime = true;
    do {
      if (!firstTime) projection << delimiter; 
      projection << TD::Convert(_e[e-1]); // transform space
      firstTime = false;
      e = targetOf(idx,e+1);
      //cerr << "projection = " << projection.str() << endl;
    } while(e>=0);
    return TD::Convert(projection.str());
  }
}

WordID Alignment::E2FProjection(int idx, string delimiter) {
  //cerr << "E2FProjection(" << idx << ")" << endl;
        ostringstream projection(ostringstream::out);
  //cerr << "i" << endl;
  int f = sourceOf(idx);
  //cerr << "j, f=" << f << endl;
  if (f<0) {
    //cerr << "projection = NULL" << endl;
    return TD::Convert("NULL");
  } else {
    bool firstTime = true;
    do {
      if (!firstTime) projection << delimiter;
      projection << TD::Convert(_f[f-1]); //transform space
      firstTime = false;
      f = sourceOf(idx,f+1);
      //cerr << "projection = " << projection.str() << endl;
    } while(f>=0);
    return TD::Convert(projection.str());
  }
}

bool Alignment::prepare(TRule& rule, const std::vector<const void*>& ant_contexts, map<WordID,int> sfw, map<WordID,int> tfw) {  
  //cerr << "===Rule===" << rule.AsString() << endl;
  _f = rule.f();
  //cerr << "F: ";
  for (int idx=0; idx<_f.size(); idx++) {
    //cerr << _f[idx] << " ";
  }
  //cerr << endl;
  //cerr << "F': ";
  for (int idx=0; idx<_f.size(); idx++) 
    if (_f[idx]>=0) {
      //cerr << TD::Convert(_f[idx]) << " "; 
    } else {
      //cerr << TD::Convert(_f[idx]*-1);
    }
  //cerr << endl;
  //cerr << "E: ";
  _e = rule.e();
  for (int idx=0; idx<_e.size(); idx++) {
    //cerr << _e[idx] << " ";
  }
  //cerr << endl;
  //cerr << "E': ";

  for (int idx=0; idx<_e.size(); idx++) 
    if (_e[idx]>0) {
      //cerr << TD::Convert(_e[idx]) << " "; 
    } else {
      //cerr << "[NT]" << " ";
    }
  //cerr << endl;
  _Arity = rule.Arity();

  SourceFWRuleIdxs[0]=0;
  for (int idx=1; idx<=_f.size(); idx++) { // in transformed space
    if (sfw[_f[idx-1]]==1) {
      SourceFWRuleIdxs[0]++;
      SourceFWRuleIdxs[3*SourceFWRuleIdxs[0]-2]=idx;
      SourceFWRuleIdxs[3*SourceFWRuleIdxs[0]-1]=_f[idx-1];
      //SourceFWRuleIdxs[3*SourceFWRuleIdxs[0]]  =F2EProjection(idx); // wrong place, als aren't yet set
    }
  }
  //cerr << "SourceFWRuleIdxs[" << SourceFWRuleIdxs[0] << "]:";
  for (int idx=1; idx<=SourceFWRuleIdxs[0]; idx++) {
    //cerr << " idx:" << SourceFWRuleIdxs[3*idx-2];
    //cerr << " F:" << SourceFWRuleIdxs[3*idx-1];
    ////cerr << " E:" << SourceFWRuleIdxs[3*idx];
    //cerr << "; ";
  }
  //cerr << endl;
  TargetFWRuleIdxs[0]=0;
  for (int idx=1; idx<=_e.size(); idx++) { // in transformed space
    if (tfw[_e[idx-1]]==1) {
      TargetFWRuleIdxs[0]++;
      TargetFWRuleIdxs[3*TargetFWRuleIdxs[0]-2]=idx;
      //TargetFWRuleIdxs[3*TargetFWRuleIdxs[0]-1]=E2FProjection(idx); // wrong place, als aren't yet set
      TargetFWRuleIdxs[3*TargetFWRuleIdxs[0]]  =_e[idx-1];
    }
  }
  //cerr << "TargetFWRuleIdxs[" << TargetFWRuleIdxs[0] << "]:";
  for (int idx=1; idx<=TargetFWRuleIdxs[0]; idx++) {
    //cerr << " idx:" << TargetFWRuleIdxs[3*idx-2];
    ////cerr << " F:" << TargetFWRuleIdxs[3*idx-1];
    //cerr << " E:" << TargetFWRuleIdxs[3*idx];
  }
  //cerr << endl;
  if (SourceFWRuleIdxs[0]+TargetFWRuleIdxs[0]==0) {
    bool nofw = true;
    for (int i_ant=0; i_ant<_Arity && nofw; i_ant++) { 
      const int* ants = reinterpret_cast<const int *>(ant_contexts[i_ant]);
      if (ants[0]>=0||ants[3]>=0||ants[6]>=0||ants[9]>=0) nofw=false;
    } 
    if (nofw) return true;
  }
  //cerr << "clearing als first" << endl;
  clearAls(_J,_I);

  //cerr << "A["<< rule.a_.size() << "]: " ;
  RuleAl[0]=0;
  // add phrase start boundary
  RuleAl[0]++; RuleAl[RuleAl[0]*2-1]=0; RuleAl[RuleAl[0]*2]=0;
  //cerr << RuleAl[RuleAl[0]*2-1] << "-" << RuleAl[RuleAl[0]*2] << " ";
  for (int idx=0; idx<rule.a_.size(); idx++) {
    RuleAl[0]++;
    RuleAl[RuleAl[0]*2-1]=source(rule.a_[idx])+1;
    RuleAl[RuleAl[0]*2]  =target(rule.a_[idx])+1;
    //cerr << RuleAl[RuleAl[0]*2-1] << "-" << RuleAl[RuleAl[0]*2] << " ";
  }
  // add phrase end boundary
  RuleAl[0]++; RuleAl[RuleAl[0]*2-1]=_f.size()+1; RuleAl[RuleAl[0]*2]=_e.size()+1;
  //cerr << RuleAl[RuleAl[0]*2-1] << "-" << RuleAl[RuleAl[0]*2] << " ";
  //cerr << endl;

  SourceRuleIdxs[0] = _f.size()+2; // +2 (phrase boundaries)
  TargetRuleIdxs[0] = _e.size()+2; 
  int ntidx=-1;
  for (int idx=0; idx<_f.size()+2; idx++) { // idx in transformed space
    SourceRuleIdxs[idx+1]=idx;
    if (0<idx && idx<=_f.size()) if (_f[idx-1]<0) SourceRuleIdxs[idx+1]=ntidx--;
  }
  for (int idx=0; idx<_e.size()+2; idx++) {
    TargetRuleIdxs[idx+1]=idx;
    if (0<idx && idx<=_e.size()) {
      //cerr << "_e[" <<(idx-1)<< "]=" << _e[idx-1] << endl;
      if (_e[idx-1]<=0) TargetRuleIdxs[idx+1]=_e[idx-1]-1;
    }
  }
  //cerr << "SourceRuleIdxs:";
  for (int idx=0; idx<SourceRuleIdxs[0]+1; idx++) {
    //cerr << " " << SourceRuleIdxs[idx];
  }
  //cerr << endl;
  //cerr << "TargetRuleIdxs:";
  for (int idx=0; idx<TargetRuleIdxs[0]+1; idx++) {
    //cerr << " " << TargetRuleIdxs[idx];
  }
  //cerr << endl;

  // sloppy, the integrity of anstates is assumed
  // total = 37 bytes
  // first 3 ints for leftmost source function words (1 for index, 4 for source WordID and 4 for target WordI
  // second 3 for rightmost source function words
  // third 3 for leftmost target function words
  // fourth 3 for rightmost target function words
  // the next 1 int for the number of alignments
  // the remaining 24 ints for alignments (source then target)
  for (int i_ant=0; i_ant<_Arity; i_ant++) {
    //cerr << "antcontexts[" << i_ant << "] ";
    const int* ants = reinterpret_cast<const int *>(ant_contexts[i_ant]);
    for (int idx=0; idx<37; idx++) //cerr << idx << "." << ants[idx] << " ";
    //cerr << endl;
    SourceFWAntsIdxs[i_ant][0]=0;
    if (ants[0]>=0) {
      if (ants[1]>=0) { // one function word
        SourceFWAntsIdxs[i_ant][0]++; SourceFWAntsIdxs[i_ant][1]=ants[0];
        SourceFWAntsIdxs[i_ant][2]=ants[1]; SourceFWAntsIdxs[i_ant][3]=ants[2]; 
      } else { // if ants[1] < 0 then compound fws
        //cerr << "ants[1]<0" << endl;
        istringstream ossf(TD::Convert(ants[1]*-1)); string ffw;
        istringstream osse(TD::Convert(ants[2])); string efw; //projection would be mostly NULL
        int delta=ants[0];
        while (osse >> efw && ossf >> ffw) {
          SourceFWAntsIdxs[i_ant][0]++; 
          SourceFWAntsIdxs[i_ant][SourceFWAntsIdxs[i_ant][0]*3-2]=ants[0]-(delta--);
          SourceFWAntsIdxs[i_ant][SourceFWAntsIdxs[i_ant][0]*3-1]=TD::Convert(ffw);  
          SourceFWAntsIdxs[i_ant][SourceFWAntsIdxs[i_ant][0]*3]  =TD::Convert(efw);
        }
      }
    }
    if (ants[3]>=0) {
      if (ants[4]>=0) {
        SourceFWAntsIdxs[i_ant][0]++; 
        SourceFWAntsIdxs[i_ant][SourceFWAntsIdxs[i_ant][0]*3-2]=ants[3]; 
        SourceFWAntsIdxs[i_ant][SourceFWAntsIdxs[i_ant][0]*3-1]=ants[4]; 
        SourceFWAntsIdxs[i_ant][SourceFWAntsIdxs[i_ant][0]*3]  =ants[5];
      } else { // if ants[4] < 0 then compound fws
        //cerr << "ants[4]<0" << endl;
        istringstream ossf(TD::Convert(ants[4]*-1)); string ffw;
        istringstream osse(TD::Convert(ants[5]));    string efw;
        int delta=0;
        while (osse >> efw && ossf >> ffw) {
          SourceFWAntsIdxs[i_ant][0]++;
          SourceFWAntsIdxs[i_ant][SourceFWAntsIdxs[i_ant][0]*3-2]=ants[3]+(delta++);
          SourceFWAntsIdxs[i_ant][SourceFWAntsIdxs[i_ant][0]*3-1]=TD::Convert(ffw);
          SourceFWAntsIdxs[i_ant][SourceFWAntsIdxs[i_ant][0]*3]  =TD::Convert(efw);
        }
      }
    }
    TargetFWAntsIdxs[i_ant][0]=0;
    if (ants[6]>=0) {
      if (ants[8]>=0) { // check the e part 
        TargetFWAntsIdxs[i_ant][0]++; 
        TargetFWAntsIdxs[i_ant][1]=ants[6];
        TargetFWAntsIdxs[i_ant][2]=ants[7]; 
        TargetFWAntsIdxs[i_ant][3]=ants[8];
      } else { // if ants[8] < 0 then compound fws
        //cerr << "ants[8]<0" << endl;
        //cerr << "ants[7]=" << TD::Convert(ants[7]) << endl;
        //cerr << "ants[8]=" << TD::Convert(ants[8]*-1) << endl;
        istringstream ossf(TD::Convert(ants[7]));    string ffw;
        istringstream osse(TD::Convert(ants[8]*-1)); string efw;
        int delta=ants[6];
        while (osse >> efw && ossf >> ffw) {
          //cerr << "efw="<< efw << ",ffw=" << ffw << endl;
          TargetFWAntsIdxs[i_ant][0]++;
          TargetFWAntsIdxs[i_ant][TargetFWAntsIdxs[i_ant][0]*3-2]=ants[6]-(delta--);
          TargetFWAntsIdxs[i_ant][TargetFWAntsIdxs[i_ant][0]*3-1]=TD::Convert(ffw);
          TargetFWAntsIdxs[i_ant][TargetFWAntsIdxs[i_ant][0]*3]  =TD::Convert(efw);
        }
      }
    }
    if (ants[9]>=0) {
      if (ants[11]>=0) {
        TargetFWAntsIdxs[i_ant][0]++; 
        TargetFWAntsIdxs[i_ant][TargetFWAntsIdxs[i_ant][0]*3-2]=ants[9];
        TargetFWAntsIdxs[i_ant][TargetFWAntsIdxs[i_ant][0]*3-1]=ants[10];
        TargetFWAntsIdxs[i_ant][TargetFWAntsIdxs[i_ant][0]*3]  =ants[11];
      } else {
        //cerr << "ants[11]<0" << endl;
        //cerr << "ants[10]=" << TD::Convert(ants[10]) << endl;
        //cerr << "ants[11]=" << TD::Convert(ants[11]*-1) << endl;
        istringstream ossf(TD::Convert(ants[10]));    string ffw;
        istringstream osse(TD::Convert(ants[11]*-1)); string efw;
        int delta = 0;
        while (osse >> efw && ossf >> ffw) {
          //cerr << "efw="<< efw << ",ffw=" << ffw << endl;
          TargetFWAntsIdxs[i_ant][0]++;
          TargetFWAntsIdxs[i_ant][TargetFWAntsIdxs[i_ant][0]*3-2]=ants[9]+(delta++);
          TargetFWAntsIdxs[i_ant][TargetFWAntsIdxs[i_ant][0]*3-1]=TD::Convert(ffw);
          TargetFWAntsIdxs[i_ant][TargetFWAntsIdxs[i_ant][0]*3]  =TD::Convert(efw);
        }
      }
    }
    AntsAl[i_ant][0]=ants[12];//number of alignments
    for (int idx=1; idx<=AntsAl[i_ant][0]; idx++) {
      AntsAl[i_ant][idx*2-1] = source(ants[12+idx]);
      AntsAl[i_ant][idx*2]   = target(ants[12+idx]);
    }
  }

  for (int i_ant=0; i_ant<_Arity; i_ant++) {
    int length = AntsAl[i_ant][0];
    int maxs = -1000;
    int maxt = -1000;
    for (int idx=0; idx<length; idx++) {
      if (maxs<AntsAl[i_ant][2*idx+1]) maxs = AntsAl[i_ant][2*idx+1];
      if (maxt<AntsAl[i_ant][2*idx+2]) maxt = AntsAl[i_ant][2*idx+2];
    }
    //cerr << "SourceFWAntsIdxs[" <<i_ant<<"][0]=" << SourceFWAntsIdxs[i_ant][0] << endl;
    for (int idx=1; idx<=SourceFWAntsIdxs[i_ant][0]; idx++) {
      //cerr << "SourceFWAntsIdxs["<<i_ant<<"]["<<(3*idx-2)<<"]="<<SourceFWAntsIdxs[i_ant][3*idx-2];
      //cerr << ","<<SourceFWAntsIdxs[i_ant][3*idx-1]<<","<<SourceFWAntsIdxs[i_ant][3*idx]<<endl;
      if (maxs<SourceFWAntsIdxs[i_ant][3*idx-2]) maxs=SourceFWAntsIdxs[i_ant][3*idx-2];
    }
    //cerr << "TargetFWAntsIdxs[" <<i_ant<<"][0]=" << TargetFWAntsIdxs[i_ant][0] << endl;
    for (int idx=1; idx<=TargetFWAntsIdxs[i_ant][0]; idx++) {
      //cerr << "TargetFWAntsIdxs["<<i_ant<<"]["<<(3*idx-2)<<"]="<<TargetFWAntsIdxs[i_ant][3*idx-2];
      //cerr << ","<<TargetFWAntsIdxs[i_ant][3*idx-1]<<","<<TargetFWAntsIdxs[i_ant][3*idx]<<endl;
      if (maxt<TargetFWAntsIdxs[i_ant][3*idx-2]) maxt=TargetFWAntsIdxs[i_ant][3*idx-2];
    }
    SourceAntsIdxs[i_ant][0] = maxs+1;
    //cerr << "SourceAntsIdxs[" << i_ant << "][0]=" <<SourceAntsIdxs[i_ant][0] << endl;
    for (int idx=0; idx<SourceAntsIdxs[i_ant][0]; idx++) SourceAntsIdxs[i_ant][idx+1]=idx;
    TargetAntsIdxs[i_ant][0] = maxt+1;
    //cerr << "TargetAntsIdxs[" << i_ant << "][0]=" <<TargetAntsIdxs[i_ant][0] << endl;
    for (int idx=0; idx<TargetAntsIdxs[i_ant][0]; idx++) TargetAntsIdxs[i_ant][idx+1]=idx;
  }
  TotalSource = SourceRuleIdxs[0] - _Arity;
  for (int idx=0; idx<_Arity; idx++) TotalSource += SourceAntsIdxs[idx][0];
  TotalTarget = TargetRuleIdxs[0] - _Arity;
  for (int idx=0; idx<_Arity; idx++) TotalTarget += TargetAntsIdxs[idx][0];
  //cerr << "TotalSource = "<< TotalSource << endl;
  //cerr << "TotalTarget = "<< TotalTarget << endl;
  int curr = 0;
  for (int idx=1; idx<=SourceRuleIdxs[0]; idx++) {
    if (SourceRuleIdxs[idx]>=0) {
      SourceRuleIdxs[idx]=curr++;
    } else {
      int i_ant = SourceRuleIdxs[idx]*-1-1;
      //cerr << "SourceAntsIdxs[" << i_ant << "]" << endl;
      for (int idx2=1; idx2<=SourceAntsIdxs[i_ant][0]; idx2++) {
        SourceAntsIdxs[i_ant][idx2]=curr++;
        //cerr << SourceAntsIdxs[i_ant][idx2] << " ";
      }
      //cerr << endl;
    }
  }
  //cerr << "SourceRuleIdxs" << endl;
  for (int idx=1; idx<=SourceRuleIdxs[0]; idx++) //cerr << SourceRuleIdxs[idx] << " ";
  //cerr << endl;
  curr = 0;
  for (int idx=1; idx<=TargetRuleIdxs[0]; idx++) {
    if (TargetRuleIdxs[idx]>=0) {
      TargetRuleIdxs[idx]=curr++;
    } else {
      int i_ant = TargetRuleIdxs[idx]*-1-1;
      //cerr << "TargetRuleIdxs[" << i_ant << "]" << endl;
      for (int idx2=1; idx2<=TargetAntsIdxs[i_ant][0]; idx2++) {
        TargetAntsIdxs[i_ant][idx2]=curr++;
        //cerr << TargetAntsIdxs[i_ant][idx2] << " ";
      }
      //cerr << endl;
    }
  }
  //cerr << "TargetRuleIdxs" << endl;
  for (int idx=1; idx<=TargetRuleIdxs[0]; idx++) //cerr << TargetRuleIdxs[idx] << " ";
  //cerr << endl;
  for (int idx=1; idx<=RuleAl[0]; idx++) {
    //cerr << RuleAl[idx*2-1] << " - " << RuleAl[idx*2] << " to ";
    //cerr << SourceRuleIdxs[RuleAl[idx*2-1]+1] << " - " << TargetRuleIdxs[RuleAl[idx*2]+1] << endl;
    set(SourceRuleIdxs[RuleAl[idx*2-1]+1],TargetRuleIdxs[RuleAl[idx*2]+1]);
  }
  for (int i_ant=0; i_ant<_Arity; i_ant++) {
    for (int idx=1; idx<=AntsAl[i_ant][0]; idx++) {
      //cerr << AntsAl[i_ant][2*idx-1] << " - " << AntsAl[i_ant][2*idx] << " to ";
      //cerr << SourceAntsIdxs[i_ant][AntsAl[i_ant][2*idx-1]+1] << " - ";
      //cerr << TargetAntsIdxs[i_ant][AntsAl[i_ant][2*idx]+1] << endl;
      set(SourceAntsIdxs[i_ant][AntsAl[i_ant][2*idx-1]+1],TargetAntsIdxs[i_ant][AntsAl[i_ant][2*idx]+1]);
    }
  }
  SourceFWIdxs[0]=0;
  //cerr << "SourceFWRuleIdxs:" << endl;
  for (int idx=1; idx<=SourceFWRuleIdxs[0]; idx++) {
    //cerr << SourceFWRuleIdxs[3*idx-2] << " to " << SourceRuleIdxs[SourceFWRuleIdxs[3*idx-2]+1] << endl;
    SourceFWRuleIdxs[3*idx-2] = SourceRuleIdxs[SourceFWRuleIdxs[3*idx-2]+1];
    SourceFWIdxs[0]++;
    SourceFWIdxs[3*SourceFWIdxs[0]-2]=SourceFWRuleIdxs[3*idx-2];
    SourceFWIdxs[3*SourceFWIdxs[0]-1]=SourceFWRuleIdxs[3*idx-1];
    SourceFWIdxs[3*SourceFWIdxs[0]]  =F2EProjection(SourceFWRuleIdxs[3*idx-2],"_SEP_");
    SourceFWRuleIdxs[3*idx] = SourceFWIdxs[3*SourceFWIdxs[0]];
  }
  for (int i_ant=0; i_ant<_Arity; i_ant++) {
    //cerr << "SourceFWAntsIdxs[" << i_ant << "]" << endl;
    for (int idx=1; idx<=SourceFWAntsIdxs[i_ant][0]; idx++) {
      //cerr << SourceFWAntsIdxs[i_ant][3*idx-2] << " to " << SourceAntsIdxs[i_ant][SourceFWAntsIdxs[i_ant][3*idx-2]+1] << endl;
      SourceFWAntsIdxs[i_ant][3*idx-2] = SourceAntsIdxs[i_ant][SourceFWAntsIdxs[i_ant][3*idx-2]+1];
      SourceFWIdxs[0]++;
      SourceFWIdxs[3*SourceFWIdxs[0]-2]=SourceFWAntsIdxs[i_ant][3*idx-2];
      SourceFWIdxs[3*SourceFWIdxs[0]-1]=SourceFWAntsIdxs[i_ant][3*idx-1];
      SourceFWIdxs[3*SourceFWIdxs[0]]  =SourceFWAntsIdxs[i_ant][3*idx];
    }
  }
  sort(SourceFWIdxs);
  //cerr << "SourceFWIdxs : ";
  for (int idx=1; idx<=SourceFWIdxs[0]; idx++) {
    //cerr << "idx:" << SourceFWIdxs[3*idx-2] << ",";
    //cerr << "F:" << SourceFWIdxs[3*idx-1] << ",";
    //cerr << "E:" << SourceFWIdxs[3*idx] << " ";
  }
  //cerr << endl;

  TargetFWIdxs[0]=0;
  //cerr << "TargetFWRuleIdxs:" << endl;
  for (int idx=1; idx<=TargetFWRuleIdxs[0]; idx++) {
    //cerr << TargetFWRuleIdxs[3*idx-2] << " to " << TargetRuleIdxs[TargetFWRuleIdxs[3*idx-2]+1] << endl;
    TargetFWRuleIdxs[3*idx-2] = TargetRuleIdxs[TargetFWRuleIdxs[3*idx-2]+1];
    TargetFWIdxs[0]++;    
    TargetFWIdxs[3*TargetFWIdxs[0]-2]=TargetFWRuleIdxs[3*idx-2];
    TargetFWIdxs[3*TargetFWIdxs[0]-1]=E2FProjection(TargetFWRuleIdxs[3*idx-2],"_SEP_");
    TargetFWRuleIdxs[3*idx-1] = TargetFWIdxs[3*TargetFWIdxs[0]-1];
    TargetFWIdxs[3*TargetFWIdxs[0]]  =TargetFWRuleIdxs[3*idx];
  }
  for (int i_ant=0; i_ant<_Arity; i_ant++) {
    //cerr << "TargetFWAntsIdxs[" << i_ant << "]" << endl;
    for (int idx=1; idx<=TargetFWAntsIdxs[i_ant][0]; idx++) {
      //cerr << TargetFWAntsIdxs[i_ant][3*idx-2] << " to " << TargetAntsIdxs[i_ant][TargetFWAntsIdxs[i_ant][3*idx-2]+1] << endl;
      TargetFWAntsIdxs[i_ant][3*idx-2] = TargetAntsIdxs[i_ant][TargetFWAntsIdxs[i_ant][3*idx-2]+1];
      TargetFWIdxs[0]++;
      TargetFWIdxs[3*TargetFWIdxs[0]-2]=TargetFWAntsIdxs[i_ant][3*idx-2];
      TargetFWIdxs[3*TargetFWIdxs[0]-1]=TargetFWAntsIdxs[i_ant][3*idx-1];
      TargetFWIdxs[3*TargetFWIdxs[0]]  =TargetFWAntsIdxs[i_ant][3*idx];
    }
  }
  sort(TargetFWIdxs);
  //cerr << "TargetFWIdxs : ";
  for (int idx=1; idx<=TargetFWIdxs[0]; idx++) {
    //cerr << "idx:" << TargetFWIdxs[3*idx-2]<< ",";
    //cerr << "E:" << TargetFWIdxs[3*idx-1]<< ",";
    //cerr << "F:" << TargetFWIdxs[3*idx]<< " ";
  }
  //cerr << endl;
  //cerr << AsString() << endl;
  fas = firstSourceAligned(1); las = lastSourceAligned(_J-2);
  fat = firstTargetAligned(1); lat = lastTargetAligned(_I-2); 
  //cerr << "fas=" << fas << ", las=" << las << ", fat=" << fat << ", lat=" << lat << endl;
  assert(fas<=las);
  assert(fat<=lat);
  curr_al = ToVectorInt();
  //cerr << "end prepare" << endl;
  return false;
}      

string Alignment::AsString() {
  ostringstream stream;
  stream << "J:" << getJ() << " I:" << getI();
  for (int j=0; j<getJ(); j++) {
    int t = targetOf(j,minTSpan(j));
    while (t>=0) {
      stream << " " << j << "-" << t;
      t = targetOf(j,t+1);
    }
  }
  stream << " TargetSpan:";
  for (int j=0; j<getJ(); j++)
    if (minTSpan(j)!=MINIMUM_INIT)
      stream << " " << j << "[" << minTSpan(j) << "," << maxTSpan(j) << "]";
    else
      stream << " " << j << "[-,-]";
  stream << " SourceSpan:";
  for (int i=0; i<getI(); i++)
    if (minSSpan(i)!=MINIMUM_INIT)
      stream << " " << i << "[" << minSSpan(i) << "," << maxSSpan(i) << "]";
    else
      stream << " " << i << "[-,-]";
  return stream.str();
};

vector<int> Alignment::ToVectorInt() {
  vector<int> ret;
  for (int j=0; j<_J; j++) {
    int i = targetOf(j);
    while (i>=0) {
      ret.push_back(link(j,i));
      i = targetOf(j,i+1);
    }
  }
  return ret;
}
