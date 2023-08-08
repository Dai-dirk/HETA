import sys
from typing import Dict, Tuple
from numpy.lib.npyio import load
import optuna
import logging
import numpy as np
import time
import json
import subprocess
from archSpec.archSpec import *
from usrConfig.paraDefine import *
from visualization.visual import *

start_time = time.time()
firstStride = True
strideUsage = {}
worseStride = 0
# usedUsage = {"GIB":[],"ADDSUB":[],"MUL":[],"CMP":[],"LOGIC":[],"SHIFT":[],"GPEDly":[],"LSUDly":[]}

def readHomoArch() -> dict:
    with open('cgra_spec_bestHomo.json', 'r') as f:
        bestHomoPara = json.load(f)
    return bestHomoPara

def readHomoEva() -> list: 
    with open('cgra_eva_bestHomo.json', 'r') as f:
        bestHomoEva = json.load(f)
    return bestHomoEva

def readHomoUsage() -> dict:
    usage = {}
    with open('Usage.txt', 'r') as file:
        lines = file.readlines()  
        for line in lines:
            parts = line.strip().split(":")  
            key = parts[0].strip()  
            values = [int(value) for value in parts[1].strip().split()]  
            usage[key] = values 
    return usage

def cutSpec(bestHomoPara:dict, usage:dict, judge:list) -> dict:
    global firstStride
    col = bestHomoPara["num_colum"]
    row = bestHomoPara["num_row"]
    gibPos = usage["GIB"][0]
    addPos = usage["ADDSUB"][0]
    mulPos = usage["MUL"][0]
    cmpPos = usage["CMP"][0]
    logicPos = usage["LOGIC"][0]
    shiftPos = usage["SHIFT"][0]
    gpedPos = usage["GPEDly"][0]
    lsudPos = usage["LSUDly"][0]
    
    #GIB cutting
    gibRow = gibPos // 2
    gibCol = (col+1) // 2
    gibRowSort = 0 if(gibPos%2==0) else gibCol
    gibRowBound = gibCol if(gibPos%2==0) else (col+1)
    ##opin_2_ipin_connect_flexibility
    if(bestHomoPara["gibs"][gibRow][gibRowSort]["opin_2_ipin_connect_flexibility"]["num_ipin_side1"]!=0 and judge[1][0]==1):
        for i in range (gibRowSort, gibRowBound) :
            bestHomoPara["gibs"][gibRow][i]["opin_2_ipin_connect_flexibility"]["num_ipin_side1"] -= 1
    if(bestHomoPara["gibs"][gibRow][gibRowSort]["opin_2_ipin_connect_flexibility"]["num_ipin_side2"]!=0 and judge[1][1]==1):   
        for i in range (gibRowSort, gibRowBound) :
            bestHomoPara["gibs"][gibRow][i]["opin_2_ipin_connect_flexibility"]["num_ipin_side2"] -= 1
    if(bestHomoPara["gibs"][gibRow][gibRowSort]["opin_2_ipin_connect_flexibility"]["num_ipin_side3"]!=0 and judge[1][2]==1):
        for i in range (gibRowSort, gibRowBound) :
            bestHomoPara["gibs"][gibRow][i]["opin_2_ipin_connect_flexibility"]["num_ipin_side3"] -= 1
    ##opin_2_otrack_connect_flexibility
    if(bestHomoPara["gibs"][gibRow][gibRowSort]["opin_2_otrack_connect_flexibility"]["num_otrack_side1"]!=0 and judge[1][3]==1):
        for i in range (gibRowSort, gibRowBound) :
            bestHomoPara["gibs"][gibRow][i]["opin_2_otrack_connect_flexibility"]["num_otrack_side1"] -= 1
    if(bestHomoPara["gibs"][gibRow][gibRowSort]["opin_2_otrack_connect_flexibility"]["num_otrack_side2"]!=0 and judge[1][4]==1):
        for i in range (gibRowSort, gibRowBound) :
            bestHomoPara["gibs"][gibRow][i]["opin_2_otrack_connect_flexibility"]["num_otrack_side2"] -= 1
    if(bestHomoPara["gibs"][gibRow][gibRowSort]["opin_2_otrack_connect_flexibility"]["num_otrack_side3"]!=0 and judge[1][5]==1):
        for i in range (gibRowSort, gibRowBound) :
            bestHomoPara["gibs"][gibRow][i]["opin_2_otrack_connect_flexibility"]["num_otrack_side3"] -= 1
    if(bestHomoPara["gibs"][gibRow][gibRowSort]["opin_2_otrack_connect_flexibility"]["num_otrack_side4"]!=0 and judge[1][6]==1):
        for i in range (gibRowSort, gibRowBound) :
            bestHomoPara["gibs"][gibRow][i]["opin_2_otrack_connect_flexibility"]["num_otrack_side4"] -= 1
    ##itrack_2_ipin_connect_flexibility
    if(bestHomoPara["gibs"][gibRow][gibRowSort]["itrack_2_ipin_connect_flexibility"]["num_itrack_side1"]!=0 and judge[1][7]==1):
        for i in range (gibRowSort, gibRowBound) :
            bestHomoPara["gibs"][gibRow][i]["itrack_2_ipin_connect_flexibility"]["num_itrack_side1"] -= 1
    if(bestHomoPara["gibs"][gibRow][gibRowSort]["itrack_2_ipin_connect_flexibility"]["num_itrack_side2"]!=0 and judge[1][8]==1):
        for i in range (gibRowSort, gibRowBound) :
            bestHomoPara["gibs"][gibRow][i]["itrack_2_ipin_connect_flexibility"]["num_itrack_side2"] -= 1
    if(bestHomoPara["gibs"][gibRow][gibRowSort]["itrack_2_ipin_connect_flexibility"]["num_itrack_side3"]!=0 and judge[1][9]==1):
        for i in range (gibRowSort, gibRowBound) :
            bestHomoPara["gibs"][gibRow][i]["itrack_2_ipin_connect_flexibility"]["num_itrack_side3"] -= 1
    if(bestHomoPara["gibs"][gibRow][gibRowSort]["itrack_2_ipin_connect_flexibility"]["num_itrack_side4"]!=0 and judge[1][10]==1):
        for i in range (gibRowSort, gibRowBound) :
            bestHomoPara["gibs"][gibRow][i]["itrack_2_ipin_connect_flexibility"]["num_itrack_side4"] -= 1
    
    #LSU cutting
    lsuCut = row // 2
    lsuSort = 0 if(lsudPos%2==0) else 1
    lsuUpBound = 0 if(lsudPos<=1) else lsuCut
    lsuDownBound = lsuCut if(lsudPos<=1) else row
    if(bestHomoPara["lsus"][lsuUpBound][lsuSort]["max_delay"] > 1 and judge[0][1]==1):
        for i in range(lsuUpBound, lsuDownBound):
            bestHomoPara["lsus"][i][lsuSort]["max_delay"] -= 1

    #GPE cutting
    gpeRow = row // 2
    gpeAddCol = addPos // 2
    gpeMulCol = mulPos // 2
    gpeCmpCol = cmpPos // 2
    gpeLogCol = logicPos // 2
    gpeShiCol = shiftPos // 2
    gpeDelCol = gpedPos // 2
    gpeAddUpBound = 0 if(addPos%2==0) else gpeRow
    gpeMulUpBound = 0 if(mulPos%2==0) else gpeRow
    gpeCmpUpBound = 0 if(cmpPos%2==0) else gpeRow
    gpeLogUpBound = 0 if(logicPos%2==0) else gpeRow
    gpeShiUpBound = 0 if(shiftPos%2==0) else gpeRow
    gpeDelUpBound = 0 if(gpedPos%2==0) else gpeRow
    gpeAddDownBound = gpeRow if(addPos%2==0) else row
    gpeMulDownBound = gpeRow if(mulPos%2==0) else row
    gpeCmpDownBound = gpeRow if(cmpPos%2==0) else row
    gpeLogDownBound = gpeRow if(logicPos%2==0) else row
    gpeShiDownBound = gpeRow if(shiftPos%2==0) else row
    gpeDelDownBound = gpeRow if(gpedPos%2==0) else row
    if(addPos==-1):
        if(firstStride):
            for r in range(row):
                for c in range(col):
                    pass
                    # bestHomoPara["gpes"][r][c]["operations"].remove("ADD")
                    # bestHomoPara["gpes"][r][c]["operations"].remove("SUB")
    else:
        if("ADD" in bestHomoPara["gpes"][gpeAddUpBound][gpeAddCol]["operations"] and judge[2][0]==1):
            for i in range(gpeAddUpBound, gpeAddDownBound):
                bestHomoPara["gpes"][i][gpeAddCol]["operations"].remove("ADD")
                bestHomoPara["gpes"][i][gpeAddCol]["operations"].remove("SUB")
    if(mulPos==-1):
        if(firstStride):
            for r in range(row):
                for c in range(col):
                    bestHomoPara["gpes"][r][c]["operations"].remove("MUL")
    else:
        if("MUL" in bestHomoPara["gpes"][gpeMulUpBound][gpeMulCol]["operations"] and judge[2][1]==1):
            for i in range(gpeMulUpBound, gpeMulDownBound):
                bestHomoPara["gpes"][i][gpeMulCol]["operations"].remove("MUL")
    if(cmpPos==-1):
        if(firstStride):
            for r in range(row):
                for c in range(col):
                    pass
                    # bestHomoPara["gpes"][r][c]["operations"].remove("EQ")
                    # bestHomoPara["gpes"][r][c]["operations"].remove("LT")
                    # bestHomoPara["gpes"][r][c]["operations"].remove("LE")
    else:
        if("EQ" in bestHomoPara["gpes"][gpeCmpUpBound][gpeCmpCol]["operations"] and judge[2][2]==1):
            for i in range(gpeCmpUpBound, gpeCmpDownBound):
                bestHomoPara["gpes"][i][gpeCmpCol]["operations"].remove("EQ")
                bestHomoPara["gpes"][i][gpeCmpCol]["operations"].remove("LT")
                bestHomoPara["gpes"][i][gpeCmpCol]["operations"].remove("LE")
    if(logicPos==-1):
        if(firstStride):
            for r in range(row):
                for c in range(col):
                    pass
                    # bestHomoPara["gpes"][r][c]["operations"].remove("AND")
                    # bestHomoPara["gpes"][r][c]["operations"].remove("OR")
                    # bestHomoPara["gpes"][r][c]["operations"].remove("XOR")
    else:
        if("AND" in bestHomoPara["gpes"][gpeLogUpBound][gpeLogCol]["operations"] and judge[2][3]==1):
            for i in range(gpeLogUpBound, gpeLogDownBound):
                bestHomoPara["gpes"][i][gpeLogCol]["operations"].remove("AND")
                bestHomoPara["gpes"][i][gpeLogCol]["operations"].remove("OR")
                bestHomoPara["gpes"][i][gpeLogCol]["operations"].remove("XOR")
    if(shiftPos==-1):
        if(firstStride):
            for r in range(row):
                for c in range(col):
                    pass
                    # bestHomoPara["gpes"][r][c]["operations"].remove("SHL")
                    # bestHomoPara["gpes"][r][c]["operations"].remove("LSHR")
                    # bestHomoPara["gpes"][r][c]["operations"].remove("ASHR")
    else:
        if("SHL" in bestHomoPara["gpes"][gpeShiUpBound][gpeShiCol]["operations"] and judge[2][4]==1):
            for i in range(gpeShiUpBound, gpeShiDownBound):
                bestHomoPara["gpes"][i][gpeShiCol]["operations"].remove("SHL")
                bestHomoPara["gpes"][i][gpeShiCol]["operations"].remove("LSHR")
                bestHomoPara["gpes"][i][gpeShiCol]["operations"].remove("ASHR")
    if(bestHomoPara["gpes"][gpeDelUpBound][gpeDelCol]["max_delay"] > 1 and judge[0][0]==1):
        for i in range(gpeDelUpBound, gpeDelDownBound):
            bestHomoPara["gpes"][i][gpeDelCol]["max_delay"] -= 1

    return bestHomoPara

