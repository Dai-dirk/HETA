import pandas as pd
import numpy  as np
import json
import subprocess
import scipy as sp
import torch
import matplotlib.pyplot as plt
import time
from pickle import TRUE
from unicodedata import decimal
from xml.sax import make_parser
from click import option
from numpy import *
from hebo.design_space.design_space import DesignSpace
from hebo.optimizers.hebo import HEBO
from pymoo.factory import get_problem
from pymoo.util.plotting import plot
from pymoo.util.dominator import Dominator
from hebo.optimizers.general import GeneralBO


start_time = time.time()

allSample = {}

def obj(params : pd.DataFrame) -> np.ndarray:

    with open('area.txt', 'r') as a:
        area = float(a.read().strip())
    with open('time.txt', 'r') as t:   
        time = float(t.read().strip())
    with open('ii.txt', 'r') as i:   
        ii = float(i.read().strip())
    with open('mappingFailureRate.txt', 'r') as m:   
        mappingFailureRate = float(m.read().strip())

    x = 100
    y = 100000
    z = 8000000
    k = 10000000

    comprehensive = x*area+y*time+z*ii+k*mappingFailureRate

    return np.array(comprehensive).reshape(-1, 1)
 

def formResult(spec):
    with open('area.txt', 'r') as a:
        area = float(a.read().strip())
    with open('time.txt', 'r') as t:   
        time = float(t.read().strip())
    with open('ii.txt', 'r') as i:   
        ii = float(i.read().strip())
    with open('mappingFailureRate.txt', 'r') as m:   
        mappingFailureRate = float(m.read().strip())

    x = 100
    y = 100000
    z = 8000000
    k = 10000000
    
    key = x*area+y*time+z*ii+k*mappingFailureRate
    allSample[key] = spec
    restoreResult = [key]
    result = [area,time,ii,mappingFailureRate]
    return (restoreResult,result)


params = [
    {'name' : 'num_row',                     'type' : 'int', 'lb' : 4,  'ub' : 8  },
    {'name' : 'num_colum',                   'type' : 'int', 'lb' : 4,  'ub' : 8  },
    {'name' : 'num_output_ib',               'type' : 'int', 'lb' : 1,  'ub' : 1  },
    {'name' : 'num_input_ib',                'type' : 'int', 'lb' : 1,  'ub' : 1  }, 
    {'name' : 'num_input_ob',                'type' : 'int', 'lb' : 2,  'ub' : 2  },
    {'name' : 'num_output_ob',               'type' : 'int', 'lb' : 1,  'ub' : 1  },
    {'name' : 'num_track',                   'type' : 'int', 'lb' : 1,  'ub' : 3  },
    {'name' : 'max_delay',                   'type' : 'int', 'lb' : 1,  'ub' : 4 },
    {'name' : 'num_perside_otrack_per_opin', 'type' : 'int', 'lb' : 1,  'ub' : 6  },
    {'name' : 'num_perside_itrack_per_ipin', 'type' : 'int', 'lb' : 1,  'ub' : 6  },
    {'name' : 'num_perside_ipin_per_opin',   'type' : 'int', 'lb' : 0,  'ub' : 2  },
    {'name' : 'connect_2_side1_p2p',         'type' : 'bool' },
    {'name' : 'connect_2_side2_p2p',         'type' : 'bool' },
    {'name' : 'connect_2_side3_p2p',         'type' : 'bool' },
    {'name' : 'connect_2_side1_p2t',         'type' : 'bool' },
    {'name' : 'connect_2_side2_p2t',         'type' : 'bool' },
    {'name' : 'connect_2_side3_p2t',         'type' : 'bool' },
    {'name' : 'connect_2_side4_p2t',         'type' : 'bool' },
]
# params = [
#     {'name' : 'num_row',                     'type' : 'int', 'lb' : 6,  'ub' : 6  },
#     {'name' : 'num_colum',                   'type' : 'int', 'lb' : 4,  'ub' : 4  },
#     {'name' : 'num_output_ib',               'type' : 'int', 'lb' : 1,  'ub' : 1  },
#     {'name' : 'num_input_ib',                'type' : 'int', 'lb' : 1,  'ub' : 1  }, 
#     {'name' : 'num_input_ob',                'type' : 'int', 'lb' : 2,  'ub' : 2  },
#     {'name' : 'num_output_ob',               'type' : 'int', 'lb' : 1,  'ub' : 1  },
#     {'name' : 'num_track',                   'type' : 'int', 'lb' : 1,  'ub' : 1  },
#     {'name' : 'max_delay',                   'type' : 'int', 'lb' : 1,  'ub' : 1  },
#     {'name' : 'num_perside_otrack_per_opin', 'type' : 'int', 'lb' : 2,  'ub' : 2  },
#     {'name' : 'num_perside_itrack_per_ipin', 'type' : 'int', 'lb' : 2,  'ub' : 2  },
#     {'name' : 'num_perside_ipin_per_opin',   'type' : 'int', 'lb' : 2,  'ub' : 2  }
# ]

