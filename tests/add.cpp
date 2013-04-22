#include <iostream>
#include <time.h>
using namespace std;
extern "C" {
int add(int x, int y){
    sleep(5);
    return x + y;
}
}

int main(){
    int z;
    int x, y;
    cout << "Enter two numbers" << endl;
    cin >> x >> y ;
    cout << add(x, y);
    return 0;
}
