#include <stdlib.h>
#include <vector>
#include <map>
#include <string>
#include <ostream>
#include "wordid.h"
#include "trule.h"
#include <boost/functional/hash.hpp>
#include <tr1/unordered_map>
#include <boost/tuple/tuple.hpp>

using namespace std;
using namespace std::tr1;
using namespace boost::tuples;
using namespace boost;

#ifndef DWARF_H
#define DWARF_H

class CountTable {
public:
        int* third;
        map<WordID,int*> second;
        map<WordID,int*> first;
};

class Alignment {
public:
  const static int MAX_WORDS = 200;
  const static int MINIMUM_INIT = 1000;
  const static int MAXIMUM_INIT = -1000;
  const static int MAX_ARITY = 2;
  const static double alpha1 = 0.01;
  const static double alpha2 = 0.01;
  const static double alpha3 = 0.01;
  const static double alpha4 = 0.01;
  //static bool _matrix[MAX_WORDS][MAX_WORDS]; 
  //static short _sSpan[MAX_WORDS][2]; //the source span of a target index; 0->min, 1->max
  //static short _tSpan[MAX_WORDS][2]; //the target span of a source index; 0->min, 2->max
  //static short _sConsistent[MAX_WORDS][3]; //known consistent span (to speed up) from a source index i; i:[0]-[1]:[2]    
  void set(int j,int i); // j is the source index, while i is the target index
  void reset(int j,int i); 
  int getJ() {return _J;};
  int getI() {return _I;};
  void setI(int I) { _I = I;};
  void setJ(int J) { _J = J;};
  void setF(vector<WordID> f) { _f=f;};
  void setE(vector<WordID> e) { _e=e;};
  void clearAls(int prevJ=200, int prevI=200);
  int sourceOf(int i, int start = -1);
  int targetOf(int j, int start = -1);
  int minSSpan(int i) { return _sSpan[i][0];}
  int maxSSpan(int i) { return _sSpan[i][1];}
  int minTSpan(int j) { return _tSpan[j][0];}
  int maxTSpan(int j) { return _tSpan[j][1];}
  Alignment();
  bool prepare(TRule& rule, const std::vector<const void*>& ant_contexts, map<WordID,int> sfw, map<WordID,int> tfw);
  void simplify(int *ret);
  void simplify_nofw(int *ret);
  int DominanceSource(int fw1, int fw2);
  void OrientationSource(int fw, int*oril, int* orir, bool Lcompute=true, bool Rcompute=true);
  int DominanceTarget(int fw1, int fw2);
  void OrientationTarget(int fw, int*oril, int* orir, bool Lcompute=true, bool Rcompute=true);
  void ScoreOrientationRight(CountTable table, int ori, WordID cond1, WordID cond2, bool isBonus, double *cost, double *bonus, double *bo1, double *bo1_bonus, double *bo2, double *bo2_bonus);
  void ScoreOrientationLeft(CountTable table, int ori, WordID cond1, WordID cond, bool isBonus, double *cost, double *bonus, double *bo1, double *bo1_bonus, double *bo2, double *bo2_bonus);
  void ScoreOrientation(CountTable table, int offset, int ori, WordID cond1, WordID cond2, bool isBonus, double *cost, double *bonus, double *bo1, double *bo1_bonus, double *bo2, double *bo2_bonus);
  void computeOrientationSource(CountTable table, double *cost, double *bonus, double *bo1, double *bo1_bonus, double *bo2, double *bo2_bonus);
  void computeOrientationTarget(CountTable table, double *cost, double *bonus, double *bo1, double *bo1_bonus, double *bo2, double *bo2_bonus);
  void computeDominanceSource(CountTable table, double *cost, double *bonus, double *bo1, double *bo1_bonus, double *bo2, double *bo2_bonus);
  void computeDominanceTarget(CountTable table, double *cost, double *bonus, double *bo1, double *bo1_bonus, double *bo2, double *bo2_bonus);
  void ScoreDominance(CountTable table, int dom, WordID s1, WordID s2, WordID t1, WordID t2, double *cost, double *bonus, double *bo1, double *bo1_bonus, double *bo2, double *bo2_bonus);
  void SetSourceBorderingFW();
  void SetTargetBorderingFW();
  void fillFWIdxs(int *state, int fas, int las, int fat, int lat);
  WordID F2EProjection(int idx, string delimiter=" ");
  WordID E2FProjection(int idx, string delimiter=" ");
  vector<int> ToVectorInt();
  string AsString();
  static int link(int s, int t) { return s*65536 + t; }
  static int source(int st) {return st / 65536; }
  static int target(int st) {return st % 65536; }
  
private:
  unordered_map<vector<int>, int, boost::hash<vector<int> > > oris_hash;
  unordered_map<vector<int>, int, boost::hash<vector<int> > > orit_hash;
  unordered_map<vector<int>, int, boost::hash<vector<int> > > doms_hash;
  unordered_map<vector<int>, int, boost::hash<vector<int> > > domt_hash;
  unordered_map<vector<int>, vector<int>, boost::hash<vector<int> > > simplify_hash;
  unordered_map<vector<int>, vector<int>, boost::hash<vector<int> > > prepare_hash;
  int _J; // source length;
  int _I; // target length;
  bool _matrix[MAX_WORDS][MAX_WORDS]; 
  short _sSpan[MAX_WORDS][2]; //the source span of a target index; 0->min, 1->max
  short _tSpan[MAX_WORDS][2]; //the target span of a source index; 0->min, 2->max
  short _sConsistent[MAX_WORDS][3]; //known consistent span (to speed up) from a source index i; i:[0]-[1]:[2]    
  int SourceFWRuleIdxs[40]; //the indexes of function words in the rule; 
          // applies to all *FW*Idxs
          // *FW*Idxs[0] = size
          // *FW*Idxs[idx*3-2] = index in the alignment, where idx starts from 1 to size
          // *FW*Idxs[idx*3-1] = source WordID
          // *FW*Idxs[idx*3]   = target WordID
  int TargetFWRuleIdxs[40]; //the indexes of function words in the rule; zeroth element is the count
  int ** SourceFWAntsIdxs;  //the indexes of function words in antecedents
  int ** TargetFWAntsIdxs;  //the indexes of function words in antecedents
  int SourceRuleIdxs[40]; //the indexes of SOURCE tokens (zeroth element is the number of source tokens)
        //>0 means terminal, -i means the i-th Xs
  int TargetRuleIdxs[40]; //the indexes of TARGET tokens (zeroth element is the number of target tokens)
  int ** SourceAntsIdxs;  //the array of indexes of a particular antecedent's SOURCE tokens
  int ** TargetAntsIdxs;  //the array of indexes of a particular antecedent's TARGET tokens
  int SourceFWIdxs[40];
  int TargetFWIdxs[40];
  int TotalSource;    
  int TotalTarget;
  void sort(int* num);
  void quickSort(int arr[], int top, int bottom);
  int* blockSource(int fw1, int fw2);
  int* blockTarget(int fw1, int fw2);
  int least(int i1, int i2) { return (i1<i2)?i1:i2; }
  int most(int i1, int i2) { return (i1>i2)?i1:i2; }
  void SplitIfViolateDanglingTargetFWIdxs(vector<int *>*blocks, int* block, vector<int>danglings);
  int _Arity;
  std::vector<WordID> _f;
  std::vector<WordID> _e;
  int RuleAl[40];
  int **AntsAl;
  int firstSourceAligned(int start);
  int firstTargetAligned(int start);
  int lastSourceAligned(int end);
  int lastTargetAligned(int end);
  int fas, las, fat, lat; // first aligned source, last aligned source, first aligned target, last aligned target
  bool MemberOf(int* FWIdxs, int pos1, int pos2);
  vector<int> curr_al;
};

#endif
