"""Contains frames to select users or create new users.

Author: Xavier Corbillon
IMT Atlantique
"""

import logging
from tkinter import *
from functools import partial

module_logger = logging.getLogger('TestManager.GUIHelpers.UserSelectionFrame')


class ExistingUserFrame(Frame):
    """Frame used to select an existing user."""

    def __init__(self, *args, userManager, **kwargs):
        """Init function."""
        Frame.__init__(self, *args, **kwargs)
        self.parent = args[0]
        self.userManager = userManager

        self.userListComboBox = tix.ComboBox(self,
                                             label='Existing User: ',
                                             editable=0)
        self.UpdateUserList()
        self.userListComboBox.grid(row=0, column=0)

        self.useExistingUserButton = Button(
            self,
            text='Use existing user',
            command=partial(self.PressedButton)
            )
        self.useExistingUserButton.grid(row=0, column=1)

    def UpdateUserList(self):
        """Update the user list in the combobox."""
        module_logger.debug('Update existing user list')
        self.userListComboBox.subwidget('listbox').delete(0, END)
        for uid, userName in self.userManager.GetExistingUserList().items():
            self.userListComboBox.insert(uid, '{}: {}'.format(uid, userName))

    def PressedButton(self):
        """Callback when the self.useExistingUserButton is pressed."""
        selection = self.userListComboBox['selection']
        if (len(selection) > 0):
            uid, name = selection.split(':')
            uid = int(uid)
            module_logger.info(
                'Existing user with id {} selected for a new test'.format(
                    uid))
            self.parent.grid_remove()
            self.parent.Reset()


class QuestionnaireFrame(Frame):
    """Frame containing the form to fill."""

    def __init__(self, *args, **kwargs):
        """Init function."""
        Frame.__init__(self, *args, **kwargs)
        self.parent = args[0]

        self.firstNameLabelEntry = tix.LabelEntry(self, label='First name: ')
        self.lastNameLabelEntry = tix.LabelEntry(self, label='Last name: ')

        self.submitTheFormButton = Button(
            self,
            text='Submit the form',
            command=partial(self.PressedButton)
            )

        self.firstNameLabelEntry.grid(row=0, column=0)
        self.lastNameLabelEntry.grid(row=0, column=1)
        self.submitTheFormButton.grid(row=1, column=0)

    def Reset(self):
        """Reset the frame."""
        module_logger.debug('Reset QuestionnaireFrame')
        self.firstNameLabelEntry.subwidget('entry').delete(0, 'end')
        self.lastNameLabelEntry.subwidget('entry').delete(0, 'end')

    def PressedButton(self):
        """Callback when the self.submitTheFormButton is pressed."""
        module_logger.info('Submit the form button pressed')
        firstName = self.firstNameLabelEntry.subwidget('entry').get()
        lastName = self.lastNameLabelEntry.subwidget('entry').get()
        if len(firstName) > 0 and len(lastName) > 0:
            self.parent.userManager.AddNewUser(firstName, lastName)
        self.parent.parent.Reset()


class NewUserFrame(Frame):
    """Frame used to create a new user by filling a form."""

    def __init__(self, *args, userManager, **kwargs):
        """Init function."""
        Frame.__init__(self, *args, **kwargs)
        self.parent = args[0]
        self.userManager = userManager
        self.newUserButton = Button(
            self,
            text='Create a new user',
            command=partial(self.PressedButton)
            )
        self.questionnaireFrame = QuestionnaireFrame(self)
        self.newUserButton.grid(row=0, column=0)

    def PressedButton(self):
        """Callback when the self.newUserButton is pressed."""
        module_logger.info('Create new user button pressed, '
                           'display the questionnaire')
        self.newUserButton.grid_remove()
        self.parent.existingUserFrame.grid_remove()
        self.questionnaireFrame.grid(row=0, column=0)

    def Reset(self):
        """Reset the frame."""
        module_logger.debug('Reset NewUserFrame')
        self.questionnaireFrame.grid_remove()
        self.newUserButton.grid(row=0, column=0)


class UserSelectionFrame(Frame):
    """The main frame used for user selection.

    (creating a new one after filling a form or using an existing one)
    """

    def __init__(self, *args, userManager, **kwargs):
        """Init function."""
        Frame.__init__(self, *args, **kwargs)
        self.userManager = userManager
        self.existingUserFrame = ExistingUserFrame(self,
                                                   userManager=userManager)
        self.newUserFrame = NewUserFrame(self,
                                         userManager=userManager)
        self.existingUserFrame.grid(row=1, column=0)
        self.newUserFrame.grid(row=0, column=0)

    def Reset(self):
        """Reset to first display state."""
        module_logger.debug('Reset user selection frame')
        self.newUserFrame.Reset()
        self.Update()
        self.existingUserFrame.grid(row=1, column=0)
        self.newUserFrame.grid(row=0, column=0)

    def Update(self):
        """Update the frame. Especialy update the existing user list."""
        self.existingUserFrame.UpdateUserList()
