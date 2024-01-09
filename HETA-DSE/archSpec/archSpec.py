import subprocess
import json

def formatSpec(sample : dict) -> dict:
    opin_2_ipin_connect_flexibility = {
        'num_ipin_side1' : sample['num_ipin_side1'],
        'num_ipin_side2' : sample['num_ipin_side2'],
        'num_ipin_side3' : sample['num_ipin_side3']
    }
    del sample['num_ipin_side1']
    del sample['num_ipin_side2']
    del sample['num_ipin_side3']
    sample['opin_2_ipin_connect_flexibility'] = opin_2_ipin_connect_flexibility

    opin_2_otrack_connect_flexibility = {
        'num_otrack_side1' : sample['num_otrack_side1'],
        'num_otrack_side2' : sample['num_otrack_side2'],
        'num_otrack_side3' : sample['num_otrack_side3'],
        'num_otrack_side4' : sample['num_otrack_side4']
    }
    del sample['num_otrack_side1']
    del sample['num_otrack_side2']
    del sample['num_otrack_side3']
    del sample['num_otrack_side4']
    sample['opin_2_otrack_connect_flexibility'] = opin_2_otrack_connect_flexibility

    itrack_2_ipin_connect_flexibility = {
        'num_itrack_side1' : sample['num_itrack_side1'],
        'num_itrack_side2' : sample['num_itrack_side2'],
        'num_itrack_side3' : sample['num_itrack_side3'],
        'num_itrack_side4' : sample['num_itrack_side4']
    }
    del sample['num_itrack_side1']
    del sample['num_itrack_side2']
    del sample['num_itrack_side3']
    del sample['num_itrack_side4']
    sample['itrack_2_ipin_connect_flexibility'] = itrack_2_ipin_connect_flexibility

    row = sample["num_row"]
    col = sample["num_colum"]
    opin_2_ipin_connect_flexibility = sample['opin_2_ipin_connect_flexibility']
    opin_2_otrack_connect_flexibility = sample['opin_2_otrack_connect_flexibility']
    itrack_2_ipin_connect_flexibility = sample['itrack_2_ipin_connect_flexibility']
    num_rf_reg = sample['num_rf_reg']
    operations = sample['operations']
    max_delay = sample['max_delay']
    memory_depth = sample['memory_depth']
    num_mem_perin = sample['num_mem_perin']
    num_mem_out = sample['num_mem_out']
    GIB = {"gibs":addGIB(row,col,opin_2_ipin_connect_flexibility,
                         opin_2_otrack_connect_flexibility,
                         itrack_2_ipin_connect_flexibility)}
    sample.update(**GIB)
    GPE = {"gpes":addGPE(row,col,num_rf_reg,operations,max_delay)}
    sample.update(**GPE)
    LSU = {"lsus":addLSU(row,max_delay,memory_depth,num_mem_perin,num_mem_out)}
    sample.update(**LSU)

    return sample

def addGIB(row,col,opin_2_ipin_connect_flexibility,
           opin_2_otrack_connect_flexibility,itrack_2_ipin_connect_flexibility):
    gib = []
    for i in range(row+1):
        gib.append([])
        for j in range(col+1):
            gib[i].append({"opin_2_ipin_connect_flexibility" : opin_2_ipin_connect_flexibility,
                           "opin_2_otrack_connect_flexibility" : opin_2_otrack_connect_flexibility,
                           "itrack_2_ipin_connect_flexibility" : itrack_2_ipin_connect_flexibility})
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
    if(detect1==1):raise TypeError
    detect2 = subprocess.run('cp ../cgra-mg/area.txt ./sampleEva',shell=True).returncode
    if(detect2==1):raise TypeError
    detect3 = subprocess.run('rm ../cgra-mg/area.txt',shell=True).returncode
    if(detect3==1):raise TypeError
    detect4 = subprocess.run('cd ../cgra-compiler && ./run.sh', shell=True).returncode
    
    detect5 = subprocess.run('cp ../cgra-compiler/result/time.txt ./sampleEva',shell=True).returncode
    if(detect5==1):raise TypeError
    detect6 = subprocess.run('rm ../cgra-compiler/result/time.txt',shell=True).returncode
    if(detect6==1):raise TypeError
    detect7 = subprocess.run('cp ../cgra-compiler/result/ii.txt ./sampleEva',shell=True).returncode
    if(detect7==1):raise TypeError
    detect8 = subprocess.run('rm ../cgra-compiler/result/ii.txt',shell=True).returncode
    if(detect8==1):raise TypeError
    detect9 = subprocess.run('cp ../cgra-compiler/result/mappingFailureRate.txt ./sampleEva',shell=True).returncode
    if(detect9==1):raise TypeError
    detect10 = subprocess.run('rm ../cgra-compiler/result/mappingFailureRate.txt',shell=True).returncode
    if(detect10==1):raise TypeError

