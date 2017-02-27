"""Contains all information needed on a user.

Author: Xavier Corbillon
IMT Atlantique
"""

import os
import re


def natural_keys(text):
    """Used to sort in humer order.

    alist.sort(key=natural_keys) sorts in human order
    http://nedbatchelder.com/blog/200712/human_sorting.html
    (See Toothy's implementation in the comments)
    """
    return [int(c) if c.isdigit() else c for c in re.split('(\d+)', text)]


class User(object):
    """Class that contains all usefull information on a user."""

    def __init__(self, userFirstName, userLastName, userId, rootResultFolder):
        """Init function.

        :type userFirstName: str
        :type userLastName: str
        :type userId: int
        """
        self.firstName = userFirstName
        self.lastName = userLastName
        self.uid = userId
        self.userResultFolder = os.path.join(rootResultFolder,
                                             'uid-'+str(self.uid)
                                             )
        self.age = None
        self.sex = None
        self.nbHourHMD = None
        if not os.path.exists(self.userResultFolder):
            os.makedirs(self.userResultFolder)

    def GetUserResultFolder(self):
        """Return the path to the user result folder.

        :rtype: str
        """
        return self.userResultFolder

    def GetPathToUserFormAnswers(self):
        """Return the path to the user form answers file.

        :rtype: str
        """
        return os.path.join(self.userResultFolder, 'formAnswers.txt')

    def ParseFormAnswers(self):
        """Get infos from the form answers."""
        if self.sex is None or self.age is None or self.nbHourHMD is None:
            pathToForm = self.GetPathToUserFormAnswers()
            with open(pathToForm, 'r') as f:
                for line in f:
                    if len(line) > 0 and line[0] != '#':
                        questionId, value = (line.rstrip()).split(';')
                        if questionId == '1':
                            self.sex = value
                        elif questionId == '2':
                            self.age = int(value)
                        elif questionId == '4':
                            self.nbHourHMD = float(value)

    def GetExistingTestPathList(self):
        """Return a list of path to existing test."""
        outputList = list()
        for root, dirs, files in os.walk(self.userResultFolder):
            if root == self.userResultFolder:
                for name in dirs:
                    if 'test' in name and name[0:4] == 'test':
                        outputList.append(os.path.join(root, name))
        outputList.sort(key=natural_keys)
        return outputList

    def GetNextTestId(self):
        """Number of test that already exist for this user.

        :rtype: int
        """
        nbTest = -1
        testPathList = self.GetExistingTestPathList()
        for testPath in testPathList:
            name = os.path.basename(testPath)
            nbTest = max(nbTest, int(name[4:]))
        nbTest += 1
        return nbTest

    def GetTestResultFolder(self, testNumber):
        """Return the path to the test testNumber for this user.

        :rtype: str
        """
        return os.path.join(self.userResultFolder, 'test{}'.format(testNumber))
