#include <iostream>
using namespace std;

int add(int x, int y){
    return x + y;
}

int main(){
    int z;
    int x, y;
    cout << "Enter two numbers" << endl;
    cin >> x >> y ;
    cout << add(x, y);
    return 0;
}
