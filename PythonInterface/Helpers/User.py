"""Contains all information needed on a user.

Author: Xavier Corbillon
IMT Atlantique
"""

import os


class User(object):
    """Class that contains all usefull information on a user."""

    def __init__(self, userFirstName, userLastName, userId, rootResultFolder):
        """init function.

        :type userFirstName: str
        :type userLastName: str
        :type userId: int
        """
        self.firstName = userFirstName
        self.lastName = userLastName
        self.uid = userId
        self.userResultFolder = os.path.join(rootResultFolder,
                                             'uid'+str(self.uid)
                                             )
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

    def GetNumberExistingTest(self):
        """Number of test that already exist for this user.

        :rtype: int
        """
        nbTest = 0
        for root, dirs, files in os.walk(self.userResultFolder):
            for name in dirs:
                if 'test' in name:
                    nbTest += 1
        return nbTest

    def GetTestResultFolder(self, testNumber):
        """Return the path to the test testNumber for this user.

        :rtype: str
        """
        return os.path.join(self.userResultFolder, 'test{}'.format(testNumber))
