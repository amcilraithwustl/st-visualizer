#include <cmath>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

int func(vector<string> &s)
{
    s[0] = "z";
    return 0;
}

int main()
{
    vector<string> test = {"a", "b", "c"};
    func(test);
    double pi = M_PI;
    cout << pi << endl;
    return 0;
}