"""Contains the UserManager class.

Author: Xavier Corbillon
IMT Atlantique
"""

from .User import User
import logging
import os


class UserManager(object):
    """class that manager all the known users in the system.

    This class is used to parse the existing user file and to add a new user
    """

    def __init__(self, pathToExistingUserFile):
        """init function that parse the existing user file.

        :param pathToExistingUserFile: Path to the existing user file.
            This file contains one line for each user. It contains the user
            first name, last name and the user id separated with ';'.
        :type pathToExistingUserFile: str
        """
        self.logger = logging.getLogger('TestManager.Helpers.UserManager')
        self.pathToExistingUserFile = pathToExistingUserFile

        self.userDict = dict()
        self.maxUid = -1
        if os.path.exists(self.pathToExistingUserFile):
            with open(self.pathToExistingUserFile, 'r') as i:
                for line in i:
                    info = line.split(';')
                    uid = int(info[2])
                    self.maxUid = max(uid, self.maxUid)
                    firstName = info[0]
                    lastName = info[1]
                    self.logger.info(
                        'Add existing user: [{}] {} {}'.format(uid,
                                                               firstName,
                                                               lastName)
                        )
                    self.userDict[uid] = User(firstName, lastName, uid)

    def GetExistingUserList(self):
        """Return a dict of uid,string: first and last name."""
        ans = dict()
        for uid, user in self.userDict.items():
            ans[uid] = '{} {}'.format(user.firstName, user.lastName)
        return ans

    def AddNewUser(self, firstName, lastName):
        """Add a new user to the user dict and return the uid."""
        self.maxUid += 1
        self.logger.info(
            'Create a new user with uid {}: {} {}'.format(self.maxUid,
                                                          firstName,
                                                          lastName)
            )
        self.userDict[self.maxUid] = User(firstName, lastName, self.maxUid)
        self.StoreUserList()
        return self.maxUid

    def StoreUserList(self):
        """Store the user dict to the pathToExistingUserFile file."""
        with open(self.pathToExistingUserFile, 'w') as o:
            self.logger.debug('Update existing user file')
            for user in self.userDict.values():
                o.write('{};{};{}\n'.format(user.firstName,
                                            user.lastName,
                                            user.uid)
                        )
