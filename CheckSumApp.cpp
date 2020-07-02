// CheckSumApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include<stdio.h>
#include<string>



namespace fs = std::filesystem;
using namespace std;

string exCMD(string command);
void ckAndPrint(const fs::directory_entry& PATH, unordered_map<string, string>& M);
void popMap(unordered_map<string, string>& mapRef, fs::path& mainPath);
void test(unordered_map<string, string>& preFile, unordered_map<string, string>& preFile2);

void ckAndPrint(const fs::directory_entry& PATH, unordered_map<string, string>& M) { 
    //A func to take a file path, get the md5sum and store both in the unordered map;
    string filePath = PATH.path().string();
    string first = " certutil -hashfile \"";
    string last = "\" md5";
    string fullComand = first + filePath + last;
    string resultCK = exCMD(fullComand); //Forming and sending the comand through a pipe

    int pos = resultCK.find(filePath);
    int len = filePath.size();
    resultCK.erase(pos, len);
    resultCK.erase(0, 13);
    resultCK.erase(33);  // Isolating the MD5 sum value

    pos = filePath.rfind("\\");
    filePath = filePath.substr(pos); //Isolating the file name
    M[filePath] = resultCK;  //Pushing the name and value into the map
}
string exCMD(string command) {
    //A basic function to pipe a command to system and return the result
    char buffer[128];
    string result = "";
    FILE* pipe = _popen(command.c_str(), "r"); //Opening the pipe to FILE
    if (!pipe) {
        throw " popen failed! Pipe to CMD line failed to open\n Please restart application and try again\n";
    }
    while (!feof(pipe)) { //Read to end of proccess
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer; //Using a char array buffer to add to the result string
    }
    _pclose(pipe);
    return result;
}
void popMap(unordered_map<string, string>& mapRef, fs::path& mainPath) {
    //A function that pushes every path in a directory to ckAndPrint
    fs::recursive_directory_iterator dirIt(mainPath);
    for (const fs::directory_entry& p : dirIt) {
        if (!p.is_directory()) {
            ckAndPrint(p, mapRef);
        }
    }
}
void test(unordered_map<string, string>& preFile, unordered_map<string, string>& preFile2) {
    //A function that confirms what files, if any, are different in a directory
    if (preFile.size()!=preFile2.size()) {
        throw "Directories contain a different amount of items....."; //Confirms both directorys have the same amount of elements
    }
    bool testBool;
    int testVar = 0;
    for (auto it = preFile.begin(); it != preFile.end(); it++) { //Test if each files MD5 sum is the same across each directory
        if (it->second != preFile2[it->first]) {
            testVar++;
        }
    }
        if (testVar == 0) { testBool = true; }
        else { testBool = false; }

    if (testBool==true) {  //Printing the results of the previouse for loop
        cout << "CKsum is the same;\n";
    }
    else {
        cout << "Sum is different in the following files;\n";
        for (auto it = preFile.begin(); it != preFile.end(); it++) { //Printing any functions that failed the previouse test
            if (it->second != preFile2[it->first]) {
                cout << it->first << endl;
            }
        }

    }

}

int main()
//we need to get a dir path, get each files cksum, and then compair to another instance.
{
    fs::path mainPath;
    fs::path mainPath2;
    unordered_map<string, string> preFile;
    auto refPreFile = ref(preFile);
    unordered_map<string, string> preFile2;
    auto refPreFile2 = ref(preFile2);
    int menuItem;

    cout << ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n ====================   Welcome to CKsum   ====================\n";
    cout<< ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n 1:Checksum two files and compair\n";
    cout << " 2:Compare a past check to a new one\n 3:Check one and output a file with data\n 4:Quit\n Enter a number to choose...\n";
     cin >> menuItem;

     try {
         if (menuItem == 1)
         {
             string x;
             cout << "Input the address of the first folder you want to check:\n";    //collect main directory path
             cin.ignore();
             getline(cin, x);
             mainPath = x;
             popMap(refPreFile, mainPath); //Populating a map with each file and its MD5 sum

             x = " ";
             cout << "Input the address of the second folder you want to check:\n";    //collect copied directory path
             getline(cin, x);
             mainPath2 = x;
             popMap(refPreFile2, mainPath2); //Populating a map with each file and its MD5 sum again

             test(refPreFile, refPreFile2); //Testing if both maps are the same and printing the results 

         }
         else if (menuItem == 2) {
             string x;
             cout << "Input the outCK.txt file location:\n"; //Collecting the old file path
             cin.ignore();
             getline(cin, x);
             ifstream outCKIn(x);
           if (outCKIn.is_open()) {  //Importing the file data to a map
                 string name, num;
                 while (getline(outCKIn, name), getline(outCKIn, num)) {
                     num.insert(0, "\n");
                     preFile.insert({ name, num });
                 }
                 outCKIn.close();
           }
             else { throw "File path invalid..."; }

             cout << "Input the address of the folder you want to compair:\n";    //Collecting main directory path
             getline(cin, x);
             mainPath = x;
             popMap(refPreFile2, mainPath);  //Populating a map with the file names and MD5 data
             test(refPreFile, refPreFile2);  //Testing if the map data matches eachother and printing the results
         }
         else if (menuItem == 3) {
             string x;
             cout << "Input the desired directory:\n"; //Collecting the directory
             cin.ignore();
             getline(cin, x);
             mainPath = x;

             popMap(refPreFile, mainPath);  //Creating a map with the file names and MD5 data
             ofstream outCK("outCK.txt");   //Creating a file to export data to
             for (auto it = preFile.begin(); it != preFile.end(); it++) {  //Exporting data
                 outCK << it->first << it->second << endl;
             }
             cout << "File \"outCK.txt\" created at program launch location.\n";
             outCK.close();
         }
         else {
             cout << "Exiting....\n";
         }
     }
     catch (string& x) { cout << x << "\nExiting...\n";}   //Exception catches for bad paths and various errors
     catch (fs::filesystem_error& x) { cout << x.what() << "\nMost likely a bad path...\nExiting.....\n"; }
} 

