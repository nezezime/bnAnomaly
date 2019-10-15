# bnAnomaly
Anomaly Detection With Bayesian Networks

Build the code in cpp_loglik folder by using the Rcpp package for R (C++ 11 is required):
```
library(Rcpp)
Sys.setenv("PKG_CXXFLAGS"="-std=c++11")  
sourceCpp('path_to_rcppLogLik.cpp')
```

Using the bnlearn package for Bayesian Networks in R is highly recommended (http://www.bnlearn.com/)
  - see bnlearn tutorials on how to specify BN topology
  - fit BN parameters by using bnlearn's bn.fit() function
  - initialize cpp_loglik with the bn.fit object:
  
  - log-likelihood can now be computed with or without the OutlierMiner quantitative rules:
  
Note:
  - currently only complete evidence is supported in log-likelihood computation
  
