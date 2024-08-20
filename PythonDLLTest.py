import ctypes
import time
from matplotlib import pyplot as plt

# Load the shared library
try:
    mydll = ctypes.CDLL('./libgccDppConsoleDP5.so')
except:
    print("Can't open dll")
else:
    # Configure functions
    mydll.ConnectToDefaultDPP.restype = ctypes.c_bool
    mydll.AcquireSpectrum.restype = ctypes.POINTER(ctypes.c_long)

    # Connect to device
    result = mydll.ConnectToDefaultDPP()

    # If connected, get device status.
    if result:

        status = mydll.GetDppStatus()

        # If status returned, read DP5 Configuration
        if status:

            mydll.ReadDppConfigurationFromHardware(ctypes.c_bool(True))

            # Before acquiring spectra, must reset device (disable device, clear old spectra, then re-enable MCA) 
            reset = mydll.ResetDevice()

            # If resetting device successful, begin acquiring spectra.
            if reset:
                fig = plt.figure()
                ax = fig.add_subplot(111)
                x = np.arange(0, 2048, 1)
                y = np.zeros(2048)
                line1, = ax.plot(x, y, 'r-')

                # Begin a loop 
                for _ in range(10):
                    data_ptr = mydll.AcquireSpectrum()
                    data_list = [data_ptr[i] for i in range(2048)]
                    mydll.free_memory(data_ptr)

                    print(f'Data: {data_list}')
                    time.sleep(1)

                dis = mydll.DisableMCA()
                if dis:
                    print('finished')
    else:
        mydll.CloseConnection()