def paramSpace(trial:optuna.Trial, usage:dict) -> list:
    
    lsuDelay = trial.suggest_int('lsuDelay', 0, 0)
    gpeDelay = trial.suggest_int('gpeDelay', 0, 1)
    num_ipin_side1 = trial.suggest_int('num_ipin_side1', 0, 0)
    num_ipin_side2 = trial.suggest_int('num_ipin_side2', 0, 0)
    num_ipin_side3 = trial.suggest_int('num_ipin_side3', 0, 0)
    num_otrack_side1 = trial.suggest_int('num_otrack_side1', 0, 0)
    num_otrack_side2 = trial.suggest_int('num_otrack_side2', 0, 0)
    num_otrack_side3 = trial.suggest_int('num_otrack_side3', 0, 0)
    num_otrack_side4 = trial.suggest_int('num_otrack_side4', 0, 0)
    num_itrack_side1 = trial.suggest_int('num_itrack_side1', 0, 0)
    num_itrack_side2 = trial.suggest_int('num_itrack_side2', 0, 0)
    num_itrack_side3 = trial.suggest_int('num_itrack_side3', 0, 0)
    num_itrack_side4 = trial.suggest_int('num_itrack_side4', 0, 0)
    add = trial.suggest_int('add', 0, 0) if(usage["ADDSUB"][0]!=-1) else 0   
    mul = trial.suggest_int('mul', 0, 1) if(usage["MUL"][0]!=-1) else 0
    cmp = trial.suggest_int('cmp', 0, 0) if(usage["CMP"][0]!=-1) else 0
    log = trial.suggest_int('log', 0, 0) if(usage["LOGIC"][0]!=-1) else 0
    shi = trial.suggest_int('shi', 0, 0) if(usage["SHIFT"][0]!=-1) else 0

    delay = [gpeDelay, lsuDelay]
    gib = [num_ipin_side1, num_ipin_side2, num_ipin_side3,
           num_otrack_side1, num_otrack_side2, num_otrack_side3, num_otrack_side4,
           num_itrack_side1, num_itrack_side2, num_itrack_side3, num_itrack_side4]
    gpe = [add, mul, cmp, log, shi]
    judge = [delay, gib, gpe]
    return judge

