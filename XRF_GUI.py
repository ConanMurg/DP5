# -*- coding: utf-8 -*-
"""
Created on Tue Nov 15 10:11:49 2022
Python Code for Handling Detector Data
University of Leicester
School of Physics and Astronomy

@author: cm846

Requires installation of a few modules:
 - mttkinter
 - glob
 - natsorted
"""
from mttkinter import mtTkinter as tkr
import logging
from tkinter import ttk
from tkinter.filedialog import askopenfilename as askf
from tkinter.filedialog import asksaveasfilename as askfs
from tkinter.filedialog import askdirectory as askd
import os
import csv
import json
import glob
from threading import Thread, Event
from functools import partial
from queue import Queue
from natsort import natsorted
from scipy.stats import norm
import numpy as np
from matplotlib.figure import (Figure, SubplotParams)
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
from matplotlib.backend_bases import key_press_handler
import matplotlib.ticker as ticker
from tifffile import imread
from CAS_ParameterGUI import ParamsWindow ######
from DP5_Analysis import dp5_analysis 
import ctypes

__all__ = ['Parameters', 'MyWindow', 'Startup']

class Parameters:
    """
    Description:
        Stores parameters from a config file. Uses setters and getters to ensure
        that the config file contains the correct data types and values for
        every variable.

    Params: vals.
        vals = [rows, cols, threshold, sec_threshold, searchgrid, dir_df,
                dir_hp, dir_save]

    E.g.
        rows, cols = int.
        threshold, sec_threshold = int or float.
        searchgrid = list of 4 integers.
        dir_df, dir_hp, dir_save = string

        vals = [2040, 2048, 90, 20, [460, 1555, 0, 935], "C:/Users/cm846/dark.raw",
                "C:/Users/cm846/hp.raw", "C:/Users/cm846/"]
    """
    def __init__(self, vals):
        self.voltage = vals[0]
        self.current = vals[1]
        self.pret = vals[2]
        
        
    @property
    def voltage(self):
        """
        Returns voltage
        """
        return self.__voltage

    @voltage.setter
    def voltage(self, var):
        """
        Setter for voltage. Ensures new value is positive and an integer.
        """
        if var > 0 and isinstance(var, int):
            self.__voltage= var

    @property
    def current(self):
        """
        Returns current
        """
        return self.__current

    @current.setter
    def current(self, var):
        """
        Setter for current. Ensures new value is positive and an integer.
        """
        if var > 0 and isinstance(var, int):
            self.__current = var

    @property
    def pret(self):
        """
        Returns pret value.
        """
        return self.__pret

    @pret.setter
    def pret(self, value):
        """
        Setter for pret. Ensures value is numeric and positive.
        """
        if isinstance(value, (int, float)):
            if value > 0:
                self.__pret = value


