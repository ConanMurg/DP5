#include <iostream>

extern "C" {  // Ensure the function names are not mangled

int* getTestData(int* size) {
    static int data[5] = {1, 2, 3, 4, 5};  // Static to ensure it remains in scope
    *size = 5;  // Set the size of the array
    return data;  // Return the static array
}

}