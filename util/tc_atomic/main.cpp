#include <iostream>
#include "tc_automic.h"

using namespace std;

int main()
{
    tars::TC_Atomic atomic(10);

    atomic.add(1);


    cout << atomic.get() << endl;

    return 0;
}