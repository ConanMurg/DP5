# -*- coding: utf-8 -*-
"""
Created on Mon Jan 23 11:17:03 2023

@author: cm846
"""
import tkinter as tkr

__all__ = ['ParamsWindow']

class ParamsWindow:
    """
    Description: 
        Creates a Tkinter GUI for importing data and identifying
        hot pixels on a CCD detector.
            
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
    def __init__(self, win, nwin, params, b_quit):
        self.params = params
        self.lblvoltage = tkr.Label(win, text='Voltage')
        self.lblcurrent = tkr.Label(win, text='Current')
        self.lblpret = tkr.Label(win, text='Preset Time')
        
        # Entry boxes
        # self.text_sigma = tkr.Entry(  win, bd=3, width = 20) # Sigma
        # self.text_offset = tkr.Entry( win, bd=3, width = 20) # Offset
        self.text_voltage = tkr.Entry( win, bd=3, width = 20) # Primary Threshold
        self.text_current = tkr.Entry(win, bd=3, width = 20) # Secondary Threshold
        self.text_pret = tkr.Entry(win, bd=3, width = 20) # Sigma
        
        self.button_defaults = tkr.Button(win, text='Restore Default Values', width = 20, command = lambda: self.defaults(win))
        self.button_params = tkr.Button(win, text='Update Parameters', width = 20, command = lambda: self.update_params(win))
       
        
        self.lblvoltage.grid(row = 1, column = 1, padx=5, pady=5)
        self.lblcurrent.grid(row = 2, column = 1, padx=5, pady=5)
        self.lblpret.grid(row = 3, column = 1, padx=5, pady=5)
        
        self.text_voltage.grid(row = 1, column = 2, padx=5, pady=5)
        self.text_current.grid(row = 2, column = 2, padx=5, pady=5)
        
        self.text_pret.grid(row = 4, column = 2, padx=5, pady=5)
        
        self.button_params.grid(row = 14, column = 1, padx=5, pady=5)
        self.button_defaults.grid(row = 15, column = 1, padx=5, pady=5)
        b_quit.grid(row = 15, column = 2, padx=5, pady=5)

        self.update_display()


    def win_quit(self, win, nwin):
        """
        Closes the GUI window, and reopens the main GUI.

        Parameters
        ----------
        win : Tkinter Frame
            The Tkinter frame in which the GUI is running.
            
        nwin : Tkinter Frame.
            The main Tkinter frame which created this frame.

        Returns
        -------
        None.

        """    
        # nwin.deiconify()
        win.destroy()


    def defaults(self, win):
        self.params.voltage = 15
        self.params.current = 15
        self.params.pret = 20
        self.update_display()


    def update_display(self):
        self.text_voltage.delete(0, tkr.END)
        self.text_current.delete(0, tkr.END)
        self.text_pret.delete(0, tkr.END)
        
        self.text_voltage.insert(0, self.params.voltage)
        self.text_current.insert(0, self.params.current)
        self.text_pret.insert(0, self.params.pret)
        

    def update_params(self, win):
        """
        Stuff

        Parameters
        ----------
        win : TYPE
            DESCRIPTION.

        Returns
        -------
        None.

        """
        txt_voltage = str(self.text_voltage.get())
        if txt_voltage.isdigit():
            self.params.voltage = int(txt_voltage)

        txt_current = str(self.text_current.get())
        if txt_current.isdigit():
            self.params.current = int(txt_current)

        txt_pret = str(self.text_pret.get())
        if txt_pret.isdigit():
            self.params.pret = int(txt_pret)
        
        
        self.update_display()