op = {  
        "track_reged_mode" : 1,
        "num_rf_reg"       : 1,
        "data_width"       : 32,
        "memory_depth"     : 64,
        "num_mem_perin"    : 1,
        "num_mem_out"      : 1,
        "max_II"           : 8,
        "cfg_data_width"   : 32,
        "cfg_addr_width"   : 18,
        "cfg_blk_offset"   : 2,
        "operations"       : ["PASS", "ADD", "SUB", "MUL", "AND", "OR", "XOR"]
        # "connect_2_side1_p2p":True,
        # "connect_2_side2_p2p":True,
        # "connect_2_side3_p2p":True,
        # "connect_2_side1_p2t":True,
        # "connect_2_side2_p2t":True,
        # "connect_2_side3_p2t":True,
        # "connect_2_side4_p2t":True
    }


def formatSpec(sample : dict) -> dict:
    connect_flexibility = {
        'num_perside_otrack_per_opin' : sample['num_perside_otrack_per_opin'],
        'num_perside_itrack_per_ipin' : sample['num_perside_itrack_per_ipin'],
        'num_perside_ipin_per_opin'   : sample['num_perside_ipin_per_opin']
    }
    del sample['num_perside_otrack_per_opin']
    del sample['num_perside_itrack_per_ipin']
    del sample['num_perside_ipin_per_opin']
    sample['connect_flexibility'] = connect_flexibility

    pin_2_pin_connection = {
        'connect_2_side1' : sample['connect_2_side1_p2p'],
        'connect_2_side2' : sample['connect_2_side2_p2p'],
        'connect_2_side3' : sample['connect_2_side3_p2p']
    }
    del sample['connect_2_side1_p2p']
    del sample['connect_2_side2_p2p']
    del sample['connect_2_side3_p2p']
    sample['pin_2_pin_connection'] = pin_2_pin_connection

    pin_2_track_connection = {
        'connect_2_side1' : sample['connect_2_side1_p2t'],
        'connect_2_side2' : sample['connect_2_side2_p2t'],
        'connect_2_side3' : sample['connect_2_side3_p2t'],
        'connect_2_side4' : sample['connect_2_side4_p2t']
    }
    del sample['connect_2_side1_p2t']
    del sample['connect_2_side2_p2t']
    del sample['connect_2_side3_p2t']
    del sample['connect_2_side4_p2t']
    sample['pin_2_track_connection'] = pin_2_track_connection

    return sample

def addGIB(row,col,connect_flexibility,pin_2_pin_connection,pin_2_track_connection):
    gib = []
    for i in range(row+1):
        gib.append([])
        for j in range(col+1):
            gib[i].append({"connect_flexibility"    : connect_flexibility,
                           "pin_2_pin_connection"   : pin_2_pin_connection,
                           "pin_2_track_connection" : pin_2_track_connection})
    return gib


def addGPE(row,col,num_rf_reg,operations,max_delay):
    gpe = []
    for i in range(row):
        gpe.append([])
        for j in range(col):
            gpe[i].append({"num_rf_reg"  : num_rf_reg,
                           "operations"  : operations,
                           "max_delay"   : max_delay,
                           "from_dir"    : [4,5,7,6],
                           "to_dir"      : [4,5,7,6]})
    return gpe

def addLSU(row,max_delay,memory_depth,num_mem_perin,num_mem_out):
    lsu = []
    for i in range(row):
        lsu.append([])
        for j in range(2):
            lsu[i].append({"max_delay" : max_delay,
                         "memory_depth" : memory_depth,
                         "num_mem_perin" : num_mem_perin,
                         "num_mem_out" : num_mem_out})
    return lsu

def run_cgra():
    detect1 = subprocess.run('cd ../cgra-mg && ./run.sh', shell=True).returncode
    detect2 = subprocess.run('cp ../cgra-mg/area.txt .',shell=True).returncode
    detect3 = subprocess.run('rm ../cgra-mg/area.txt',shell=True).returncode
    detect4 = subprocess.run('cd ../cgra-compiler && ./run.sh', shell=True).returncode
    detect5 = subprocess.run('cp ../cgra-compiler/result/time.txt .',shell=True).returncode
    detect6 = subprocess.run('rm ../cgra-compiler/result/time.txt',shell=True).returncode
    detect7 = subprocess.run('cp ../cgra-compiler/result/ii.txt .',shell=True).returncode
    detect8 = subprocess.run('rm ../cgra-compiler/result/ii.txt',shell=True).returncode
    detect9 = subprocess.run('cp ../cgra-compiler/result/mappingFailureRate.txt .',shell=True).returncode
    detect10 = subprocess.run('rm ../cgra-compiler/result/mappingFailureRate.txt',shell=True).returncode
    if (detect1==1 or detect2==1 or detect3==1 or detect5==1 or detect6==1 or 
    detect7==1 or detect8==1 or detect9==1 or detect10==1): 
        raise TypeError
        


