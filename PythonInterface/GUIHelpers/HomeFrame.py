"""Frame that contains the startup widgets of the application.

Author: Xavier Corbillon
IMT Atlantique
"""

import logging
from tkinter import *
from functools import partial


class HomeFrame(Frame):
    """Frame that contains the startup widgets."""

    def __init__(self, *args, userSelectionFrame, **kwargs):
        """Init function.

        :param userSelectionFrame: a Frame used to select an existing user
            or create a new one

        *args and **kwargs are forwared to the Frame constructor
        """
        Frame.__init__(self, *args, **kwargs)
        self.logger = logging.getLogger('TestManager.GUIHelpers.HomeFrame')
        self.userSelectionFrame = userSelectionFrame
        self.startTestButton = Button(
            self,
            text='Start a new test',
            command=partial(self._pressStartNewTestButton)
            )
        self.startTestButton.grid(row=0, column=0)

    def _pressStartNewTestButton(self):
        """Call when the start a new test button is pressed.

        Hide this frame and display the userSelectionFrame
        """
        self.logger.info('Start a new test button pressed')
        self.grid_remove()
        self.userSelectionFrame.grid(row=0, column=0)