# def oldEva(oldeva:list) -> float:
    
#     area = oldeva[0]
#     time = oldeva[1]
#     ii = oldeva[2]
#     mappingFailureRate = oldeva[3]
#     weight = [100,100000,8000000,10000000]
#     comprehensive = weight[0]*area+weight[1]*time+weight[2]*ii+weight[3]*mappingFailureRate
#     return comprehensive

def dict2List(cutDict:dict) -> list:

    lsuDelay = cutDict["lsuDelay"]
    gpeDelay = cutDict["gpeDelay"]
    num_ipin_side1 = cutDict["num_ipin_side1"]
    num_ipin_side2 = cutDict["num_ipin_side2"]
    num_ipin_side3 = cutDict["num_ipin_side3"]
    num_otrack_side1 = cutDict["num_otrack_side1"]
    num_otrack_side2 = cutDict["num_otrack_side2"]
    num_otrack_side3 = cutDict["num_otrack_side3"]
    num_otrack_side4 = cutDict["num_otrack_side4"]
    num_itrack_side1 = cutDict["num_itrack_side1"]
    num_itrack_side2 = cutDict["num_itrack_side2"]
    num_itrack_side3 = cutDict["num_itrack_side3"]
    num_itrack_side4 = cutDict["num_itrack_side4"]
    if 'add' in cutDict :
        add = cutDict["add"] 
    else:
        add = 0
    if 'mul' in cutDict :
        mul = cutDict["mul"]
    else:
        mul = 0
    if 'cmp' in cutDict :
        cmp = cutDict["cmp"] 
    else:
        cmp = 0
    if 'log' in cutDict :
        log = cutDict["log"]
    else:
        log = 0
    if 'shi' in cutDict :
        shi = cutDict["shi"]
    else:
        shi = 0

    delay = [gpeDelay, lsuDelay]
    gib = [num_ipin_side1, num_ipin_side2, num_ipin_side3,
           num_otrack_side1, num_otrack_side2, num_otrack_side3, num_otrack_side4,
           num_itrack_side1, num_itrack_side2, num_itrack_side3, num_itrack_side4]
    gpe = [add, mul, cmp, log, shi]
    judgeList = [delay, gib, gpe]
    return judgeList

