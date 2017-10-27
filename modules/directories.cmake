# Path to CPLEX Studio.
# CPLEX Studio 12.4 can be installed in the following default locations:
#   /opt/ibm/ILOG/CPLEX_Studio<edition>124 - Linux
#   /opt/IBM/ILOG/CPLEX_Studio<edition>124 - UNIX
#   ~/Applications/IBM/ILOG/CPLEX_Studio<edition>124 - Mac OS X
#   C:\Program Files\IBM\ILOG\CPLEX_Studio<edition>124 - Windows

set(APPLE_DIR $ENV{HOME}/Applications/IBM/ILOG)
set(LINUX_DIR /opt/ibm/ILOG)
set(UNIX_DIR /opt/IBM/ILOG)
set(WIN64_DIR "C:/Program Files/IBM/ILOG")
set(WIN86_DIR "C:/Program Files (x86)/IBM/ILOG")



# Path to GLPK, there is not default location
# so it is necessary to set your library location
set(GLPK_DIR /home/rozliv/Documents/glpk-4.63)
