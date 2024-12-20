#include <iostream>
#include <string.h>
#include <vector>
#include <map>
#include "resp.cpp"

using namespace std;

resp ping(resp y){
if(y.bulk==""){y.bulk="PONG";}
return y;
}


//vai tomar no cu codigo do caralho
//tem nem sentido saporra