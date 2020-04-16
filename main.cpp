#include <iostream>
#include<fstream>
#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include<conio.h>
using namespace std;

#define MAX_DIST 3          // Maximum edit distance allowed for 2 words to be considered similar
#define SUBSTITUTION_COST 1 //Cost for one operation
const char Dictionary[] = "dictionary.bin";

struct HashTable
{
    char** table;
    int size;
};

//INSERTING INTO THE HASHTABLE
HashTable* insertInHash(FILE* fp)
{
    int size, n;
    fread(&size, sizeof(int), 1, fp); // Size of the HashTable.
    fread(&n, sizeof(int), 1, fp); // Number of elements in the HashTable
    HashTable* hashTable = new HashTable();


    if(hashTable == NULL)
    {
        cout<<"Could not allocate memory.\n";
        exit(0);
    }

    hashTable->size = size;
    hashTable->table = (char**)calloc(size, sizeof(char*));

    if(hashTable->table == NULL)
    {
        cout<<"Could not allocate memory.\n";
        exit(0);
    }

    int i, address, l;
    for(i=0; i<n; i++)
    {
        fread(&address, sizeof(int), 1, fp);
        fread(&l, sizeof(int), 1, fp);
        hashTable->table[address] = (char*)calloc(l, sizeof(char));

        if(hashTable->table[address] == NULL)
        {
            cout<<"Could not allocate memory.\n";
            exit(0);
        }
        fread(hashTable->table[address], sizeof(char), l, fp);
    }
    return hashTable;
}

//FUNCTION TO FIND HASH VALUE OF  A GIVEN WORD
int getHash(HashTable* hashTable, char* key, int length)
{
    const int constant = 31;
    int address = 0;

    for(int i=0; i<length; i++)
        address = (address * constant + (key[i] - 96)) % hashTable->size;
    return address;
}


//FINDING THE KEY IN THE HASH TABLE FOR  A GIVEN WORD
int findKey(HashTable* hashTable, char* key)
{
    int length = strlen(key);
    int address = getHash(hashTable, key, length);
    int count = 0;

    while(hashTable->table[address] != NULL && count < hashTable->size)
    {
        if(strcmp(hashTable->table[address], key) == 0)
            return address;
        address = (++address) % hashTable->size;
        count++;
    }
    return -1;
}



//DELETING MEMORY FOR HASHTABLE AFTER PERFORMING ALL NECESSARY OPERATION
void deleteHashTable(HashTable* hashTable)
{
    for(int i=0; i<hashTable->size; i++)
        delete(hashTable->table[i]);
    delete(hashTable->table);
    delete(hashTable);
}


//FUNCTION TO FIND MINIMUM OF THREEE VALUES
int findMinimum(int x, int y, int z)
{
    return ( (x<y)? (x<z?x:z): (y<z?y:z) );
}


//FUNCTION TO FORM EDIT DISTANCE MATRIX
int** editDistanceMatrix(char* str1, int len1, char* str2, int len2)
{
    // Create edit distance matrix
    int** ed_matrix = new int*[len1+1];
    if(ed_matrix == NULL)
    {
        cout<<"Could not allocate memory.\n";
        exit(0);
    }
    int i,j;
    for(i = 0; i <= len1; i++)
    {
        ed_matrix[i] = new int[len2+1];
        if(ed_matrix[i] == NULL)
        {
            cout<<"Could not allocate memory.\n";
            exit(0);
        }

        // Initializing first column
        ed_matrix[i][0] = i;
    }
    // Initializing first row
    for(j = 0; j<=len2; j++)
        ed_matrix[0][j] = j;

    for(i = 1; i<=len1; i++)
    {
        for(j = 1; j<=len2; j++)
        {
            if(str1[i-1] == str2[j-1])
                ed_matrix[i][j] = ed_matrix[i-1][j-1];
            else
                ed_matrix[i][j] = findMinimum(ed_matrix[i-1][j] + 1, ed_matrix[i][j-1] + 1, ed_matrix[i-1][j-1] + SUBSTITUTION_COST);
        }
    }
    return ed_matrix;
}

//FUNCTION TO FIND EDIT DISTANCE BETWEEN TWO WORDS USING EDIT DISTANCE MATRIX
int editDistance(char* str1, char* str2)
{
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    int** ed_matrix = editDistanceMatrix(str1, len1, str2, len2);

    // Return the bottom-right element
    int val = ed_matrix[len1][len2];
    for(int i = 0;i <= len1; i++)
        delete(ed_matrix[i]);
    delete(ed_matrix);
    return val;
}

//FINDING THE CLOSEST WORD TO THE GIVEN WORD FOR A FIXED EDIT DISTANCE
int closestWord(HashTable* hashTable, char* key, char** answer)
{
    char* word;
    int min, distance;
    int i = 0;

    // Move in the dictionary until a non-null element is found.
    while((word = hashTable->table[i]) == NULL && i < hashTable->size)
        i++;
    // Verify that the dictionary is not empty.
    if(i == hashTable->size)
    {
        *answer = NULL;
        return -1;
    }

    // Select the first element as minimum.
    min = editDistance(word, key);
    *answer = word;
    // Find the minimum
    i++;
    for( ; i<hashTable->size; i++)
    {
        word = hashTable->table[i];
        if(word == NULL) continue;

        distance = editDistance(word, key);

        if(distance < min)
        {
            min = distance;
            *answer = word;
        }
    }
    return min;
}


