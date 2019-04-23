# CPLEX
CPLEX official: 
- http://www-01.ibm.com/support/docview.wss?uid=swg24044295
- https://www-01.ibm.com/support/docview.wss?uid=swg21444285

## Steps to install
- download installer
- make installer executable
- run installer

# Gurobi
Gurobi official: http://www.gurobi.com/documentation/8.1/quickstart_linux/software_installation_guid.html#section:Installation

Docs in pdf: https://cw.fel.cvut.cz/wiki/_media/courses/ko/01_gurobi.pdf


## Steps to install:
- register at gurobi website using academic e-mail and from academic network
- download tar.gz
- unpack in /opt
- get licence key
- update env vars in .bashrc / .zshrc
- when running in clion you may need to set env variables in the runner:
  `GUROBI_HOME=/opt/gurobi811/linux64/;LD_LIBRARY_PATH=/opt/gurobi811/linux64/lib;PATH=/opt/gurobi811/linux64/bin`
  

# GLPK

Docs in pdf: https://kam.mff.cuni.cz/~elias/glpk.pdf

Some notes about file locations:

https://en.wikibooks.org/wiki/GLPK/Linux_OS

File	Role	Default	File
glpsol	command-line solver	/usr/local/bin	executable file
glpk.h	GLPK API header	/usr/local/include	C language header file
libglpk.a	static library	/usr/local/lib	regular archive file
libglpk.so	shared library	/usr/local/lib	symlink to shared object file

Note: if GLPK was added as a Linux package, then the "/local" part is omitted from the paths indicated.

## Steps to install

    wget https://ftp.gnu.org/gnu/glpk/glpk-4.65.tar.gz
    tar -xzf glpk-4.65.tar.gz
    cd glpk-4.65
    ./configure
    make CFLAGS=-O3 LDFLAGS=-s
    sudo make install

