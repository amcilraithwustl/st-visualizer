#include <cmath>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

int main()
{
    vector<string> test = {"a", "b", "c"};
    string *s = &test[0];
    cout << *s << endl;
    return 0;
}