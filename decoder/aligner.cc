#include "aligner.h"

#include "array2d.h"
#include "hg.h"
#include "sentence_metadata.h"
#include "inside_outside.h"
#include "viterbi.h"
#include <set>

using namespace std;

static bool is_digit(char x) { return x >= '0' && x <= '9'; }

boost::shared_ptr<Array2D<bool> > AlignerTools::ReadPharaohAlignmentGrid(const string& al) {
  int max_x = 0;
  int max_y = 0;
  int i = 0;
  size_t pos = al.rfind(" ||| ");
  if (pos != string::npos) { i = pos + 5; }
  while (i < al.size()) {
    if (al[i] == '\n' || al[i] == '\r') break;
    int x = 0;
    while(i < al.size() && is_digit(al[i])) {
      x *= 10;
      x += al[i] - '0';
      ++i;
    }
    if (x > max_x) max_x = x;
    assert(i < al.size());
    if(al[i] != '-') {
      cerr << "BAD ALIGNMENT: " << al << endl;
      abort();
    }
    ++i;
    int y = 0;
    while(i < al.size() && is_digit(al[i])) {
      y *= 10;
      y += al[i] - '0';
      ++i;
    }
    if (y > max_y) max_y = y;
    while(i < al.size() && al[i] == ' ') { ++i; }
  }

  boost::shared_ptr<Array2D<bool> > grid(new Array2D<bool>(max_x + 1, max_y + 1));
  i = 0;
  if (pos != string::npos) { i = pos + 5; }
  while (i < al.size()) {
    if (al[i] == '\n' || al[i] == '\r') break;
    int x = 0;
    while(i < al.size() && is_digit(al[i])) {
      x *= 10;
      x += al[i] - '0';
      ++i;
    }
    assert(i < al.size());
    assert(al[i] == '-');
    ++i;
    int y = 0;
    while(i < al.size() && is_digit(al[i])) {
      y *= 10;
      y += al[i] - '0';
      ++i;
    }
    (*grid)(x, y) = true;
    while(i < al.size() && al[i] == ' ') { ++i; }
  }
  // cerr << *grid << endl;
  return grid;
}

void AlignerTools::SerializePharaohFormat(const Array2D<bool>& alignment, ostream* out) {
  bool need_space = false;
  for (int i = 0; i < alignment.width(); ++i)
    for (int j = 0; j < alignment.height(); ++j)
      if (alignment(i,j)) {
        if (need_space) (*out) << ' '; else need_space = true;
        (*out) << i << '-' << j;
      }
  (*out) << endl;
}

// used with lexical models since they may not fully generate the
// source string
void SourceEdgeCoveragesUsingParseIndices(const Hypergraph& g,
                                          vector<set<int> >* src_cov) {
  src_cov->clear();
  src_cov->resize(g.edges_.size());
  
  for (int i = 0; i < g.edges_.size(); ++i) {
    const Hypergraph::Edge& edge = g.edges_[i];
    set<int>& cov = (*src_cov)[i];
    // no words
    if (edge.rule_->EWords() == 0 || edge.rule_->FWords() == 0)
      continue;
    // aligned to NULL (crf ibm variant only)
    if (edge.prev_i_ == -1 || edge.i_ == -1)
      continue;
    assert(edge.j_ >= 0);
    assert(edge.prev_j_ >= 0);
    if (edge.Arity() == 0) {
      for (int k = edge.prev_i_; k < edge.prev_j_; ++k)
        cov.insert(k);
    } else {
      // note: this code, which handles mixed NT and terminal
      // rules assumes that nodes uniquely define a src and trg
      // span.
      int k = edge.prev_i_;
      int j = 0;
      const vector<WordID>& f = edge.rule_->e();  // rules are inverted
      while (k < edge.prev_j_) {
        if (f[j] > 0) {
          cov.insert(k);
          // cerr << "src: " << k << endl;
          ++k;
          ++j;
        } else {
          const Hypergraph::Node& tailnode = g.nodes_[edge.tail_nodes_[-f[j]]];
          assert(tailnode.in_edges_.size() > 0);
          // any edge will do:
          const Hypergraph::Edge& rep_edge = g.edges_[tailnode.in_edges_.front()];
          //cerr << "skip " << (rep_edge.prev_j_ - rep_edge.prev_i_) << endl;  // src span
          k += (rep_edge.prev_j_ - rep_edge.prev_i_);  // src span
          ++j;
        }
      }
    }
  }
}

