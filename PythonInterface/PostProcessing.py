#!/usr/bin/env python3
"""This script do the postprocessing of the test results.

Author: Xavier Corbillon
IMT Atlantique
"""

import argparse
import os
import logging

from Helpers import GetIniConfParser, GetGlobalUserManager, GetGlobalStatistics

if __name__ == '__main__':
    # create logger with 'spam_application'
    logger = logging.getLogger('TestManager')
    logger.setLevel(logging.DEBUG)
    # create file handler which logs even debug messages
    fh = logging.FileHandler('testManagerStats.log')
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
    parser.add_argument('withVideo', action='store_true',
                        help='if set compute heatmap videos',
                        )

    args = parser.parse_args()

    # parse the ini file
    iniConfParser = GetIniConfParser(args.configFile, ch=ch, fh=fh)

    # parse existing user file
    userManager = GetGlobalUserManager(os.path.join(
        iniConfParser.resultFolder,
        '.private_existingUsers.txt'
        ),
                                      iniConfParser.resultFolder
                                      )

    # Init the global statistics object
    stats = GetGlobalStatistics(userManager)

    print(args.withVideo)
    stats.RunComputation(args.withVideo)
