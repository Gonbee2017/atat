#include"atat.h"
#include<iostream>

using namespace atat;
using namespace std;

int main(int argc,char**argv)
{
    setup_io();
    return run(argc,argv,cin,cout,cerr);
}
