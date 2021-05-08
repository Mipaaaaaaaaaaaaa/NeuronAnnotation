#include "ErrorMessage.hpp"
#include <iostream>

using namespace std;

int main(){
    ErrorMessage em = ("错误！");
    cout << em.ToJson() << endl;
}