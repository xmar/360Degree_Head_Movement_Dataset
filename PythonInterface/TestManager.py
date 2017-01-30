#!/usr/bin/env python3

"""GUI to help the user to run head position measurement tests.

Author: Xavier Corbillon
IMT Atlantique
"""

from functools import partial
from tkinter import *
from tkinter.ttk import *
from tkinter.constants import *
import argparse
import os
import logging


import GUIHelpers
import Helpers


if __name__ == '__main__':

    # create logger with 'spam_application'
    logger = logging.getLogger('TestManager')
    logger.setLevel(logging.DEBUG)
    # create file handler which logs even debug messages
    fh = logging.FileHandler('testManager.log')
    fh.setLevel(logging.DEBUG)
    # create console handler with a higher log level
    ch = logging.StreamHandler()
    ch.setLevel(logging.DEBUG)
    # create formatter and add it to the handlers
    formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
        )
    fh.setFormatter(formatter)
    ch.setFormatter(formatter)
    # add the handlers to the logger
    logger.addHandler(fh)
    logger.addHandler(ch)

    logger.info('Start the TestManager')

    # get program arguments
    parser = argparse.ArgumentParser(
        description='GUI to run head position measurements')
    parser.add_argument('--configFile', '-c',
                        type=str,
                        help='path to the configuration file [config.ini]',
                        default='config.ini'
                        )

    args = parser.parse_args()

    # parse the ini file
    iniConfParser = Helpers.GetIniConfParser(args.configFile, ch=ch, fh=fh)

    # parse existing user file
    userManager = Helpers.UserManager(os.path.join(iniConfParser.resultFolder,
                                                   '.private_existingUsers.txt'
                                                   ),
                                      iniConfParser.resultFolder
                                      )

    # Define the GUI window
    root = GUIHelpers.GetRootFrame()
    root.geometry("500x400+100+100")
    root.resizable(True, True)

    # This frame contains the objects used for the questionnaries
    userSelectionFrame = \
        GUIHelpers.UserSelectionFrame(root,
                                      userManager=userManager,
                                      videoManager=iniConfParser.videoManager
                                      )
    # This frame will contains the startup objets
    mainFrame = GUIHelpers.GetHomeFrame(
        root, userSelectionFrame=userSelectionFrame)
    mainFrame.grid(row=0, column=0)

    root.grid_rowconfigure(0, weight=1)
    root.grid_columnconfigure(0, weight=1)

    # Start the GUI main loop
    logger.info('Start main loop')
    root.mainloop()
    logger.info('Quit main loop')
