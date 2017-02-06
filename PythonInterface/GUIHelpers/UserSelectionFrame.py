"""Contains frames to select users or create new users.

Author: Xavier Corbillon
IMT Atlantique
"""

import logging
from tkinter import *
from tkinter.ttk import *
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


class CheckboxWithLabel(Frame):
    """Class that manage a checkbox with a label."""

    def __init__(self, *args, label, withEntry=False, **kwargs):
        """init function."""
        Frame.__init__(self, *args, **kwargs)
        self.withEntry = withEntry
        self.labelText = label
        self.label = Label(self, text=self.labelText)
        self.value = BooleanVar(self, False)
        self.checkbox = Checkbutton(self, variable=self.value)
        if withEntry:
            self.entryValue = StringVar(self)
            self.entry = Entry(self,
                               textvariable=self.entryValue,
                               justify='right')
        self.checkbox.grid(row=0, column=0)
        self.label.grid(row=0, column=1)
        if withEntry:
            self.entry.grid(row=0, column=2)

    def GetValue(self):
        """Return the value of the checkbox.

        The value is the label if withEntry=False and the checkbox is set,
        the value of self.entryValue if withEntry=True and the checkbox is set,
        or None if the checkbox is not set.
        """
        if not self.value.get():
            return None
        elif self.withEntry:
            return self.entryValue.get()
        else:
            return self.labelText