space = DesignSpace().parse(params)

opt   = HEBO(space)

print('  <<<<< design space exploration begin >>>>>')

result = []
    
y = []
keyAndSignleValue = []

it = 50
validNum = 0
for i in range(it):
    rec = opt.suggest(n_suggestions=1)
    sample = rec.to_dict('records')[0]
    sample.update(**op)
    spec = formatSpec(sample)
    #spec.update(**op)

    # track_coarse = spec['num_track']
    # if(spec["connect_flexibility"]["num_perside_otrack_per_opin"] <= 2*track_coarse):
    #     if(spec["connect_flexibility"]["num_perside_otrack_per_opin"]%2 != 0):
    #         spec["connect_flexibility"]["num_perside_otrack_per_opin"] = spec["connect_flexibility"]["num_perside_otrack_per_opin"] - 1
    # else:
    #     spec["connect_flexibility"]["num_perside_otrack_per_opin"] = 2*track_coarse

    # if(spec["connect_flexibility"]["num_perside_itrack_per_ipin"] <= 2*track_coarse):
    #     if(spec["connect_flexibility"]["num_perside_itrack_per_ipin"]%2 != 0):
    #         spec["connect_flexibility"]["num_perside_itrack_per_ipin"] = spec["connect_flexibility"]["num_perside_itrack_per_ipin"] - 1
    # else:
    #     spec["connect_flexibility"]["num_perside_itrack_per_ipin"] = 2*track_coarse

    row = spec["num_row"]
    col = spec["num_colum"]
    connect_flexibility = spec['connect_flexibility']
    pin_2_pin_connection = spec['pin_2_pin_connection']
    pin_2_track_connection = spec['pin_2_track_connection']
    num_rf_reg = spec['num_rf_reg']
    operations = spec['operations']
    max_delay = spec['max_delay']
    memory_depth = spec['memory_depth']
    num_mem_perin = spec['num_mem_perin']
    num_mem_out = spec['num_mem_out']
    GIB = {"gibs":addGIB(row,col,connect_flexibility,pin_2_pin_connection,pin_2_track_connection)}
    spec.update(**GIB)
    GPE = {"gpes":addGPE(row,col,num_rf_reg,operations,max_delay)}
    spec.update(**GPE)
    LSU = {"lsus":addLSU(row,max_delay,memory_depth,num_mem_perin,num_mem_out)}
    spec.update(**LSU)

    try:
        re1 = subprocess.run('rm ../cgra-mg/src/main/resources/cgra_spec.json',shell=True).returncode
        with open('cgra_spec.json', 'w') as f:
            json.dump(spec, f, indent=4)
        re2 = subprocess.run('cp cgra_spec.json ../cgra-mg/src/main/resources/', shell=True)
        if(re1==1 or re2==1): raise TypeError
    except TypeError:
        print("cgra_spec file generation error!")
        
    try:
        run_cgra()
    except TypeError:
        print("--------RUNNING ERROR--------")
        # with open('area.txt', 'w') as a:
        #     a.write("10000000")
        with open('time.txt', 'w') as p:   
            p.write("100")
        with open('ii.txt', 'w') as l:   
            l.write("100")
        with open('mappingFailureRate.txt', 'w') as m:   
            m.write("1")
    
        opt.observe(rec, obj(rec))

        restoreResult = formResult(spec)[0]
        
        if(i==0):
            y.append(restoreResult[0])
            keyAndSignleValue.append(formResult(spec))
        elif(restoreResult[0] < opt.y.min()):
            y.append(restoreResult[0])
            keyAndSignleValue.append(formResult(spec))
        else:
            y.append( opt.y.min() )
        
        continue

    
    opt.observe(rec, obj(rec))

    restoreResult = formResult(spec)[0]
    
    if(i==0):
        y.append(restoreResult[0])
        keyAndSignleValue.append(formResult(spec))
    elif(restoreResult[0] < opt.y.min()):
        y.append(restoreResult[0])
        keyAndSignleValue.append(formResult(spec))
    else:
        y.append( opt.y.min() )
    
    print('After %d iterations, best obj is %.2f' % (i, opt.y.min()))
    keyAndSignleValue.append(formResult(spec))
    validNum += 1


resultKey = opt.y.min()
if(resultKey.size!=0):
    print(allSample[resultKey.astype(np.float)])
    # with open('cgra_spec.json', 'w') as f:
    #     json.dump(allSample[resultKey.astype(np.float)], f, indent=4)
    for j in range(len(keyAndSignleValue)):
        if(keyAndSignleValue[j][0]==resultKey):
            print(keyAndSignleValue[j][1])
        else:
            continue
