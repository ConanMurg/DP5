import ctypes
import time


# Load the shared library
mydll = ctypes.CDLL('./libgccDppConsoleMiniX2.so')

mydll.ConnectToDefaultDPP.restype = ctypes.c_bool
mydll.AcquireSpectrum.restype = ctypes.POINTER(ctypes.c_long)


result = mydll.ConnectToDefaultDPP()

print(result)

if result:
    test = mydll.GetDppStatus()
    print(test)
    if test:

        print('Req Config')
        mydll.ReadDppConfigurationFromHardware(ctypes.c_bool(True))

        res = mydll.ResetDevice()
        if res:
            for _ in range(10):

                data_ptr = mydll.AcquireSpectrum()
                data_list = [data_ptr[i] for i in range(2048)]
                mydll.free_memory(data_ptr)

                print(f'Data: {data_list}')
                time.sleep(1)
            dis = mydll.DisableMCA()
            if dis:
                print('finished')


#print("data:", list(spectrum_data.data[:spectrum_data.channels]))



# mydll = ctypes.CDLL('./libmydll.so')


# Define the function prototype
# mydll.add.argtypes = (ctypes.c_int, ctypes.c_int)
# mydll.add.restype = ctypes.c_int

# # Call the function
#result = mydll.ConnectToDefaultDPP()
# result = mydll.add(3, 4)
# print(f"The result of add(3, 4) is {result}")
