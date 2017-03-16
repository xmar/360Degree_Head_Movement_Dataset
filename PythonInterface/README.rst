README
======

Introduction
------------

This folder contains scripts used to:

 1. run a test session
 2. do some basic post-processing on the dataset
 3. export the dataset into a tar.gz file removing the private information about the users

Initialisation:
---------------

Before running the first test you need to install python3, cmake and a C++ compiler.
It is also recommended to use the virtualenv package from python3.

You need to create inside the PythonInerface folder an empty virtualenv directory named .env:

  python -m venv .env

Start a test session:
---------------------

To start a test session, you can run the startTestManager.sh bash script. This script
will start the virtualenv, install the python package required inside (those
package are listed in the file requirements.txt), compile the Helpers.CQuaternions module
using cmake and then run the TestManager.py script.

The TestManager will create a folder named results that will contains the answers of the questionnaire
for each user and the dataset gathered for each user.

This script will read the configuration file named config.ini that should be in the same folder.

Here an example of the config.ini file (the one used to harvest our dataset):

  [AppConfig]
  resultFolder = results
  pathToOsvrClientPlayer = ../build/OSVRClientTest
  portForInterprocessCommunication=5542
  ;Section name of the video used for the training (empty for none)
  ;trainingVideo = Noa_Neal_GraffitiNoa_Neal_Graffiti_t
  trainingVideo =
  ;Elephant, Rhino
  ;List of Section name (comma separated) of the video used in the test
  videoConfigList = Diving, Rollercoaster, Timelapse, Venise, Paris
  ;supported log levels: DEBUG, INFO, WARNING, ERROR
  consoleLogLevel= DEBUG
  fileLogLevel=DEBUG

  [Elephant]
  path=videos/2bpICIClAIg.webm
  id=Elephant-training-2bpICIClAIg
  nbMaxFrames=2100
  bufferSize=250
  startOffsetInSecond=15

  [Rhino]
  path=videos/7IWp875pCxQ.webm
  id=Rhino-training-7IWp875pCxQ
  nbMaxFrames=2100
  bufferSize=250
  startOffsetInSecond=15

  [Diving]
  path=videos/2OzlksZBTiA.mkv
  id=Diving-2OzlksZBTiA
  nbMaxFrames=2100
  bufferSize=250
  startOffsetInSecond=40

  [Rollercoaster]
  path=videos/8lsB-P8nGSM.mkv
  id=Rollercoaster-8lsB-P8nGSM
  nbMaxFrames=2100
  ;nbMaxFrames=1000
  bufferSize=250
  startOffsetInSecond=65

  [Timelapse]
  path=videos/CIw8R8thnm8.mkv
  id=Timelapse-CIw8R8thnm8
  nbMaxFrames=2100
  bufferSize=250
  startOffsetInSecond=0

  [Venise]
  path=videos/s-AJRFQuAtE.mkv
  id=Venise-s-AJRFQuAtE
  nbMaxFrames=1800
  bufferSize=250
  startOffsetInSecond=0

  [Paris]
  path=videos/sJxiPiAaB4k.mkv
  id=Paris-sJxiPiAaB4k
  nbMaxFrames=4200
  bufferSize=250
  startOffsetInSecond=0

This config.ini suppose that all video are stores in the folder named videos and
suppose the  ``OSVR Video Player and Head Movement'' logger program is store at
this location: ../build/OSVRClientTest


Run the Post-Processing script
------------------------------

You can run the basic post processing script by using the startPostProcessing.sh
bash script. This script will generate a statistics folder inside the results folder.


Export the dataset
------------------

To export the dataset and for instance to share it on our website ( http://dash.ipv6.enstb.fr/headMovements )
you can use the ExportResults.py python3 script. It will read the results folder and will
generate a dataset.tar.gz archive that contains only the dataset without the private user
information nor the post-processing results folder.

Test platform:
--------------

All the codes and scripts were tested on linux: archlinux with
up-to-date packages.
In theory, the C++ software will work on windows with the windows
version of the OSVR API. We never tested it.

We used the Razer OSVR HDK2 Head Mounted Display. The sofware should
work as it is with other HMDs such as the Oculus Rift or the HTC Vive
if the osvr driver for those HMDs are installed.

We use a Nvidia GE 1060 graphic card.

Contacts:
---------
E-mail: xavier.corbillon[at]imt-atlantique.fr
Website: http://dash.ipv6.enstb.fr/headMovements
