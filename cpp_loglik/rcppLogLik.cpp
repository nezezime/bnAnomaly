#include "rcppLogLik.h"

BayesNet network;

// [[Rcpp::export]]
void rcpp_print_tables()
{
  network.printNodes();
}

// [[Rcpp::export]]
double cpp_logLik(DataFrame evidence)
{
  unsigned int evidence_size = evidence.size();

  //read column names
  StringVector colnames = evidence.names();

  std::unordered_map<std::string, std::string> value_pairs;
  for(unsigned int i=0; i<evidence_size; i++)
  {
    std::string col = static_cast<std::string> (colnames[i]);
    std::string val = static_cast<std::string> (static_cast<StringVector> (evidence[i])[0]);
    value_pairs.insert({col, val});
  }

  double log_lik = 0;
  for(unsigned int i=0; i<network.nNodes(); i++)
  {
    BnNode *node = network.nodes.at(i);
    double table_value;
    std::string key;
    //std::cout << "node " << node->name << ", value ";

    key = value_pairs.at(node->name);
    if(node->is_parent == false)
    {
      //so that the correct key is generated
      for(unsigned int j=0; j<node->nParents(); j++)
      {
        key += value_pairs.at(node->parentName(j));
      }
    }
    table_value = node->readTableByKey(key);
    log_lik += table_value;
    //std::cout << table_value << " for key " << key << std::endl;
  }

  return log_lik;
}

//use rules from OutlierMiner algorithm to compute sample log-likelihood
// [[Rcpp::export]]
double cpp_logLikOm(DataFrame evidence)
{
  //tmp - take as parameters
  double maxconf = log(0.8);
  double minconf = log(0.1);
  unsigned int evidence_size = evidence.size();
  StringVector colnames = evidence.names();

  //make key-value pairs from evidence
  std::unordered_map<std::string, std::string> value_pairs;
  for(unsigned int i=0; i<evidence_size; i++)
  {
    std::string col = static_cast<std::string> (colnames[i]);
    std::string val = static_cast<std::string> (static_cast<StringVector> (evidence[i])[0]);
    value_pairs.insert({col, val});
  }

  double log_lik = 0;
  for(unsigned int i=0; i<network.nNodes(); i++)
  {
    BnNode *node = network.nodes.at(i);
    double table_value;
    std::string key;

    key = value_pairs.at(node->name);
    if(node->is_parent == false)
    {
      //so that the correct key is generated
      for(unsigned int j=0; j<node->nParents(); j++)
      {
        std::string parent_name = node->parentName(j);
        key += value_pairs.at(parent_name);
      }
    }

    table_value = node->readTableByKey(key);

    bool R1 = true;
    bool R2 = true;

    //apply OutlierMiner rules
    if(node->is_parent == false)
    {
      for(unsigned int j=0; j<node->nParents(); j++)
      {
        std::string parent_name = node->parentName(j);
        //std::cout << "parent " << parent_name << std::endl;

        //check if parent node of current node is root node
        std::unordered_map<std::string, BnNode *>::iterator it_root;
        it_root = network.parent_nodes.find(parent_name);
        if(it_root != network.parent_nodes.end())
        {
          BnNode *root_node = it_root->second;
          std::string root_key = value_pairs.at(parent_name);

          double prior_val = root_node->readTableByKey(root_key);
          bool prior = (prior_val == root_node->minsupp) ? true:false;

          R1 = (prior && table_value >= maxconf) ? true:false;
          R2 = (!prior && table_value <= minconf) ? true:false;
        }
      }
    }

    if(R1 || R2)
    {
      log_lik += table_value;
    }
  }

  return log_lik;
}

// init bnlearn bn.fit node parameters and add it to Bayesian network reference
// [[Rcpp::export]]
int rcpp_init_node(DataFrame bn_table, String bn_node_name, StringVector bn_node_parents, std::string log_base)
{
  bool DEBUG = false;
  bool NA_PRESENT = false;
  bool HAS_ZERO_PROBABILITIES = false;
  unsigned int n_col = bn_table.size();
  unsigned int n_row = bn_table.nrow();

  // required in outlierMiner for parent nodes
  double minsupp = 0;

  std::vector<StringVector> event_columns;
  std::string node_name = static_cast<std::string> (bn_node_name.get_cstring());
  std::vector<std::string> node_parents;

  double (*log_fn) (double) = log10;
  if(log_base.compare("e") == 0)
  {
    log_fn = log;
  }

  std::cout << "node " << node_name << " with parents ";
  for(int i=0; i<bn_node_parents.size(); i++)
  {
    std::string parent_name = static_cast<std::string> (bn_node_parents[i]);
    std::cout << parent_name << ", ";
    node_parents.push_back(parent_name);
  }
  std::cout << std::endl;

  BnNode *node = network.addNode(node_name, node_parents);

  //display input data in console
  if(DEBUG)
  {
    std::cout << "table column number: " << std::to_string(n_col) << std::endl;
    std::cout << "table row number: " << std::to_string(n_row) << std::endl;

    for(int i=0; i<bn_table.size(); i++)
    {
      StringVector col = bn_table[i];
      std::cout << "table row number: " << std::to_string(col.size()) << std::endl;

      for(int j=0; j<col.size(); j++)
      {
        std::cout << col[j] << std::endl;        
      }
    }
  }

  for(unsigned int i=0; i<n_col-1; i++)
  {
    event_columns.push_back(static_cast<StringVector> (bn_table[i]));
  }
  NumericVector prob_column = bn_table[n_col - 1];
  std::cout << "event columns size " << event_columns.size() << std::endl;


  // each row corresponds to an entry in map
  for(unsigned int i=0; i<n_row; i++)
  {
    //generate map keys from event labels
    std::string map_key;

    for(unsigned int j=0; j<event_columns.size(); j++)
    {
      map_key = map_key + static_cast<std::string> (event_columns.at(j)[i]);
    }

    double value;
    if(NumericVector::is_na(prob_column[i]))
    {
      NA_PRESENT = true;
      value = 1;
    }
    else
    {
      value = static_cast<double> (prob_column[i]);
    }
    //zero probability results in -inf log
    if(value == 0)
    {
      HAS_ZERO_PROBABILITIES = true;
      value = 1e-50;


      node->addTableElement(map_key, value);
    }
    else
    {
      value = log_fn(value);
      node->addTableElement(map_key, value);
    }

    //update minsupp if required
    if(value < minsupp)
    {
      minsupp = value;
    }
  }
  //network.printNodes();

  if(node->is_parent == true)
  {
    node->minsupp = minsupp;
    std::cout << "parent node minsupp: " << minsupp << std::endl;
  }

  if(NA_PRESENT)
  {
    warning("Input probability table contains NaN values");
  }
  
  if(HAS_ZERO_PROBABILITIES)
  {
    warning("Input probability table contains zero probabilities");
  }

  return 0;
}
