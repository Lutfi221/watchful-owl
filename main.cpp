#include <iostream>
#include "capturer.h"

using namespace std;

int main()
{
    vector<AppRecord> apps;
    getOpenedApps(&apps);

    for (auto const& appRecord: apps) {
        cout << appRecord.title << endl;
    }

    cin.ignore();
    return 0;
}
