import load
import plot
from typing import Final
import pandas as pd

def main() -> None:
    metricsDf: Final[pd.DataFrame] = load.loadMetrics("outputs/metrics.h5")
    
    plot.saveSingleLinePlot(x=metricsDf["step"].iloc[5000:], y=metricsDf["avgFirmAssets"].iloc[5000:], title="Firm Asset", xlabel="step", ylabel="asset",
                            color="red", filename="outputs/firm.png")
    plot.saveSingleLinePlot(x=metricsDf["step"].iloc[5000:], y=metricsDf["avgHholdAssets"].iloc[5000:], title="Household Asset", xlabel="step", ylabel="asset",
                            color="red", filename="outputs/hhold.png")

if __name__ == "__main__":
    main()