#!/usr/bin/python3
"""This script is used to export the results into a tar.gz file.

The exported results will not contains the name of the users and can be merge
with other results.

This script suppose the results are stored in the folder named "results" in the
current folder.

Output a tar gz archiv named "dataset.tar.gz"

Author: Xavier Corbillon
IMT Atlantique
"""

import tarfile
import os

if __name__ == '__main__':
    if not os.path.exists('results'):
        print('"results" folder not found')
        exit(1)
    def __exclude(fileName):
        return '.private_existingUsers.txt' in fileName or 'statistics' in fileName
    with tarfile.open('dataset.tar.gz', mode='w:gz') as outputTar:
        outputTar.add('results', exclude=__exclude)