else:
    print("no valid arch reached")

print('\n the number of valid para iters are:%d' %validNum)


# print('\n <<<<< start optimizing >>>>>')
# optIter = 0
# finalArea = 0.0
# finalTime = 0.0
# finalII = 0.0
# finalMappingFailureRate = 0.0
# beforeOpt = allSample[resultKey.astype(np.float)]
# if(resultKey.size!=0):
#     while(1):
#         beforeOpt['num_row'] = beforeOpt['num_row'] - 1
#         with open('cgra_spec.json', 'w') as f:
#             json.dump(beforeOpt, f, indent=4)
#         subprocess.run('cp cgra_spec.json ../cgra-mg/src/main/resources/', shell=True)

#         try:
#             run_cgra()
#             with open('area.txt', 'r') as a:
#                 finalArea = float(a.read().strip())
#             with open('time.txt', 'r') as t:   
#                 finalTime = float(t.read().strip())
#             with open('ii.txt', 'r') as i:   
#                 finalII = float(i.read().strip())
#             with open('mappingFailureRate.txt', 'r') as m:   
#                 finalMappingFailureRate = float(m.read().strip())
#             optIter += 1
            
#         except TypeError:
#             beforeOpt['num_row'] = beforeOpt['num_row'] + 1
#             break
    
#     while(1):
#         beforeOpt['num_colum'] = beforeOpt['num_colum'] - 1
#         with open('cgra_spec.json', 'w') as f:
#             json.dump(beforeOpt, f, indent=4)
#         subprocess.run('cp cgra_spec.json ../cgra-mg/src/main/resources/', shell=True)

#         try:
#             run_cgra()
#             with open('area.txt', 'r') as a:
#                 finalArea = float(a.read().strip())
#             with open('time.txt', 'r') as t:   
#                 finalTime = float(t.read().strip())
#             with open('ii.txt', 'r') as i:   
#                 finalII = float(i.read().strip())
#             with open('mappingFailureRate.txt', 'r') as m:   
#                 finalMappingFailureRate = float(m.read().strip())
#             optIter += 1
#         except TypeError:
#             beforeOpt['num_colum'] = beforeOpt['num_colum'] + 1
#             break

#     while(1):
#         beforeOpt['num_track'] = beforeOpt['num_track'] - 1
#         with open('cgra_spec.json', 'w') as f:
#             json.dump(beforeOpt, f, indent=4)
#         subprocess.run('cp cgra_spec.json ../cgra-mg/src/main/resources/', shell=True)

#         try:
#             run_cgra()
#             with open('area.txt', 'r') as a:
#                 finalArea = float(a.read().strip())
#             with open('time.txt', 'r') as t:   
#                 finalTime = float(t.read().strip())
#             with open('ii.txt', 'r') as i:   
#                 finalII = float(i.read().strip())
#             with open('mappingFailureRate.txt', 'r') as m:   
#                 finalMappingFailureRate = float(m.read().strip())
#             optIter += 1
#         except TypeError:
#             beforeOpt['num_track'] = beforeOpt['num_track'] + 1
#             break


#     if(optIter!=0):
#         print('\nThe optimal parameters are as follows:')
#         num_track = beforeOpt['num_track']
#         if(beforeOpt["connect_flexibility"]["num_perside_otrack_per_opin"] >= 2*num_track):
#             beforeOpt["connect_flexibility"]["num_perside_otrack_per_opin"] = 2*num_track
#         if(beforeOpt["connect_flexibility"]["num_perside_itrack_per_ipin"] >= 2*num_track):
#             beforeOpt["connect_flexibility"]["num_perside_itrack_per_ipin"] = 2*num_track

#         print(beforeOpt)
#         print('the area is: %f'%finalArea)
#         print('the mapping time is: %f'%finalTime)
#         print('the II is: %f'%finalII)
#         print('the fail rate is:%f'%(finalMappingFailureRate))
#     else:
#         print('\nThe optimal parameters are as follows:')
#         print(allSample[resultKey.astype(np.float)])
#         for j in range(len(keyAndSignleValue)):
#             if(keyAndSignleValue[j][0]==resultKey):
#                 print('the area is: %f'%keyAndSignleValue[j][1][0])
#                 print('the mapping time is: %f'%(1-keyAndSignleValue[j][1][1]))
#                 print('the II is: %f'%keyAndSignleValue[j][1][2])
#                 print('the fail rate is:%f'%(keyAndSignleValue[j][1][3]))
#             else:
#                 continue
# else:
#     print("no valid arch reached, there is no need to optimize")


print('\n <<<<< design space exploration finished >>>>>')
end_time = time.time()
print('\n Total Time = '+str(end_time-start_time)+'\n')


