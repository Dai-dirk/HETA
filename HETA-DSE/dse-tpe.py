import sys
import optuna
import logging
import numpy as np
import time
import json
from archSpec.archSpec import *
from usrConfig.paraDefine import *
from visualization.visual import *

start_time = time.time()
global buildFromExistModel
buildFromExistModel = False
maxIt = 2

//@yuan: Full version of the code will be released after publication