class MyWindow:
    """
    Description:
        Creates a Tkinter GUI for importing data and creating a
        dark frame image for a CCD detector.

    Params: win.
        The Tkinter window which the following buttons and layout will populate.

    E.g.
        <<< import tkinter as tkr
        <<< window = tkr.Tk()
        <<< mywin = MyWindow(window)
        <<< window.title('Example Code')
        <<< window.geometry("750x750+10+10")
        <<< window.mainloop()

    """
    def __init__(self, win):
        try:
            values = self.defaults()
            self.params = Parameters(values)

        except IndexError as i_err:
            print("IndexError: Number of Values in Config are wrong. Program aborting.\n")
            win.destroy()
            raise SystemExit(0) from i_err

        except TypeError as t_err:
            print("TypeError: Values in Config are wrong. Program aborting.\n")
            win.destroy()
            raise SystemExit(0) from t_err

        except ValueError as v_err:
            print("Exception: threshold must be larger than sec_threshold. Program aborting.\n")
            win.destroy()
            raise SystemExit(0) from v_err
        
        try:
            self.mydll = ctypes.CDLL('./libgccDppConsoleDP5.so')
        except:
            print("Can't open dll")
            # raise SystemExit(0)

        # Configure functions
        # self.mydll.ConnectToDefaultDPP.restype = ctypes.c_bool
        # self.mydll.AcquireSpectrum.restype = ctypes.POINTER(ctypes.c_long)

        # Connect to device
        # result = self.mydll.ConnectToDefaultDPP()
        result = True
        if not result:
            raise SystemExit(0)
    
        self.plot_select = True
        self.stop_plot = False
        
        self.queue_time = Queue()
        self.queue_data = Queue()
        # self.event_MX2 = Event()
        self.event_DP5 = Event()
        
        self.spectrum_channels = []
        self.spectrum_data = []
        
        self.after_id_DP5 = None
        self.save_hist_param = False
        
        # Create buttons
        self.button_create_hp = tkr.Button(win,  bg='darkseagreen')
        self.button_create_hp.config(text='Create Hot Pixel Map', width=20)      # Hot Pixel Map

        self.button_create_df = tkr.Button(win,  bg='darkseagreen')
        self.button_create_df.config(text='Create Dark Frame', width=20)         # Dark Frame

        self.button_hist_save = tkr.Button(win, command = self.change_save, bg='lightsteelblue')
        self.button_hist_save.config(text='Enable Save Histogram', width=20)     # Enable Save Histogram

        self.button_datas = tkr.Button(win,command = lambda: self.check_params_DP5(win))
        self.button_datas.config(text='Start Acquisition', width=20)           # Start analysis

        self.button_quit = tkr.Button(win, command = lambda: self.confirm_exit(win), bg='lightcoral')
        self.button_quit.config(text = 'Quit', width = 20)                       # Quit

        self.button_params = tkr.Button(win, command = lambda: self.params_gui(win, self.params))
        self.button_params.config(text='Update Parameters', width = 20)          # Update Params


        lbl1 = tkr.Label(win, text='Voltage (kV)')
        lbl2 = tkr.Label(win, text='Current (uA)')
        lbl3 = tkr.Label(win, text='Preset Time (s)')
        
        self.text_pret = tkr.Entry(win, bd=3, width = 20) # Search Grid
        self.text_pret.insert(0, self.params.pret)
        self.text_pret.config(state='readonly')
        
        self.text_voltage = tkr.Entry(win, bd=3, width = 20)     # Primary Threshold
        self.text_voltage.insert(0, self.params.voltage)
        self.text_voltage.config(state='readonly')
        
        self.text_current = tkr.Entry(win, bd=3, width = 20)    # Secondary Threshold
        self.text_current.insert(0, self.params.current)
        self.text_current.config(state='readonly')

    
        # Grid Layout
        self.button_params.grid(row=1, column=0, padx=10, pady=5)
        self.button_datas.grid(row=2, column=0, padx=10, pady=5)
        self.button_quit.grid(row=3, column=0, padx=10, pady=5)
        
        lbl1.grid(row=1, column=1, padx=10, pady=5)
        self.text_voltage.grid(row=1, column=2, padx=10, pady=5)
        
        lbl2.grid(row=2, column=1, padx=10, pady=5)
        self.text_current.grid(row=2, column=2, padx=10, pady=5)
        
        lbl3.grid(row=3, column=1, padx=10, pady=5)
        self.text_pret.grid(row=3, column=2, padx=10, pady=5)
    
        self.my_progress = ttk.Progressbar(win, orient=tkr.HORIZONTAL, length=380, mode='determinate')
        self.my_progress.grid(row=4, column=0, columnspan=3, padx=20, pady=5)
    
        # Frame for time displays
        self.time_frame = tkr.Frame(win)
        self.time_frame.grid(row=5, column=0, columnspan=3, padx=20, pady=5)
        self.elapsedtime = tkr.Text(self.time_frame, width=15, height=1, state='disabled')
        self.elapsedtime.pack(side="left")
        self.remainingtime = tkr.Text(self.time_frame, width=15, height=1, state='disabled')
        self.remainingtime.pack(side="right")
    
        # Create interactive figure with a toolbar inside Frame
        image_frame = tkr.Frame(win)
        image_frame.grid(row=0, column=3, rowspan=8, padx=20, pady=5, sticky="nsew")
        
        bottom = tkr.Frame(image_frame)
        bottom.pack(side=tkr.BOTTOM, fill=tkr.BOTH, expand=tkr.TRUE)
        
        plot_params = SubplotParams(left=0.1, bottom=0.1, right=0.95, top=0.95, wspace=0.05, hspace=0.05)
        self.fig = Figure(figsize=(9, 9), dpi=72, subplotpars=plot_params)
        self.canvas = FigureCanvasTkAgg(master=image_frame, figure=self.fig)
        self.canvas.draw()
    
        self.toolbar = NavigationToolbar2Tk(window=image_frame, canvas=self.canvas)
        self.toolbar.update()
        self.toolbar.pack(in_=bottom, side=tkr.LEFT)
        
        button_ch = tkr.Button(image_frame, command=self.clear_pic)
        button_ch.config(text='Clear', width=5)
        button_ch.pack(in_=bottom, side=tkr.RIGHT)
        
        self.button_stop_plot = tkr.Button(win, command=self.stop_plotting)
        self.button_stop_plot.config(text='Lock', width=5)
        self.button_stop_plot.pack(in_=bottom, side=tkr.RIGHT, padx=1)
    
        self.button_sum = tkr.Button(win, command=self.data_sum)
        self.button_sum.config(text='Sum', width=5)
        self.button_sum.pack(in_=bottom, side=tkr.RIGHT, padx=1)
    
        self.canvas.get_tk_widget().pack(side=tkr.BOTTOM, fill=tkr.BOTH, expand=tkr.TRUE)
    
        def on_key_press(event):
            key_press_handler(event, self.canvas, self.toolbar)
        self.canvas.mpl_connect("key_press_event", on_key_press)
    
        # Enable Buttons button
        button_enable = tkr.Button(win, command=self.enable_buttons)
        button_enable.config(width=20, text="Force Enable Buttons")
        button_enable.grid(row=6, column=0, padx=10, pady=5)
    
        # Switch Graph button
        button_calc_stop = tkr.Button(win, command=self.stop_calc_DP5, bg='mistyrose')
        button_calc_stop.config(width=20, text="Stop Acquisition")
        button_calc_stop.grid(row=6, column=1, padx=10, pady=5)
    
        # Console Display
        self.console = tkr.Text(win, width=38, height=3, state='disabled')
        self.console.grid(row=7, column=0, columnspan=3, padx=20, pady=5, sticky="nsew")
        
        self.plot_line(np.arange(0, 2048, 1), np.zeros(2048))

        win.protocol('WM_DELETE_WINDOW', lambda: self.confirm_exit(win))  # GUI exit protocol


    def change_save(self):
        self.save_hist_param = not self.save_hist_param
        print(self.save_hist_param)


    def data_sum(self):
        """
        Find the number of counts within the range shown on image pane.

        Returns
        -------
        None.

        """
        if not self.event_DP5.is_set():
            return

        p = self.fig.gca()
        xmin, xmax = p.get_xlim()
        xmin = np.around(xmin, 0)
        xmax = np.around(xmax, 0)
        
        e = np.array(self.spectrum_data)
        
        e = e[e > xmin]
        e = e[e < xmax]
        # self.update_console(f'Number of events in window: {len(e)}.')
        self.update_console(f' - Range: {xmin} - {xmax} ADU')
        self.update_console(f'Number of events in window: {len(e)}.')



    def save_data(self):
        """
        Saves the data into a csv.

        Returns
        -------
        None.

        """
        if len(self.spectrum_data) == 0:
            self.update_console("Collect some data first!")
            return
        
        print('TBC')
        return 0
    
        # initdir = os.path.dirname(os.path.realpath(__file__))
        # try:
        #     f = askfs(filetypes=[("csv", ".csv")], defaultextension=".csv",
        #                                  initialdir=initdir)
        #     with open(f, "w", newline="", encoding='UTF-8') as f:
        #         writer = csv.writer(f)
        #         writer.writerows(self.parameters)
        #         writer.writerows(self.data_mp)
        #         writer.writerows(self.data_sp)

        # except Exception as err:
        #     logging.exception(err)
        #     self.update_console("Failed to save data")
        #     print("Failed to save data")


    def load_prev_data(self):
        """
        Loads in a csv file containing all the data from previous analysis

        Raises
        ------
        ValueError
            DESCRIPTION.

        Returns
        -------
        None.

        """
        print('TBC')
        return 0

        # initdir = os.path.dirname(os.path.realpath(__file__))
        # try:
        #     filename = askf(initialdir=initdir, defaultextension='.npy',
        #                     filetypes=[('csv','*.csv'), ('All files','*.*')])
        #     with open(filename, encoding='UTF-8') as file:
        #         reader = csv.reader(file)
        #         states = [[item for item in row if item != ''] for row in reader]

        #     if len(states[0]) != 5:
        #         print("Here")
        #         print(states)
        #         raise ValueError

        #     params = [json.loads(item) for item in states[0]]
        #     dir_dark, dir_hp, dir_filenames = states[1:4]
        #     parameters = [params, dir_dark, dir_hp, dir_filenames]

        #     hot_pixels = np.load(dir_hp[0]).tolist()
        #     rows, cols, searchgrid = params[0:3]

        #     # EDITS REQUIRED
        #     if os.path.splitext(os.path.basename(dir_dark[0]))[1] == '.tiff':
        #         filenames = natsorted(glob.glob(os.path.join(dir_filenames[0], '*.tiff')))
        #         dark = imread(dir_dark[0])
        #         dark = dark[::self.params.scale, ::self.params.scale]
        #         self.is_tiff_file = True

        #     else:
        #         filenames = natsorted(glob.glob(os.path.join(dir_filenames[0], '*.raw')))
        #         dark = np.fromfile(dir_dark[0], dtype='>u2')

        #     states[4:] = [[json.loads(item) for item in row] for row in states[4:]]

        #     data_mp = states[4:int(len(states)/2+2)]
        #     data_sp = states[int(len(states)/2+2):int(len(states))]

        #     energy_mp = []
        #     for i in data_mp:
        #         for k in i:
        #             energy_mp.append(sum(k[1]))

        #     energy_sp = []
        #     for i in data_sp:
        #         for k in i:
        #             energy_sp.append(k[1])

        #     parameter = [rows, cols, searchgrid] + [hot_pixels]

        # except Exception as err:
        #     print("Fail")
        #     logging.exception(err)
        #     data_sp = []
        #     data_mp = []
        #     energy_sp = None
        #     energy_mp = None
        #     filenames = None
        #     params = None
        #     self.update_console("Failed to load previous data!")

        # else:
        #     self.params.dark = dark
        #     self.params.dir_df = dir_dark[0]
        #     # os.path.dirname(os.path.abspath(dir_dark[0])) # CHANGE TO JUST DIRECTORY

        #     self.params.hot_pixels = hot_pixels
        #     self.params.dir_hp = dir_hp[0]

        #     self.file_view = partial(self.data_viewer, files=filenames,
        #                              params=[rows,cols,searchgrid,hot_pixels])
        #     self.params.dir_filenames = dir_filenames[0]
        #     self.params.element = os.path.basename(os.path.dirname(filenames[0]))

        #     self.params.threshold = params[3]
        #     self.params.sec_threshold = params[4]
        #     self.params.searchgrid = searchgrid

        #     self.parameters = parameters

        #     self.data_sp = data_sp
        #     self.data_mp = data_mp
        #     self.energy_sp = energy_sp
        #     self.energy_mp = energy_mp
        #     self.update_params()

        #     self.event.set()
        #     self.file_view = partial(self.data_viewer, files=filenames, params=parameter)
        #     self.switch_hist()


    def stop_plotting(self):
        """
        Locks the display

        Returns
        -------
        None.

        """
        self.stop_plot = not self.stop_plot # Switch from locked/unlocked to unlocked/locked

        if self.stop_plot:
            self.button_stop_plot.config(text='Unlock')
            self.update_console("Image is pinned to the display")
        else:
            self.button_stop_plot.config(text='Lock')
            self.update_console("Display is now unlocked")


    # def stop_calc_MX2(self):
    #     """
    #     Stops the calculation running in another thread.

    #     Returns
    #     -------
    #     None.

    #     """
    #     try:
    #         if not self.event_MX2.is_set():
    #             self.event_MX2.set()
    #             self.worker_MX2.join()

    #     except AttributeError: # Will occur if threading event has not run
    #         pass


    def stop_calc_DP5(self):
        """
        Stops the calculation running in another thread.

        Returns
        -------
        None.

        """
        print('Stopping')
        try:
            if not self.event_DP5.is_set():
                self.event_DP5.set()
                self.worker_DP5.join()

        except AttributeError: # Will occur if threading event has not run
            print('Error Stopping')
            pass




    def confirm_exit(self, win):
        """
        Saves parameters into config file for next run of software.

        """
        try:
            self.event_DP5.set()
            # self.event_MX2.set()
            self.worker_DP5.join()
            # self.worker_MX2.join()

        except AttributeError: # Will occur if threading event has not run
            pass

        # try:
        #     self.update_console("Saving parameters")
        #     values = []
        #     for _ in self.params.__dict__.values():
        #         values += [_]

        #     with open("config", "w", encoding='UTF-8') as config:
        #         json.dump(values[0:8], config)

        # except Exception as err:
        #     logging.exception(err)
        #     self.update_console("Failed to save parameters")
        #     print("Failed to save parameters")

        # else:
        #     self.update_console("Parameters successfully saved")

        try:
            self.update_console("Quitting")
            for after_id in win.tk.eval('after info').split(): # Ensures after callback cancelled
                win.after_cancel(after_id)

            # win.quit()
            win.destroy()
            #raise SystemExit(0)


        except Exception as exc:
            self.update_console("Error Quitting Program")
            logging.exception(exc)
            raise SystemExit(0) from exc


    def update_console(self, text):
        """
        Print text to the console on the GUI.

        Parameters
        ----------
        text : TYPE
            DESCRIPTION.

        Returns
        -------
        None.

        """
        self.console["state"] = 'normal'    # Elapsed Time
        self.console.insert(tkr.END, '\n' + text)
        self.console.see(tkr.END)
        self.console["state"] = 'disabled'


    def update_params(self):
        """
        Update all the parameters from the textboxes in the GUI.

        Returns
        -------
        None.

        """
        # self.update_console("Updated!")
        self.text_voltage.config(state='normal')
        self.text_current.config(state='normal')
        self.text_pret.config(state='normal')

        self.text_voltage.delete(0, tkr.END)
        self.text_current.delete(0, tkr.END)
        self.text_pret.delete(0, tkr.END)
        
        
        self.text_voltage.insert(0, self.params.voltage)
        self.text_current.insert(0, self.params.current)
        self.text_pret.insert(0, self.params.pret)

        self.text_voltage.config(state='readonly')
        self.text_current.config(state='readonly')
        self.text_pret.config(state='readonly')


    def defaults(self):
        """
        Returns default values for config file.
        """
        values = [15, 15, 20]
        return values


    def disable_toolbar(self):
        """
        Disables the toolbar under the image pane on the GUI

        Returns
        -------
        None.

        """
        toolitems = list(map(list, zip(*self.toolbar.toolitems)))[0] # Find button names on toolbar
        toolitems = [_ for _ in toolitems if _ is not None] # Remove any Nonetypes

        for i in toolitems:
            self.toolbar._buttons[i]["state"] = tkr.DISABLED


    def disable_buttons(self):
        """
        Disables the buttons on the GUI.

        Returns
        -------
        None.

        """
        buttons = [self.button_load_df, self.button_load_hp, self.button_data,
                   self.button_datas, self.button_quit, self.button_params]

        for i in buttons:
            i["state"] = tkr.DISABLED

        self.disable_toolbar()


    def enable_toolbar(self):
        """
        Enable the toolbar under the image pane on the GUI

        Returns
        -------
        None.

        """
        toolitems = list(map(list, zip(*self.toolbar.toolitems)))[0] # Find button names on toolbar
        toolitems = [_ for _ in toolitems if _ is not None] # Remove any Nonetypes

        for i in toolitems:
            self.toolbar._buttons[i]["state"] = tkr.NORMAL


    def enable_buttons(self):
        """
        Enables the buttons on the GUI.

        Returns
        -------
        None.

        """
        buttons = [self.button_load_df, self.button_load_hp, self.button_data,
                   self.button_datas, self.button_quit, self.button_params]

        for i in buttons:
            i["state"] = tkr.NORMAL

        self.enable_toolbar()


    # def check_params_MiniX2(self, win):
    #     """
    #     Checks that dark frame has been imported and that there are files in
    #     the selected directory before beginning x-ray analysis.

    #     """
    #     self.update_console("\n" + "Starting Calculation" + "\n")
        #     self.disable_buttons()
    #     self.event.clear()

    #     voltage = self.params.voltage # Make a subset
    #     current = self.params.current
        
    #     parameters = [voltage, current]
        
    #     self.worker_MX2=Thread(target=xrf_analysis, args=(filenames, self.queue_time, self.queue_data, self.event, parameters))
    #     self.worker_MX2.start()


    def check_params_DP5(self, win):
        """
        Checks that dark frame has been imported and that there are files in
        the selected directory before beginning x-ray analysis.

        """
        self.update_console("\n" + "Starting Calculation" + "\n")
        # self.disable_buttons()
        self.event_DP5.clear()

        pret = self.params.pret
        
        reset = self.mydll.ResetDevice()

            # If resetting device successful, begin acquiring spectra.
        if not reset:
            print('Failed to reset')
            return 0
        
        # fig = plt.figure()
        # ax = fig.add_subplot(111)
        # x = np.arange(0, 2048, 1)
        # y = np.zeros(2048)
        # line1, = ax.plot(x, y, 'r-')

        # Begin a loop 
        
        self.worker_DP5=Thread(target=dp5_analysis, args=(self.mydll, self.queue_time, self.queue_data, self.event_DP5, pret))
        self.worker_DP5.start()
        self.after_id_DP5 = None
        self.time_frame.after(1, self.update_time_display)


    def update_time_display(self):
        """
        Updates Tkinter time elapsed, time remaining, and progress bar as the
        x-ray finder calculation runs in another thread.

        Returns
        -------
        None.

        """
        stop = 0
        if not self.queue_time.empty():
            elaps, remain, percent, spectrum_channels, spectrum_data, stop = self.queue_time.get() # Get values from Queue()

            self.elapsedtime["state"] = 'normal'    # Elapsed Time
            self.elapsedtime.delete('1.0',   tkr.END)
            self.elapsedtime.insert('1.0',   elaps)
            self.elapsedtime["state"] = 'disabled'

            self.remainingtime["state"] = 'normal'  # Remaining Time
            self.remainingtime.delete('1.0', tkr.END)
            self.remainingtime.insert('1.0', remain)
            self.remainingtime["state"] = 'disabled'

            self.my_progress['value'] = percent     # Percentage Bar

            if not self.stop_plot:
                self.disable_toolbar()

                string = ('DP5 Histogram')
                self.update_line(spectrum_channels, spectrum_data, string=string)

            else:
                self.enable_toolbar()

            if stop:
                self.event_DP5.set()
                self.time_frame.after_cancel(self.after_id_DP5)
                # self.enable_buttons()
                self.spectrum_channels = spectrum_channels
                self.spectrum_data = spectrum_data


                while not self.queue_data.empty():
                    spectrum_data = self.queue_data.get()

                if not self.stop_plot:
                    if len(spectrum_data) != 0:
                        string = ('Histogram')
                        self.plot_line(spectrum_data, nr_bins=1, string=string)


        if not stop:
            self.after_id_DP5 = self.time_frame.after(500, self.update_time_display)



    def clear_pic(self):
        """
        Update frame to show blank picture.
        """
        if self.stop_plot:
            return

        self.fig.clear()    # Clear the figure
        self.canvas.draw()  # Update the canvas



    def plot_line(self, channels, data):
        """
        Plot a line graph

        Parameters
        ----------
        arr : TYPE
            DESCRIPTION.
        nr_bins : TYPE
            DESCRIPTION.
        string : TYPE
            DESCRIPTION.

        Returns
        -------
        None.

        """
        self.fig.clear()    # Clear previous figure
        ax = self.fig.gca()  # Find the subplot axes
        ax.grid(True)        # Overlay grid

        # Plot new data
        self.line_DP5, = ax.plot(channels, data)
        
        # Set ylimit as minimum 100, otherwise 10% higher than max data value
        ymax = max(100, np.amax(data)*1.1)
        ax.set_ylim([0, ymax])
        ax.set_xlim([0, 2048])
        
        # Add axis labels
        self.fig.supxlabel("Channel")
        self.fig.supylabel("Frequency (Counts)")
        self.canvas.draw()  # Update the canvas


    def update_line(self, channels, data):
        ax = self.fig.gca()
        
        # Plot new data
        self.line_DP5.set_ydata(data)
        
        # Set ylimit as minimum 100, otherwise 10% higher than max data value
        ymax = max(100, np.amax(data)*1.1)
        ax.set_ylim([0, ymax])




    def params_gui(self, win, params):
        """
        Opens a new Tkinter GUI to create a hot pixel map

        """
        # win.withdraw()
        self.params_window = tkr.Toplevel() # Create New Tkinter Interface Frame (TopLevel())
        def confirm_exit():
            self.params_window.destroy()
            self.update_params()

        button_quit = tkr.Button(self.params_window, text='Quit', width=20, command=confirm_exit)
        self.params_win = ParamsWindow(self.params_window, win, params, button_quit)  # GUI from paramsgui.py
        self.params_window.title('Parameter Editor')  # Select GUI name
        self.params_window.geometry("750x750+10+10") # Choose window geometry settings
        self.params_window.protocol('WM_DELETE_WINDOW', confirm_exit)


class Startup:
    """
    Creates Main GUI window
    """
    def __init__(self):
        self.window = tkr.Tk() # Create Tkinter Frame
        self.mywin = MyWindow(self.window) # Use MyWindow GUI Settings from GUI.py
        self.window.title('X-Ray Fluorescence Detector Software') # Select GUI name
        # self.window.iconbitmap('UoL.ico')
        self.window.geometry("1550x1000+10+10") # Choose Window Geometry
        self.window.mainloop()


if __name__ == '__main__':
    start = Startup()
    
    x = start.mywin.spectrum_channels
    y = start.mywin.spectrum_data
