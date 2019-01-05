#ifndef RCPP_LOG_LIK_H
#define RCPP_LOG_LIK_H

#include <Rcpp.h>
#include <Rinternals.h>

#include <map>
#include <string>
#include <vector>

using namespace Rcpp;

class BnNode
{
  private:
  std::unordered_map<std::string, double> table;
  std::vector<std::string> parent_names;

  public:
  std::string name;
  bool is_parent;

  //minimum probability in parent node, used by OutlierMiner
  double minsupp;

  BnNode(std::string node_name, std::vector<std::string> node_parent_names)
  {
    std::cout << "BnNode constructor" << std::endl;
    this->name = node_name;

    if(node_parent_names.size() == 0)
    {
      is_parent = true;
    }
    else
    {
      is_parent = false;
    }
    this->parent_names = node_parent_names;
  }

  int addTableElement(const std::string key, const double value)
  {
    this->table.insert({key, value});
    return 0;
  }

  double readTableByKey(const std::string key)
  {
    return this->table.at(key);
  }

  void printTable(void)
  {
    std::cout << "printTable for node " << this->name << std::endl;
    std::unordered_map<std::string, double>::iterator it;
    for(it=this->table.begin(); it!=this->table.end(); it++)
    {
      std::cout << it->first << ": " << it->second << std::endl;
    }
  }

  void printParents()
  {
    if(this->is_parent == true)
    {
      std::cout << "this is a parent node" << std::endl;
      return;
    }

    unsigned int parent_names_size = this->parent_names.size();
    for(unsigned int i=0; i<parent_names_size; i++)
    {
      std::cout << this->parent_names.at(i) << ", ";
    }
  }

  unsigned int nParents()
  {
    return parent_names.size();
  }

  std::string parentName(unsigned int index)
  {
    return parent_names.at(index);
  }
};

class BayesNet
{
  private:
  public:

  std::vector<BnNode *> nodes;
  std::unordered_map<std::string, BnNode *> parent_nodes;
  unsigned int n_nodes;

  BnNode * addNode(std::string node_name, std::vector<std::string> node_parent_names)
  {
    BnNode * node = new BnNode(node_name, node_parent_names);
    this->nodes.push_back(node);
    this->n_nodes = this->nodes.size();

    //add parent nodes to additional list
    if(node->is_parent == true)
    {
      this->parent_nodes.insert({node_name, node});
    }
    return node;
  }

  int addParentNode(BnNode *parent_node)
  {
    if(parent_node->is_parent == false)
    {
      std::cout << "addParentNode trying to add non-parent node" << std::endl;
      return -1;
    }

    return 0;
  }

  unsigned int nNodes()
  {
    return n_nodes;
  }

  void printNodes()
  {
    std::cout << "printNodes" << std::endl;
    for(unsigned int i=0; i<this->n_nodes; i++)
    {
      std::cout << std::endl << std::endl;
      std::cout << this->nodes.at(i)->name << std::endl;
      std::cout << "parents: ";
      this->nodes.at(i)->printParents();
      this->nodes.at(i)->printTable();
    }
  }

  void printParentNodes()
  {
    std::cout << std::endl << std::endl;
    std::unordered_map<std::string, BnNode *>::iterator it;
    for(it=this->parent_nodes.begin(); it!=this->parent_nodes.end(); it++)
    {
      std::cout << it->first << std::endl;
    }
  }
};

#endif // RCPP_LOG_LIK_H
