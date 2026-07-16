import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
from scipy.stats import gaussian_kde
from functools import partial
import pandas as pd
from typing import Final

def saveSingleLinePlot(x, y, title: str, xlabel: str, ylabel: str, color: str, filename: str) -> None:
    fig, ax = plt.subplots(figsize=(6,4))
    ax.plot(x, y, color=color, lw=2)
    ax.set_title(title)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.grid(True, linestyle='--', alpha=0.5)

    plt.tight_layout()
    plt.savefig(filename, dpi=150)
    plt.close()
    print(f"グラフを保存しました: {filename}")

def genericKdeFrame(frame, df: pd.DataFrame, columnName: str, ax, line, title: str, titleTemplate, xEval):
    stepData: Final[pd.DataFrame] = df[df["step"] == frame][columnName].values

    if len(stepData) > 1 and np.var(stepData) > 1e-5:
        kde: Final = gaussian_kde(stepData)
        yVals: Final = kde(xEval)
    else:
        yVals = np.zeros_like(xEval)
        if len(stepData) > 0:
            idx: Final = (np.abs(xEval - np.mean(stepData))).argmin()
            yVals[idx] = 1.0

    line.set_ydata(yVals)
    title.set_text(titleTemplate.format(frame=frame))
    return line, title

def generateKdeGif(df: pd.DataFrame, columnName: str, filename: str, titleTemplate: str ="Distribution Dynamics [{frame}]", 
                   xlabel="Value", color="teal", stepInterval=2):
    fig, ax = plt.subplots(figsize=(7, 4))
    
    xMin: Final[float] = df[columnName].min()
    xMax: Final[float] = df[columnName].max()
    xEval: Final = np.linspace(xMin, xMax, 200)
    
    ax.set_xlabel(xlabel)
    ax.set_ylabel("Density")
    ax.grid(True, linestyle='--', alpha=0.5)
    
    line, = ax.plot(xEval, np.zeros_like(xEval), color=color, lw=2)
    title = ax.set_title("")
    
    boundUpdate = partial(
        genericKdeFrame, 
        df=df, 
        columnName=columnName, 
        ax=ax, 
        line=line, 
        title=title, 
        titleTemplate=titleTemplate, 
        xEval=xEval
    )
    
    maxStep: Final[float] = df['step'].max()
    ani: Final = animation.FuncAnimation(
        fig, boundUpdate, frames=range(0, maxStep + 1, stepInterval), blit=True
    )
    
    ani.save(filename, writer='pillow', fps=15)
    plt.close()
    print(f"KDEアニメーションを保存しました: {filename}")