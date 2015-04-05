#include <iostream>
#include <ctime>

using namespace std;

int main( )
{
   // current date/time based on current system
   time_t now = time(0);
   now = 11111;
cout << "Number of sec since January 1,1970:" << (now)<< endl;
   cout << "Number of sec since January 1,1970:" << (now + 10)<< endl;

   // tm *ltm = localtime(&now);

   // // print various components of tm structure.
   // cout << "Year: "<< 1900 + ltm->tm_year << endl;
   // cout << "Month: "<< 1 + ltm->tm_mon<< endl;
   // cout << "Day: "<<  ltm->tm_mday << endl;
   // cout << "Time: "<< 1 + ltm->tm_hour << ":";
   // cout << 1 + ltm->tm_min << ":";
   // cout << 1 + ltm->tm_sec << endl;
}