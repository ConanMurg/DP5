import ctypes

# Load the shared library
mydll = ctypes.CDLL('./libgccDppConsoleDP5.so')


result = mydll.ConnectToDefaultDPP()
test = mydll.GetDppStatus()



# Set the return type and argument types of the function
mydll.getTestData.restype = ctypes.POINTER(ctypes.c_int)
mydll.getTestData.argtypes = [ctypes.POINTER(ctypes.c_int)]

# Create an integer to hold the size of the array
size = ctypes.c_int()

# Call the function
data_ptr = mydll.getTestData(ctypes.byref(size))

# Convert the returned C array to a Python list
data_list = [data_ptr[i] for i in range(size.value)]
print("Data received from C++:", data_list)




# mydll = ctypes.CDLL('./libmydll.so')


# Define the function prototype
# mydll.add.argtypes = (ctypes.c_int, ctypes.c_int)
# mydll.add.restype = ctypes.c_int

# # Call the function
#result = mydll.ConnectToDefaultDPP()
# result = mydll.add(3, 4)
# print(f"The result of add(3, 4) is {result}")