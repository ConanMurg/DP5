# -*- coding: utf-8 -*-
"""
Created on Tue Jun 20 15:10:01 2023

@author: cm846
"""
import logging
import time
import datetime
import numpy as np
import ctypes


def dp5_analysis(mydll, queue_time, queue_data, event, pret):
    """
    Imports raw files from a directory, subtracts a dark frame and offset
    from each frame. Identifies all pixels above a threshold value, then
    identifies if they are single pixel or multipixel events. Stores all
    single pixel events (coordinate, value, and file) into an HDF5 file
    and all multipixel events (coordinate, values, event number, and file)
    into another HDF5 file.


    Parameters
    ----------
    win : tkinter frame.
        The Tkinter frame which the GUI is running in.

    Returns
    -------
    error
        int. Returns 1 if an error has occured, 0 otherwise.

    """
    try:
        early, percent, stop = 0, 0.00, 0
        zero = str(datetime.timedelta(seconds=0))
        elaps = zero
        queue_time.put([elaps, zero, percent, 0, 0, stop])
        spectrum_channels, spectrum_data= [], []

        prev = total = time.time()

        reset = mydll.ResetDevice()

        # If resetting device successful, begin acquiring spectra.
        if not reset:
            raise Exception()

        for dp5_time in range(pret):
            if time.time() - prev > 1: # Update progress bar every second
                prev = time.time()
                time_c = time.time() - total
                elaps = str(datetime.timedelta(seconds = int(time_c)))
                
                time_r = pret - time_c
                remain = str(datetime.timedelta(seconds =  int(time_r)))
                
                percent = np.around((time_c) / pret * 100, 2)
                
                queue_time.put([elaps, remain, percent, spectrum_channels, spectrum_data, 0])

            if event.is_set():
                early = 1
                break
            
            # Begin a loop 
            data_ptr = mydll.AcquireSpectrum()
            spectrum_data = [data_ptr[i] for i in range(2048)]
            mydll.free_memory(data_ptr)

            # print(f'Data: {data_list}')

            queue_data.put(np.arange(0,2048, 1).tolist(), spectrum_data)

            if early:
                break
            
            time.sleep(1)

    except FileNotFoundError("File in filenames not found"):
        queue_time.put([elaps, zero, 0, spectrum_channels, spectrum_data, 1])

    except RuntimeError("Unspecified run-time error"):
        queue_time.put([elaps, zero, 0, spectrum_channels, spectrum_data, 1])

    except TypeError("Type Error"):
        queue_time.put([elaps, zero, 0, spectrum_channels, spectrum_data, 1])

    except Exception("General Exception") as err:
        logging.exception(err)
        queue_time.put([elaps, zero, 0, spectrum_channels, spectrum_data, 1])


    data_ptr = mydll.AcquireSpectrum()
    data_list = [data_ptr[i] for i in range(2048)]
    mydll.free_memory(data_ptr)

    print(f'Data: {data_list}')

    queue_data.put(spectrum_channels, spectrum_data)

    dis = mydll.DisableMCA()
    if dis:
        print('finished')

    # print(graded)
    time_c = time.time() - total
    elaps = str(datetime.timedelta(seconds = int(time_c)))
    
    time_r = pret - time_c

    remain = str(datetime.timedelta(seconds=  int(time_r)))
    percent = np.around((time_c) / pret * 100, 2)
    
    try:
        if early:
            queue_time.put([elaps, zero, percent, spectrum_channels, spectrum_data, 1]) # Set stop to 1

        if not early:
            # print(f"Number of skips {l}")
            queue_time.put([elaps, zero, 100, spectrum_channels, spectrum_data, 1]) # Set stop to 1, and progressbar to 100

    except Exception as err:
        logging.exception(err)
        print("Unspecified Error")