class QuestionnaireFrame(Frame):
    """Frame containing the form to fill."""

    def __init__(self, *args, **kwargs):
        """Init function."""
        Frame.__init__(self, *args, **kwargs)
        self.parent = args[0]

    def grid(self, *args, **kwargs):
        """Overide the grid methode to reset the frame each time we draw it."""
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
                              'presbiopia',
                              'myopia&presbiopia',
                              'hypermetropia&presbiopia',
                              'astigmatic',
                              'myopia&astigmatic',
                              'hypermetropia&astigmatic',
                              'astigmatic&presbiopia',
                              'myopia&astigmatic&presbiopia',
                              'hypermetropia&astigmatic&presbiopia',
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
        self.applicationFrame = Frame(self)
        self.appsUsedLabel = Label(self.applicationFrame,
                                   text='Which kind of application \n'
                                   'did you used?'
                                   )
        self.appsUsedGameCheckbox = CheckboxWithLabel(self.applicationFrame,
                                                      label='Game')
        self.appsUsedYouTubeCheckbox = CheckboxWithLabel(self.applicationFrame,
                                                         label='YouTube360')
        self.appsUsedFacebookCheckbox = \
            CheckboxWithLabel(self.applicationFrame,
                              label='Facebook')
        self.appsUsedStaticCheckbox = CheckboxWithLabel(self.applicationFrame,
                                                        label='Static '
                                                        'Panorama')
        self.appsUsedOtherCheckbox = CheckboxWithLabel(self.applicationFrame,
                                                       label='Other',
                                                       withEntry=True)
        # self.applicationFrame.grid(row=row, column=0)
        self.rowApps = row
        rowApp = 0
        self.appsUsedLabel.grid(row=rowApp, column=0)
        self.appsUsedGameCheckbox.grid(row=rowApp, column=1)
        rowApp += 1
        self.appsUsedYouTubeCheckbox.grid(row=rowApp, column=1)
        rowApp += 1
        self.appsUsedFacebookCheckbox.grid(row=rowApp, column=1)
        rowApp += 1
        self.appsUsedStaticCheckbox.grid(row=rowApp, column=1)
        rowApp += 1
        self.appsUsedOtherCheckbox.grid(row=rowApp, column=1, columnspan=2)
        rowApp += 1
        row += 1

        # device used
        self.devicesFrame = Frame(self)
        self.devicesUsedLabel = Label(self.devicesFrame,
                                      text='Which kind of devices \n'
                                      'have you already used?'
                                      )
        self.devicesUsedHMDCheckbox = CheckboxWithLabel(self.devicesFrame,
                                                        label='HMD')
        self.devicesUsedDesktopCheckbox = CheckboxWithLabel(self.devicesFrame,
                                                            label='Desktop '
                                                            'Player')
        self.devicesUsedSmartphoneCheckbox = \
            CheckboxWithLabel(self.devicesFrame,
                              label='Smartphone')
        self.devicesUsedOtherCheckbox = CheckboxWithLabel(self.devicesFrame,
                                                          label='Other',
                                                          withEntry=True)
        self.rowDevices = row
        # self.devicesFrame.grid(row=self.rowDevices, column=0)
        rowDevices = 0
        self.devicesUsedLabel.grid(row=rowDevices, column=0)
        self.devicesUsedHMDCheckbox.grid(row=rowDevices, column=1)
        rowDevices += 1
        self.devicesUsedDesktopCheckbox.grid(row=rowDevices, column=1)
        rowDevices += 1
        self.devicesUsedSmartphoneCheckbox.grid(row=rowDevices, column=1)
        rowDevices += 1
        self.devicesUsedOtherCheckbox.grid(row=rowDevices, column=1,
                                           columnspan=2)
        rowDevices += 1
        row += 1

        # submit button
        self.submitTheFormButton = Button(
            self,
            text='Submit the form',
            command=partial(self.PressedSubmitButton)
            )
        self.submitTheFormButton.grid(row=row, column=0)
        row += 1
        super().grid(*args, **kwargs)

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
            if float(new_value) >= 0:
                self.oldUsedHMD = new_value.strip()
                if float(new_value) > 0:
                    self.devicesFrame.grid(row=self.rowDevices, column=0,
                                           columnspan=3)
                    self.applicationFrame.grid(row=self.rowApps, column=0,
                                               columnspan=3)
                else:
                    self.devicesFrame.grid_remove()
                    self.applicationFrame.grid_remove()
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
            pathToFormAns = \
                self.parent.userManager.GetUserByUid(uid)\
                .GetPathToUserFormAnswers()
            with open(pathToFormAns, 'w') as formAns:
                formAns.write('#questionId;answer(s) -> 0 = uid; 1 = gender;'
                              ' 2 = age; 3 = impairment;'
                              ' 4 = nbHour of HDM usage;'
                              ' 5 = apps used; 6 = devices used\n')
                # userId
                formAns.write('0;{}\n'.format(uid))
                # gender
                formAns.write('1;{}\n'.format(self.genderValue.get()))
                # age
                formAns.write('2;{}\n'.format(self.ageValue.get()))
                # impairment
                formAns.write('3;{}\n'.format(self.impairmentValue.get()))
                # hours of HDM usage
                formAns.write('4;{}\n'.format(self.usedHMDValue.get()))
                appsStr = ''
                devicesStr = ''
                if float(self.usedHMDValue.get()) > 0:
                    for checkbox in [
                        self.appsUsedGameCheckbox,
                        self.appsUsedYouTubeCheckbox,
                        self.appsUsedFacebookCheckbox,
                        self.appsUsedStaticCheckbox,
                        self.appsUsedOtherCheckbox
                    ]:
                        result = checkbox.GetValue()
                        if result is not None and len(result) > 0:
                            if len(appsStr) > 0:
                                appsStr += ','
                            appsStr += result
                    for checkbox in [
                        self.devicesUsedHMDCheckbox,
                        self.devicesUsedDesktopCheckbox,
                        self.devicesUsedSmartphoneCheckbox,
                        self.devicesUsedOtherCheckbox
                    ]:
                        result = checkbox.GetValue()
                        if result is not None and len(result) > 0:
                            if len(devicesStr) > 0:
                                devicesStr += ','
                            devicesStr += result
                formAns.write('5;{}\n'.format(appsStr))
                formAns.write('6;{}\n'.format(devicesStr))
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