def findWorseStride() -> int:

    with open('Usage.txt', 'r') as file:
        content = file.readlines()
    numbers = []
    for line in content:
        line = line.strip()  
        if ':' in line:
            line = line.split(':')[1]  
            numbers += [int(num) for num in line.split()]
    max_number = max(numbers)
    # print("WorseTryStride: ", max_number)
    return max_number//2

def objective(trial: optuna.Trial) -> float:
    global strideUsage
    bestHomoPara = readHomoArch()
    # bestHomoEva = readHomoEva()
    # print(bestHomoEva)
    usage = readHomoUsage()
    judge = paramSpace(trial, usage)
    hetePara = cutSpec(bestHomoPara, usage, judge)
    specGenerate(hetePara)
    run_hete_cgra()
    
    with open('../cgra-compiler/result/Usage.txt', 'r') as file:
        content = file.read()
        strideUsage[trial.number] = content

    evaNew = usrWeighted()
    print(evaNew)

    return evaNew[0]

def newBest(cutKey:tuple):
    bestHomoPara = readHomoArch()
    usage = readHomoUsage()
    cutList = dict2List(cutKey[1])
    newPara = cutSpec(bestHomoPara, usage, cutList)
    with open('cgra_spec_bestHomo.json', 'w') as f:
        json.dump(newPara, f, indent=4)
    with open('cgra_eva_bestHomo.json', 'w') as f:
        json.dump(cutKey[0], f, indent=4)

