import h5py
import pandas as pd
import numpy as np
from typing import Final

def loadSimulationResults(filepath: str) -> None:
    allRows = []

    with h5py.File(filepath, "r") as f:
        for stepKey in sorted(f.keys()):
            stepNum: Final[int] = int(stepKey.split("_")[1])
            group: Final = f[stepKey]

            prices: Final[np.array] = group["prices"][:]

            for firmId in range(len(prices)):
                allRows.append({
                    "step": stepNum,
                    "firmId": firmId,
                    "prices": prices[firmId]
                })
    
    print("生データを読み込みました")
    return pd.DataFrame(allRows)

def loadMetrics(filepath: str) -> None:
    with h5py.File(filepath, "r") as f:
        data = {key: f[key][:] for key in f.keys()}
    print("統計量を読み込みました")
    return pd.DataFrame(data)