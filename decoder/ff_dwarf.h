#include <vector>
#include <map>
#include <string>
#include "ff.h"
#include "dwarf.h"

using namespace std;

class Dwarf : public FeatureFunction {
 public:
  //Dwarf(const std::string& oris_fn, const std::string& orit_fn, const std::string& doms_fn, const std::string& domt_fn);
  Dwarf(const std::string& param);
 protected:
  void TraversalFeaturesImpl(const SentenceMetadata& smeta,
                                     const Hypergraph::Edge& edge,
                                     const std::vector<const void*>& ant_contexts,
                                     SparseVector<double>* features,
                                     SparseVector<double>* estimated_features,
                                     void* context) const;
  bool readOrientation(CountTable* table, std::string filename, std::map<WordID,int> *fw);
  bool readDominance(CountTable* table, std::string filename, std::map<WordID,int> *fw);
 private:
  Alignment* als;
  int oris_;
  int oris_bo1_;
  int oris_bo2_;
  int orit_;
  int orit_bo1_;
  int orit_bo2_;
  int doms_;
  int doms_bo1_;
  int doms_bo2_;
  int domt_;
  int domt_bo1_;
  int domt_bo2_;
  bool flag_oris;
  bool flag_orit;
  bool flag_doms;
  bool flag_domt;
  std::map<WordID,int> sfw;
  std::map<WordID,int> tfw;
  CountTable toris;
  CountTable torit;
  CountTable tdoms;
  CountTable tdomt;
};

