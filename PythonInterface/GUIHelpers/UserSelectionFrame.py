"""Contains frames to select users or create new users.

Author: Xavier Corbillon
IMT Atlantique
"""

import logging
from tkinter import *
from tkinter import messagebox
from functools import partial

from .TestConfigurationFrame import TestConfigurationFrame

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
            self.parent.DisplayTestConfiguration(
                self.userManager.userDict[uid]
                )


class QuestionnaireFrame(Frame):
    """Frame containing the form to fill."""

    def __init__(self, *args, **kwargs):
        """Init function."""
        Frame.__init__(self, *args, **kwargs)
        self.parent = args[0]

        row = 0

        # First and Last Name
        self.firstNameLabelEntry = tix.LabelEntry(self, label='First name: ')
        self.lastNameLabelEntry = tix.LabelEntry(self, label='Last name: ')

        self.firstNameLabelEntry.grid(row=row, column=0)
        self.lastNameLabelEntry.grid(row=row, column=1)
        row += 1

        # Gender
        self.genderValue = StringVar(self)
        self.genderLabel = Label(self,
                                 text='Gender: ')
        self.womanGender = Radiobutton(self,
                                       text='woman',
                                       variable=self.genderValue,
                                       value='woman')
        self.manGender = Radiobutton(self,
                                     text='man',
                                     variable=self.genderValue,
                                     value='man')
        self.genderLabel.grid(row=row, column=0)
        self.womanGender.grid(row=row, column=1)
        self.manGender.grid(row=row, column=2)
        row += 1

        # age
        self.ageLabel = Label(self,
                              text='Age: ')
        self.ageValue = StringVar(self)
        self.ageSpinbox = Spinbox(self, from_=1, to=120, width=5,
                                  textvariable=self.ageValue, justify='right')
        self.ageLabel.grid(row=row, column=0)
        self.oldAge = '25'
        self.ageValue.set(self.oldAge)
        self.ageValue.trace('w',
                            lambda nm, idx, mode, var=self.ageValue:
                                QuestionnaireFrame.ValidateAge(self)
                            )
        self.ageSpinbox.grid(row=row, column=1)
        row += 1

        # impairment to vision
        self.impairmentLabel = Label(self,
                                     text='Impairment to vision: ')
        self.impairmentValue = StringVar(self)
        impairmentDefaults = ['none',
                              'daltonism',
                              'myopia',
                              'hypermetropia',
                              'astigmatic',
                              'myopia&astigmatic',
                              'hypermetropia&astigmatic',
                              ]
        self.impairmentSpinbox = Spinbox(self, values=impairmentDefaults,
                                         textvariable=self.impairmentValue,
                                         justify='right')
        self.impairmentLabel.grid(row=row, column=0)
        self.impairmentSpinbox.grid(row=row, column=1)
        row += 1

        # used HDM
        self.usedHMDLabel = Label(self,
                                  text='How many hours have \n'
                                       'you already used a HDM?'
                                  )
        self.usedHMDValue = StringVar(self)
        self.usedHMDEntry = Entry(self, textvariable=self.usedHMDValue,
                                  justify='right', width=5)
        self.oldUsedHMD = '0'
        self.usedHMDValue.set(self.oldUsedHMD)
        self.usedHMDValue.trace('w',
                                lambda nm, idx, mode,
                                var=self.usedHMDValue:
                                QuestionnaireFrame.ValidateUsedHMD(self)
                                )
        self.usedHMDLabel.grid(row=row, column=0)
        self.usedHMDEntry.grid(row=row, column=1)
        row += 1

        # application used
        self.appsUsedLabel = Label(self,
                                   text='Which kind of application \n'
                                   'did you used?'
                                   )
        self.appsUsedLabel.grid(row=row, column=0)
        row += 1

        # device used
        self.devicessUsedLabel = Label(self,
                                       text='Which kind of devices \n'
                                       'have you already used?'
                                       )
        self.devicessUsedLabel.grid(row=row, column=0)
        row += 1

        # submit button
        self.submitTheFormButton = Button(
            self,
            text='Submit the form',
            command=partial(self.PressedSubmitButton)
            )
        self.submitTheFormButton.grid(row=row, column=0)
        row += 1

    def ValidateAge(self):
        """Check if self.ageValue is a number."""
        new_value = self.ageValue.get()
        try:
            new_value == '' or int(new_value)
            if new_value == '' or (int(new_value) >= 1 and
                                   int(new_value) <= 120):
                self.oldAge = new_value.strip()
        except:
            self.ageValue.set(self.oldAge)
        else:
            if self.oldAge != new_value:
                self.ageValue.set(self.oldAge)

    def ValidateUsedHMD(self):
        """Check if self.usedHMDValue is a number."""
        new_value = self.usedHMDValue.get()
        try:
            new_value == '' or float(new_value)
            if new_value == '':
                new_value = '0'
            if new_value == '' or float(new_value) >= 0:
                self.oldUsedHMD = new_value.strip()
        except:
            self.usedHMDValue.set(self.oldUsedHMD)
        else:
            if self.oldUsedHMD != new_value:
                self.usedHMDValue.set(self.oldUsedHMD)

    def Reset(self):
        """Reset the frame."""
        module_logger.debug('Reset QuestionnaireFrame')
        self.firstNameLabelEntry.subwidget('entry').delete(0, 'end')
        self.lastNameLabelEntry.subwidget('entry').delete(0, 'end')

    def PressedSubmitButton(self):
        """Callback when the self.submitTheFormButton is pressed."""
        module_logger.info('Submit the form button pressed')
        firstName = self.firstNameLabelEntry.subwidget('entry').get()
        lastName = self.lastNameLabelEntry.subwidget('entry').get()
        uid = None
        self.impairmentValue.set(self.impairmentValue.get().lower())
        # check if all field are valid
        if len(firstName) == 0 or len(lastName) == 0:
            messagebox.showwarning('Not valid form', 'A first name and a last'
                                   'name should be given')
            return
        if len(self.genderValue.get()) == 0:
            messagebox.showwarning('Not valid form', 'A gender '
                                   'should be given')
            return
        if len(self.ageValue.get()) == 0:
            messagebox.showwarning('Not valid form', 'Age should not be empty')
            return
        if len(self.impairmentValue.get()) == 0:
            self.impairmentValue.set('none')
        if len(self.usedHMDValue.get()) == 0:
            messagebox.showwarning('Not valid form', 'If never used a HMD, the'
                                   'value should be 0')
            return
        uid = self.parent.userManager.AddNewUser(firstName, lastName)
        if uid is None:
            self.parent.parent.Reset()
        else:
            self.parent.parent.DisplayTestConfiguration(
                self.parent.userManager.userDict[uid]
                )


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

    def __init__(self, *args, userManager, videoManager, **kwargs):
        """Init function."""
        Frame.__init__(self, *args, **kwargs)
        self.parent = args[0]
        self.userManager = userManager
        self.videoManager = videoManager
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

    def DisplayTestConfiguration(self, selectedUser):
        """Will hide this frame and display the TestConfigurationFrame."""
        self.grid_remove()
        self.Reset()
        module_logger.info('Create the TestConfiguration for uid {}'.format(
            selectedUser.uid)
                           )
        TestConfigurationFrame(self.parent,
                               videoManager=self.videoManager,
                               selectedUser=selectedUser).grid(row=0,
                                                               column=0
                                                               )