def stride(maxIt:int) -> tuple:
    global firstStride,worseStride
    if(firstStride):
        subprocess.run('cp ../cgra-compiler/result/Usage.txt .',shell=True).returncode
        worseStride = findWorseStride()
    optuna.logging.get_logger("optuna").addHandler(logging.StreamHandler(sys.stdout))
    study_name = "hete"
    sampler = optuna.samplers.TPESampler(multivariate=True)
    #sampler = optuna.samplers.QMCSampler()
    #sampler = optuna.samplers.RandomSampler()
    study = optuna.create_study(sampler=sampler, directions=["minimize"],study_name=study_name,load_if_exists=True)
    study.optimize(objective, n_trials=maxIt, show_progress_bar=True)

    return (study.best_value, study.best_params,study.best_trial.number)

i = 0
tryWorse = 0
while True:
    evaOld = readHomoEva()
    oneStrideResult = stride(4)
    i += 1

    if((oneStrideResult[0]-evaOld)>0):
        if(tryWorse!=worseStride):
            with open('Usage.txt', 'r') as file:
                lines = file.readlines()
            new_lines = []
            for line in lines:
                line_data = line.split()
                print(line_data)
                if line_data[1] != "-1" and len(line_data) > 2:
                    line_data.pop(1) 
                new_lines.append(' '.join(line_data) + '\n') 
            with open('Usage.txt', 'w') as file:
                file.writelines(new_lines)
            tryWorse += 1
            continue
        else:
            break
    else:
        output_data = strideUsage[oneStrideResult[2]]
        output_file = "Usage.txt"
        with open(output_file, 'w') as file:
            file.write(output_data)
        print(oneStrideResult[1])
        print(oneStrideResult[0])
        print(oneStrideResult[2])
        newBest(oneStrideResult)
        firstStride = False
        tryWorse = 0
        strideUsage = {}
        print("-------Next Stride Begin-------")
        continue

print('Total Strides: %d', i)
print('\n <<<<< Exploration finished >>>>>')
end_time = time.time()
print('\n Total Time = '+str(end_time-start_time)+'\n')
#169610200.21