import load
import plot
from typing import Final

def main() -> None:
    metricsDf: Final = load.loadMetrics("outputs/metrics.h5")

    plot.saveSingleLinePlot(x=metricsDf["step"], y=metricsDf["avgFirmAssets"], title="Firm Asset", xlabel="step", ylabel="asset",
                            color="red", filename="outputs/firm.png")
    plot.saveSingleLinePlot(x=metricsDf["step"], y=metricsDf["avgHholdAssets"], title="Household Asset", xlabel="step", ylabel="asset",
                            color="red", filename="outputs/hhold.png")

if __name__ == "__main__":
    main()