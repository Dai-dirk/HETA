import optuna

def paramspace(trial: optuna.Trial) -> dict:
    num_row = trial.suggest_int('num_row', 4, 4)
    num_colum = trial.suggest_int('num_colum', 6, 6)
    num_track = trial.suggest_int('num_track', 1, 1)
    max_delay = trial.suggest_int('max_delay', 7, 7)
    num_ipin_side1 = trial.suggest_int('num_ipin_side1', 1, 1)
    num_ipin_side2 = trial.suggest_int('num_ipin_side2', 1, 1)
    num_ipin_side3 = trial.suggest_int('num_ipin_side3', 0, 0)
    num_otrack_side1 = trial.suggest_int('num_otrack_side1', 1, 1)
    num_otrack_side2 = trial.suggest_int('num_otrack_side2', 1, 1)
    num_otrack_side3 = trial.suggest_int('num_otrack_side3', 1, 1)
    num_otrack_side4 = trial.suggest_int('num_otrack_side4', 0, 0)
    num_itrack_side1 = trial.suggest_int('num_itrack_side1', 0, 0)
    num_itrack_side2 = trial.suggest_int('num_itrack_side2', 1, 1)
    num_itrack_side3 = trial.suggest_int('num_itrack_side3', 1, 1)
    num_itrack_side4 = trial.suggest_int('num_itrack_side4', 1, 1)
    # num_row = trial.suggest_int('num_row', 4, 6)
    # num_colum = trial.suggest_int('num_colum', 4, 6)
    # num_track = trial.suggest_int('num_track', 1, 4)
    # max_delay = trial.suggest_int('max_delay', 3, 7)
    # num_ipin_side1 = trial.suggest_int('num_ipin_side1', 0, 2)
    # num_ipin_side2 = trial.suggest_int('num_ipin_side2', 0, 2)
    # num_ipin_side3 = trial.suggest_int('num_ipin_side3', 0, 2)
    # num_otrack_side1 = trial.suggest_int('num_otrack_side1', 0, 4)
    # num_otrack_side2 = trial.suggest_int('num_otrack_side2', 0, 4)
    # num_otrack_side3 = trial.suggest_int('num_otrack_side3', 0, 4)
    # num_otrack_side4 = trial.suggest_int('num_otrack_side4', 0, 4)
    # num_itrack_side1 = trial.suggest_int('num_itrack_side1', 0, 4)
    # num_itrack_side2 = trial.suggest_int('num_itrack_side2', 0, 4)
    # num_itrack_side3 = trial.suggest_int('num_itrack_side3', 0, 4)
    # num_itrack_side4 = trial.suggest_int('num_itrack_side4', 0, 4)
    
    num_otrack_side1 = num_track if num_otrack_side1>num_track else num_otrack_side1
    num_otrack_side2 = num_track if num_otrack_side2>num_track else num_otrack_side2
    num_otrack_side3 = num_track if num_otrack_side3>num_track else num_otrack_side3
    num_otrack_side4 = num_track if num_otrack_side4>num_track else num_otrack_side4
    num_itrack_side1 = num_track if num_itrack_side1>num_track else num_itrack_side1
    num_itrack_side2 = num_track if num_itrack_side2>num_track else num_itrack_side2
    num_itrack_side3 = num_track if num_itrack_side3>num_track else num_itrack_side3
    num_itrack_side4 = num_track if num_itrack_side4>num_track else num_itrack_side4
    
    para = {
        "num_row" : num_row,
        "num_colum" : num_colum,
        "num_track" : num_track,
        "max_delay" : max_delay,
        "num_ipin_side1" : num_ipin_side1,
        "num_ipin_side2" : num_ipin_side2,
        "num_ipin_side3" : num_ipin_side3,
        "num_otrack_side1" : num_otrack_side1,
        "num_otrack_side2" : num_otrack_side2,
        "num_otrack_side3" : num_otrack_side3,
        "num_otrack_side4" : num_otrack_side4,
        "num_itrack_side1" : num_itrack_side1,
        "num_itrack_side2" : num_itrack_side2,
        "num_itrack_side3" : num_itrack_side3,
        "num_itrack_side4" : num_itrack_side4,
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
        "operations"       : ["PASS", "ADD", "SUB", "MUL", "AND", "OR", "XOR", "EQ", "LT", "LE", "SHL", "LSHR", "ASHR"]
    }
    return para

def usrWeighted() -> tuple:
    with open('./sampleEva/area.txt', 'r') as a:
        area = float(a.read().strip())
    with open('./sampleEva/time.txt', 'r') as t:   
        time = float(t.read().strip())
    with open('./sampleEva/ii.txt', 'r') as i:   
        ii = float(i.read().strip())
    with open('./sampleEva/mappingFailureRate.txt', 'r') as m:   
        mappingFailureRate = float(m.read().strip())

    weight = [1,35000,62000,10000000]
    comprehensive = weight[0]*area+weight[1]*time+weight[2]*ii+weight[3]*mappingFailureRate
    result = [area, time, ii, mappingFailureRate]
    print(result)
    return (comprehensive,result)