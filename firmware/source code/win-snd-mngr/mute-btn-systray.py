#!/usr/bin/env python3

import tkinter as tk
from tkinter import scrolledtext
import pystray
from debug import dbgPrint, dbgAddCallback, dbgRemoveCallback
from PIL import Image, ImageTk
import base64
import time
import threading
import queue

import muteBtnMngr
import icons


APP_VERSION = '1.0.0'



class Front():

    OrderStandBy = 0
    OrderHideWindow = 100
    OrderShowWindow = 101
    

    def __init__(self, eventCallback) -> None:
        self._eventCallback = eventCallback

        # window creation
        self.win = tk.Tk()

        # pctures stuff
        binIco16 = base64.b64decode(icons.b64Ico16)
        binIco32 = base64.b64decode(icons.b64Ico32)
        binIco64 = base64.b64decode(icons.b64Ico64)

        binHeadset32Gray = base64.b64decode(icons.b64Headset32Gray)
        binHeadset32Green = base64.b64decode(icons.b64Headset32Green)
        binHeadset32Red = base64.b64decode(icons.b64Headset32Red)

        binButtonGreen = base64.b64decode(icons.b64ButtonGreen)
        binButtonRed = base64.b64decode(icons.b64ButtonRed)
        binButtonGray = base64.b64decode(icons.b64ButtonGray)

        ico16 = ImageTk.PhotoImage(data=binIco16)
        ico32 = ImageTk.PhotoImage(data=binIco32)
        ico64 = ImageTk.PhotoImage(data=binIco64)

        self.imgHeadset32Gray = ImageTk.getimage(ImageTk.PhotoImage(data=binHeadset32Gray))
        self.imgHeadset32Green = ImageTk.getimage(ImageTk.PhotoImage(data=binHeadset32Green))
        self.imgHeadset32Red = ImageTk.getimage(ImageTk.PhotoImage(data=binHeadset32Red))

        self._imgButtonRed = ImageTk.PhotoImage(data=binButtonRed)
        self._imgButtonGreen = ImageTk.PhotoImage(data=binButtonGreen)
        self._imgButtonGray = ImageTk.PhotoImage(data=binButtonGray)

        # Window configuration
        
        self.win.title("Mute button Application")

        # Set the size of the window
        self._windowSizeSetup()
        self.win.columnconfigure(1, weight=1)
        self.win.rowconfigure(0, weight=1)
        self.win.grid_propagate(False)
        self.win.protocol('WM_DELETE_WINDOW', self._hideWindow)

        # setup app icon
        self.win.iconphoto(False, ico64, ico32, ico16)

        # app menu
        menubar = tk.Menu(self.win)
        self.win.config(menu=menubar)

        menuArchive = tk.Menu(menubar, tearoff=0)
        menuArchive.add_command(label="Close", command=self._hideWindow)
        menuArchive.add_command(label="Quit", command=self._quitWindow)

        menuAbout = tk.Menu(menubar, tearoff=0)
        menuAbout.add_command(label="Debug Mode", command=self._debugMode)

        menubar.add_cascade(label="Archive", menu=menuArchive)
        menubar.add_cascade(label="About", menu=menuAbout)

        # main layout
        mainFrame = tk.Frame(self.win)
        mainFrame.grid(row=0, column=0, sticky=tk.NSEW)

        lblTop = tk.Label(mainFrame, text="Audio Input Status:", )
        #lblTop.grid(row=0, column=0, sticky=W, padx=(0,0), pady=(0,0))
        lblTop.pack(side=tk.TOP, anchor=tk.NW, expand=False)

        self._lblBtn = tk.Label(mainFrame, image=self._imgButtonGray)
        self._lblBtn.pack(side=tk.TOP, fill=tk.BOTH, expand=True, padx=(0,0), pady=(0,0))
        self._lblBtn.bind('<Button-1>', self._toggleState)

        self._lblMessage = tk.Label(mainFrame, text="Click")
        self._lblMessage.pack(side=tk.BOTTOM, anchor=tk.NW, fill=tk.BOTH, expand=False)

        # systray menu
        self.menu = pystray.Menu(
            pystray.MenuItem('Open application', self._showWindow, default=True),
            pystray.MenuItem('Mute On', self._menuMuteOn),
            pystray.MenuItem('Mute Off', self._menuMuteOff),
            pystray.Menu.SEPARATOR,
            pystray.MenuItem('Quit', self._quitWindow), 
            )
        self.icon = pystray.Icon("name", self.imgHeadset32Gray, "Mute button Application", menu=self.menu)
        threading.Thread(daemon=True, target=lambda: self.icon.run()).start()

        # init global vars
        self._destroyOrder = False
        self._newState: int = None
        self._currState: int = muteBtnMngr.MuteButtonDevice.StateError
        self._guiOrder = Front.OrderStandBy
        self._dbgMode = False
        self._dbgMsgQueue = queue.Queue()


    def task(self) -> bool:
        if self.win is None: return False

        if self._destroyOrder: 
            self.win.destroy()
            self._destroyOrder = False
            return False
        
        if self._newState is not None:
            self._currState = self._newState
            self._newState = None

            if self._currState == muteBtnMngr.MuteButtonDevice.StateMuteOff: 
                self.icon.icon = self.imgHeadset32Green
                self._lblBtn.configure(image=self._imgButtonGreen)
                self._lblMessage.config(text='Click to Mute')
            elif self._currState == muteBtnMngr.MuteButtonDevice.StateMuteOn:
                self.icon.icon = self.imgHeadset32Red
                self._lblBtn.configure(image=self._imgButtonRed)
                self._lblMessage.config(text='Click to Unmute')
            elif self._currState == muteBtnMngr.MuteButtonDevice.StateError:
                self.icon.icon = self.imgHeadset32Gray
                self._lblBtn.configure(image=self._imgButtonGray)
                self._lblMessage.config(text='')

        if self._guiOrder != Front.OrderStandBy:
            if self._guiOrder == Front.OrderShowWindow: 
                self.win.after(0, self.win.deiconify)
            elif self._guiOrder == Front.OrderHideWindow: 
                self.win.withdraw()

            self._guiOrder = Front.OrderStandBy

        if self._dbgMode and not self._dbgMsgQueue.empty():
            self._debugText.config(state=tk.NORMAL)
            self._debugText.insert(1.0, f'{self._dbgMsgQueue.get()}\n')
            self._debugText.config(state=tk.DISABLED)

        self.win.update_idletasks()
        self.win.update()

        return True
    

    def changeState(self, state: int, volume: int):
        dbgPrint(f'Front.changeState. State: [{state}]; Volume: [{volume}]')
        self._newState = state


    def _toggleState(self, event):
        if self._eventCallback is None: return

        if self._currState == muteBtnMngr.MuteButtonDevice.StateMuteOn:
            self._eventCallback(muteBtnMngr.MuteButtonDevice.StateMuteOff, None)
        elif self._currState == muteBtnMngr.MuteButtonDevice.StateMuteOff: 
            self._eventCallback(muteBtnMngr.MuteButtonDevice.StateMuteOn, None)


    def _menuMuteOff(self):
        self._eventCallback(muteBtnMngr.MuteButtonDevice.StateMuteOff, None)


    def _menuMuteOn(self):
        self._eventCallback(muteBtnMngr.MuteButtonDevice.StateMuteOn, None)
        

    # Define a function for quit the window
    def _quitWindow(self, icon = None, item = None):
        self.icon.stop()
        self._destroyOrder = True


    # Define a function to show the window again
    def _showWindow(self, icon, item):
        self._guiOrder = Front.OrderShowWindow

      
    # Hide the window and show on the system taskbar
    def _hideWindow(self):
        self._guiOrder = Front.OrderHideWindow


    def _debugMode(self):
        if self._dbgMode:
            # disable debug mode
            self._dbgMode = False

            dbgRemoveCallback(self._dbgCallback)

            # Set the size of the window
            self._windowSizeSetup()

            # remove fields
            self._debugText.grid_remove()
            self._debugText.pack_forget()
            return
        
        # enable debug mode
        # Set the size of the window
        self._windowSizeSetup(500)

        # add text field
        dbgFont = f'Courier {7}'
        self._debugText = scrolledtext.ScrolledText(self.win, font=dbgFont)
        self._debugText.config(state=tk.DISABLED)
        self._debugText.grid(row=0, column=1, sticky=tk.NSEW)

        with self._dbgMsgQueue.mutex: self._dbgMsgQueue.queue.clear()
        dbgAddCallback(self._dbgCallback)

        self._dbgMode = True


    def _windowSizeSetup(self, extraWidth: int = 0):
        self._screenSizeX = self._imgButtonGray.width() + 10 + extraWidth
        self._screenSizeY = 340
        self.win.geometry(f'{self._screenSizeX}x{self._screenSizeY}')
        self.win.minsize(self._screenSizeX, self._screenSizeY)
        self.win.maxsize(self._screenSizeX, self._screenSizeY)


    def _dbgCallback(self, line):
        if not self._dbgMode: return
        self._dbgMsgQueue.put(line)


#______________________________________________________________________________
#______________________________________________________________________________
#______________________________________________________________________________
#______________________________________________________________________________


def appEventCallback(state: int, volume: int):
    dbgPrint(f'Event from app. State: {state}; volumne: {volume}')
    frontend.changeState(state, volume)


def frontEventCallback(state: int, volume: int):
    dbgPrint(f'Event from frontend. State: {state}; volumne: {volume}')
    app.externalCall(state, volume)



app = muteBtnMngr.App(appEventCallback)
frontend = Front(frontEventCallback)

app.start()

while True:
     time.sleep(0.05)
     if not frontend.task(): break

dbgPrint(f'Finishing....')
app.join()