def run_hete():
    detect1 = subprocess.run('cd ../cgra-mg && ./run.sh', shell=True).returncode
    if(detect1==1):raise TypeError
    detect2 = subprocess.run('cp ../cgra-mg/area.txt ./sampleEva',shell=True).returncode
    if(detect2==1):raise TypeError
    detect3 = subprocess.run('rm ../cgra-mg/area.txt',shell=True).returncode
    if(detect3==1):raise TypeError
    detect4 = subprocess.run('cd ../cgra-compiler && ./run_hete.sh', shell=True).returncode
    
    detect5 = subprocess.run('cp ../cgra-compiler/result/time.txt ./sampleEva',shell=True).returncode
    if(detect5==1):raise TypeError
    detect6 = subprocess.run('rm ../cgra-compiler/result/time.txt',shell=True).returncode
    if(detect6==1):raise TypeError
    detect7 = subprocess.run('cp ../cgra-compiler/result/ii.txt ./sampleEva',shell=True).returncode
    if(detect7==1):raise TypeError
    detect8 = subprocess.run('rm ../cgra-compiler/result/ii.txt',shell=True).returncode
    if(detect8==1):raise TypeError
    detect9 = subprocess.run('cp ../cgra-compiler/result/mappingFailureRate.txt ./sampleEva',shell=True).returncode
    if(detect9==1):raise TypeError
    detect10 = subprocess.run('rm ../cgra-compiler/result/mappingFailureRate.txt',shell=True).returncode
    if(detect10==1):raise TypeError


def run_cgra_flow():
    print('---------------------------Design Space Exploring---------------------------')
    try:
        run_cgra()
    except TypeError:
        print("--------RUNNING ERROR--------")
        with open('./sampleEva/area.txt', 'w') as a:
            a.write("5000000")
        with open('./sampleEva/time.txt', 'w') as p:   
            p.write("100")
        with open('./sampleEva/ii.txt', 'w') as l:   
            l.write("100")
        with open('./sampleEva/mappingFailureRate.txt', 'w') as m:   
            m.write("1")

def run_hete_cgra():
    print('---------------------------Design Space Exploring---------------------------')
    try:
        run_hete()
    except TypeError:
        print("--------RUNNING ERROR--------")
        with open('./sampleEva/area.txt', 'w') as a:
            a.write("5000000")
        with open('./sampleEva/time.txt', 'w') as p:   
            p.write("100")
        with open('./sampleEva/ii.txt', 'w') as l:   
            l.write("100")
        with open('./sampleEva/mappingFailureRate.txt', 'w') as m:   
            m.write("1")

def specGenerate(spec:dict):
    try:
        #re1 = subprocess.run('rm ../cgra-mg/src/main/resources/cgra_spec.json',shell=True).returncode
        with open('cgra_spec.json', 'w') as f:
            json.dump(spec, f, indent=4)
        re2 = subprocess.run('cp cgra_spec.json ../cgra-mg/src/main/resources/', shell=True)
        if(re2==1): raise TypeError
    except TypeError:
        print("cgra_spec file generation error!")