int SourceEdgeCoveragesUsingTree(const Hypergraph& g,
                                 int node_id,
                                 int span_start,
                                 vector<int>* spans,
                                 vector<set<int> >* src_cov) {
  const Hypergraph::Node& node = g.nodes_[node_id];
  int k = -1;
  for (int i = 0; i < node.in_edges_.size(); ++i) {
    const int edge_id = node.in_edges_[i];
    const Hypergraph::Edge& edge = g.edges_[edge_id];
    set<int>& cov = (*src_cov)[edge_id];
    const vector<WordID>& f = edge.rule_->e();  // rules are inverted
    int j = 0;
    k = span_start;
    while (j < f.size()) {
      if (f[j] > 0) {
        cov.insert(k);
        ++k;
        ++j;
      } else {
        const int tail_node_id = edge.tail_nodes_[-f[j]];
        int &right_edge = (*spans)[tail_node_id];
        if (right_edge < 0)
          right_edge = SourceEdgeCoveragesUsingTree(g, tail_node_id, k, spans, src_cov);
        k = right_edge;
        ++j;
      }
    }
  }
  return k;
}

void SourceEdgeCoveragesUsingTree(const Hypergraph& g,
                                  vector<set<int> >* src_cov) {
  src_cov->clear();
  src_cov->resize(g.edges_.size());
  vector<int> span_sizes(g.nodes_.size(), -1);
  SourceEdgeCoveragesUsingTree(g, g.nodes_.size() - 1, 0, &span_sizes, src_cov);
}

int TargetEdgeCoveragesUsingTree(const Hypergraph& g,
                                 int node_id,
                                 int span_start,
                                 vector<int>* spans,
                                 vector<set<int> >* trg_cov) {
  const Hypergraph::Node& node = g.nodes_[node_id];
  int k = -1;
  for (int i = 0; i < node.in_edges_.size(); ++i) {
    const int edge_id = node.in_edges_[i];
    const Hypergraph::Edge& edge = g.edges_[edge_id];
    set<int>& cov = (*trg_cov)[edge_id];
    int ntc = 0;
    const vector<WordID>& e = edge.rule_->f();  // rules are inverted
    int j = 0;
    k = span_start;
    while (j < e.size()) {
      if (e[j] > 0) {
        cov.insert(k);
        ++k;
        ++j;
      } else {
        const int tail_node_id = edge.tail_nodes_[ntc];
        ++ntc;
        int &right_edge = (*spans)[tail_node_id];
        if (right_edge < 0)
          right_edge = TargetEdgeCoveragesUsingTree(g, tail_node_id, k, spans, trg_cov);
        k = right_edge;
        ++j;
      }
    }
    // cerr << "node=" << node_id << ": k=" << k << endl;
  }
  return k;
}

void TargetEdgeCoveragesUsingTree(const Hypergraph& g,
                                  vector<set<int> >* trg_cov) {
  trg_cov->clear();
  trg_cov->resize(g.edges_.size());
  vector<int> span_sizes(g.nodes_.size(), -1);
  TargetEdgeCoveragesUsingTree(g, g.nodes_.size() - 1, 0, &span_sizes, trg_cov);
}

