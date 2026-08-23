import load
import plot
from typing import Final
import pandas as pd

def readParquet(filename: str) -> pd.DataFrame:
    return pd.read_parquet(filename)

def main() -> None:
    metricsDf: Final[pd.DataFrame] = load.loadMetrics("outputs/metrics.h5")

    plot.saveSingleLinePlot(x=metricsDf["step"].iloc[100:1000], y=metricsDf["avgFirmAssets"].iloc[100:1000], title="Firm Asset", xlabel="step", ylabel="asset", color="red", filename="outputs/firm.png")
    plot.saveSingleLinePlot(x=metricsDf["step"].iloc[100:1000], y=metricsDf["avgHholdAssets"].iloc[100:1000], title="Household Asset", xlabel="step", ylabel="asset", color="red", filename="outputs/hhold.png")
    
    plot.saveSingleLinePlot(x=metricsDf["step"].iloc[100:1000], y=metricsDf["avgPrices"].iloc[100:1000],
    title="cpi", xlabel="step", ylabel="price", color="red", filename="outputs/price.png")

if __name__ == "__main__":
    main()