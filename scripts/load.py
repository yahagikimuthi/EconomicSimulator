import h5py
import pandas as pd
import numpy as np
from typing import Final

def loadSimulationResults(filepath: str) -> None:
    allRows: list = []

    with h5py.File(filepath, "r") as f:
        for stepKey in sorted(f.keys()):
            stepNum: Final[int] = int(stepKey.split("_")[1])
            group: Final[h5py.Group] = f[stepKey]

            assets: Final[np.array] = group["firmAssets"][:]
            postedEmployments: Final[np.array] = group["postedEmployments"][:]
            employments: Final[np.array] = group["employments"][:]
            prices: Final[np.array] = group["prices"][:]
            supplies: Final[np.array] = group["supplies"][:]
            markups: Final[np.array] = group["markups"][:]
            inventories: Final[np.array] = group["inventories"]

            for firmId in range(len(prices)):
                allRows.append({
                    "step": stepNum,
                    "firmId": firmId,
                    "assets": assets[firmId],
                    "postedEmployments": postedEmployments[firmId],
                    "employments": employments[firmId],
                    "prices": prices[firmId],
                    "supplies": supplies[firmId],
                    "markups": markups[firmId],
                    "inventories": inventories[firmId]
                })
    
    print("生データを読み込みました")
    return pd.DataFrame(allRows)

def loadMetrics(filepath: str) -> None:
    with h5py.File(filepath, "r") as f:
        data: Final[dict] = {key: f[key][:] for key in f.keys()}
    print("統計量を読み込みました")
    return pd.DataFrame(data)