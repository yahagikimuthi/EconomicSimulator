import load
import plot
from typing import Final

def main() -> None:
    rawDf: Final = load.loadSimulationResults("outputs/result.h5")
    metricsDf: Final = load.loadMetrics("outputs/metrics.h5")

    plot.saveSingleLinePlot(x=metricsDf["step"], y=metricsDf["cpi"], title="CPI", xlabel="step", ylabel="cpi",
                            color="red", filename="cpi.png")
    plot.generateKdeGif(df=rawDf, columnName="prices", filename="priceGif.gif", titleTemplate="Price Distribution[{frame}]", xlabel="step", color="red")

if __name__ == "__main__":
    main()