//FUNCTION TO PRODUCE OPERATIONS DONE TO OBTAIN NEW WORD
char* procedure(char* str1, char* str2)
{

    int len1 = strlen(str1);
    int len2 = strlen(str2);

    int** ed_matrix = editDistanceMatrix(str1, len1, str2, len2);

    int pos = 0;
    char* path = new char[64 * sizeof(char)];

    if(path == NULL)
    {
        cout<<"Could not allocate memory.\n";
        exit(0);
    }

    int i,j;
    i = len1;
    j = len2;
    char table[64]; // 0: Copy 1: Del 2: Insert 3: Replace
    char index = 0;

    // Checking both strings from end
    while(i != 0 && j != 0)
    {
        if(str1[i-1] == str2[j-1])
        { // Characters are equal: Copy
            table[index++] = 0;
            i--;
            j--;
        }
        else
        { // Characters are different
            if(ed_matrix[i-1][j-1] < ed_matrix[i-1][j])
            {
                if(ed_matrix[i-1][j-1] < ed_matrix[i][j-1])
                { // Replace
                    table[index++] = 3;
                    i--;
                    j--;
                }
                else
                { // Delete
                    table[index++] = 1;
                    j--;
                }
            }
            else
            {
                if(ed_matrix[i-1][j] < ed_matrix[i][j-1])
                { // Insert
                    table[index++] = 2;
                    i--;
                }
                else
                { // Delete
                    table[index++] = 1;
                    j--;
                }
            }
        }
    }
    while(i != 0)
    {
        table[index++] = 2;
        i--;
    }
    while(j != 0)
    {
        table[index++] = 1;
        j--;
    }

    // Produce the output
    i = 1;
    for(index -= 1; index>=0; index--)
    {
        switch(table[index])
        {
            case 0:
            {
                //pos += sprintf( path + pos, "%c ", str1[i-1]);
                pos += sprintf( path + pos, "(copy) ");
                i++;
                break;
            }
            case 1:
            {
                pos += sprintf( path + pos, "(delete) ");
                break;
            }
            case 2:
            {
                //pos += sprintf( path + pos, "%c(i) ", str1[i-1]);
                pos += sprintf( path + pos, "(insert) ");
                i++;
                break;
            }
            case 3:
            {
                //pos += sprintf( path + pos, "%c(s) ", str1[i-1]);
                pos += sprintf( path + pos, "(replace) ");
                i++;
                break;
            }
        }
    }

    for(i = 0;i <= len1; i++)
        free(ed_matrix[i]);
    free(ed_matrix);

    return path;

}


//CONVERTING ALL THE LETTERS TO LOWERCASE
void toLower(char string[], int len)
{
    for(int i=0; i<len; i++)
        if(string[i] >= 'A' && string[i] <= 'Z') string[i] += 32;
}


//MAIN FUNCTION
int main()
{
    // CREATING DICTIONARY HASHTABLE
    FILE* fptr = fopen(Dictionary, "rb");
    HashTable* hashedDict;

    if(fptr == NULL)
    {
        // There is no previously hashed dictionary file.
        cout<<"Dictionary file not found\n ";
    }
    else
    {
        // A previously hashed dictionary file is found. Load it into the memory.
        hashedDict = insertInHash(fptr);
        fclose(fptr);
    }

    // PROCESSING USER INPUTS
    int choice;
    do
    {
        system("cls");
        cout<<"\n\n\n\n\t\t\t\t\t\tDICTIONARY LOADED"<<endl;
        cout<<"\t\t\t\t\t     -----------------------"<<endl;
        cout<<"\n\t\t\t\t\t\tEnter a word: ";
        char word[64];
        cin>>word;
        toLower(word, strlen(word));

        if(findKey(hashedDict, word) != -1)
            cout<<"\t\t\t\t\t        Word "<<word<<" is correct\n\n";
        else
        {
            char* answer;
            int dist = closestWord(hashedDict, word, &answer);
            char *path = procedure(answer, word);
            if(dist <= MAX_DIST)
            {
                cout<<"\n\n\t\t\t\t\t       Word "<<word<<" is incorrect"<<endl;
                cout<<"\t\t\t\t\t   The closest word found is: "<<answer<<"\n\t\t\t\t\t      The edit distance is: "<<dist<<endl;
                cout<<"\n\t\t\t\t\t\tWord: ";
                for(int i=0; i<strlen(word); i++)
                    cout<<word[i]<<"  ";
                cout<<"\n\t\t\t\t\t     Closest Word Found: ";
                for(int i=0; i<strlen(answer); i++)
                    cout<<answer[i]<<"  ";
                cout<<"\n\t\t\t\t\tProcedure:  ";
                cout<<path<<endl;
            }
            else
                cout<<"Word "<<word<<" does not exist"<<endl;
        }
        cout<<"\n\t\t\t\t------------------------------------------------------"<<endl;
        cout<<"\n\n\t\t\t\t\tDo you want to continue? ";
        cout<<"1- Yes\t2- No  ";
        cin>>choice;
    }while(choice == 1);

    deleteHashTable(hashedDict);
    cout<<"\n\n\t\t\t\t\t\tDICTIONARY DELETED\n\n\n\n\n";
    return 0;
}
