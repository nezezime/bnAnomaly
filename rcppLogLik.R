cpp.init <- function(bn_fitted, log_base = "10")
{
  library(Rcpp)
  Sys.setenv("PKG_CXXFLAGS"="-std=c++11")
  sourceCpp('./cpp_source/rcppLogLik.cpp')
  print("Functions successfully built from source")
  
  if(class(bn_fitted)[1] != "bn.fit")
  {
    stop("bn_fitted input arg should me a member of bn.fit")
  }
  
  for(dnode in bn_fitted)
  {
    dnode_name <- dnode$node
    dnode_parents <- dnode$parents
    if(rcpp_init_node(as.data.frame(dnode$prob), dnode_name, dnode_parents, log_base) != 0)
    {
      stop("C++ init function error")
    }
  }
}

cpp.printTables <- function()
{
  rcpp_print_tables()
}