// this code is rather complicated since it must deal with generating alignments
// when lattices are specified as input as well as with models that do not generate
// full sentence pairs (like lexical alignment models)
void AlignerTools::WriteAlignment(const Lattice& src_lattice,
                                  const Lattice& trg_lattice,
                                  const Hypergraph& in_g,
                                  ostream* out,
                                  bool map_instead_of_viterbi,
                                  const vector<bool>* edges) {
  bool fix_up_src_spans = false;
  const Hypergraph* g = &in_g;
  if (!src_lattice.IsSentence() ||
      !trg_lattice.IsSentence()) {
    if (map_instead_of_viterbi) {
      cerr << "  Lattice alignment: using Viterbi instead of MAP alignment\n";
    }
    map_instead_of_viterbi = false;
    fix_up_src_spans = !src_lattice.IsSentence();
  }
  if (!map_instead_of_viterbi || edges) {
    Hypergraph* new_hg = in_g.CreateViterbiHypergraph(edges);
    for (int i = 0; i < new_hg->edges_.size(); ++i)
      new_hg->edges_[i].edge_prob_ = prob_t::One();
    g = new_hg;
  }

  vector<prob_t> edge_posteriors(g->edges_.size(), prob_t::Zero());
  vector<WordID> trg_sent;
  vector<WordID> src_sent;
  if (fix_up_src_spans) {
    ViterbiESentence(*g, &src_sent);
  } else {
    src_sent.resize(src_lattice.size());
    for (int i = 0; i < src_sent.size(); ++i)
      src_sent[i] = src_lattice[i][0].label;
  }

  ViterbiFSentence(*g, &trg_sent);

  if (edges || !map_instead_of_viterbi) {
    for (int i = 0; i < edge_posteriors.size(); ++i)
      edge_posteriors[i] = prob_t::One();
  } else { 
    SparseVector<prob_t> posts;
    InsideOutside<prob_t, EdgeProb, SparseVector<prob_t>, TransitionEventWeightFunction>(*g, &posts);
    for (int i = 0; i < edge_posteriors.size(); ++i)
      edge_posteriors[i] = posts[i];
  }
  vector<set<int> > src_cov(g->edges_.size());
  vector<set<int> > trg_cov(g->edges_.size());
  TargetEdgeCoveragesUsingTree(*g, &trg_cov);

  if (fix_up_src_spans)
    SourceEdgeCoveragesUsingTree(*g, &src_cov);
  else
    SourceEdgeCoveragesUsingParseIndices(*g, &src_cov);

  // figure out the src and reference size;
  int src_size = src_sent.size();
  int ref_size = trg_sent.size();
  Array2D<prob_t> align(src_size, ref_size, prob_t::Zero());
  for (int c = 0; c < g->edges_.size(); ++c) {
    const prob_t& p = edge_posteriors[c];
    const set<int>& srcs = src_cov[c];
    const set<int>& trgs = trg_cov[c];
    for (set<int>::const_iterator si = srcs.begin();
         si != srcs.end(); ++si) {
      for (set<int>::const_iterator ti = trgs.begin();
           ti != trgs.end(); ++ti) {
        align(*si, *ti) += p;
      }
    }
  }
  if (g != &in_g) { delete g; g = NULL; }

  prob_t threshold(0.9);
  const bool use_soft_threshold = true; // TODO configure

  Array2D<bool> grid(src_size, ref_size, false);
  for (int j = 0; j < ref_size; ++j) {
    if (use_soft_threshold) {
      threshold = prob_t::Zero();
      for (int i = 0; i < src_size; ++i)
        if (align(i, j) > threshold) threshold = align(i, j);
      //threshold *= prob_t(0.99);
    }
    for (int i = 0; i < src_size; ++i)
      grid(i, j) = align(i, j) >= threshold;
  }
  if (out == &cout) {
    // TODO need to do some sort of verbose flag
    cerr << align << endl;
    cerr << grid << endl;
  }
  (*out) << TD::GetString(src_sent) << " ||| " << TD::GetString(trg_sent) << " ||| ";
  SerializePharaohFormat(grid, out);
};

