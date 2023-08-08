import optuna
from optuna import Study
import json

def visual(obj : Study, evaluCGRA : list, buildFromExistMOdel : bool):
    df = obj.trials_dataframe(attrs=('value','params','state'))
    print(df)
    # fig1 = optuna.visualization.plot_optimization_history(obj)
    # fig2 = optuna.visualization.plot_parallel_coordinate(obj)
    # fig3 = optuna.visualization.plot_param_importances(obj)
    # fig1.show()
    # fig2.show()
    # fig3.show()
    best_params = obj.best_params
    if best_params["num_otrack_side1"] > best_params["num_track"]:
        best_params["num_otrack_side1"] = best_params["num_track"] 
    if best_params["num_otrack_side2"] > best_params["num_track"]:
        best_params["num_otrack_side2"] = best_params["num_track"] 
    if best_params["num_otrack_side3"] > best_params["num_track"]:
        best_params["num_otrack_side3"] = best_params["num_track"] 
    if best_params["num_otrack_side4"] > best_params["num_track"]:
        best_params["num_otrack_side4"] = best_params["num_track"] 
    if best_params["num_itrack_side1"] > best_params["num_track"]:
        best_params["num_itrack_side1"] = best_params["num_track"] 
    if best_params["num_itrack_side2"] > best_params["num_track"]:
        best_params["num_itrack_side2"] = best_params["num_track"] 
    if best_params["num_itrack_side3"] > best_params["num_track"]:
        best_params["num_itrack_side3"] = best_params["num_track"] 
    if best_params["num_itrack_side4"] > best_params["num_track"]:
        best_params["num_itrack_side4"] = best_params["num_track"] 
    print("\nThe Optimal Parameters are as follows: ")
    print(best_params)
    if buildFromExistMOdel :
        json_str = json.dumps(evaluCGRA)
        with open('history.json', 'w') as f:
            f.write(json_str)
        print("\nArea is : %d", evaluCGRA[obj.best_trial.number][0])
        print("Mapping time is : %d", evaluCGRA[obj.best_trial.number][1])
        print("II is : %d", evaluCGRA[obj.best_trial.number][2])
        print("Mapping Failture rate is : %d", evaluCGRA[obj.best_trial.number][3])
        with open('cgra_eva_bestHomo_decouple.json', 'w') as f:
            json.dump(evaluCGRA[obj.best_trial.number], f)
        with open('cgra_eva_bestHomo.json', 'w') as f:
            json.dump(obj.best_value, f)
    else:
        json_str = json.dumps(evaluCGRA)
        with open('history.json', 'w') as f:
            f.write(json_str)
        print("\nArea is : %d", evaluCGRA[obj.best_trial.number][0])
        print("Mapping time is : %d", evaluCGRA[obj.best_trial.number][1])
        print("II is : %d", evaluCGRA[obj.best_trial.number][2])
        print("Mapping Failture rate is : %d", evaluCGRA[obj.best_trial.number][3])
        with open('cgra_eva_bestHomo_decouple.json', 'w') as f:
            json.dump(evaluCGRA[obj.best_trial.number], f)
        with open('cgra_eva_bestHomo.json', 'w') as f:
            json.dump(obj.best_value, f)
