import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
from scipy.stats import gaussian_kde
from functools import partial

def saveSingleLinePlot(x, y, title, xlabel, ylabel, color, filename):
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

def genericKdeFrame(frame, df, columnName, ax, line, title, titleTemplate, xEval):
    stepData = df[df["step"] == frame][columnName].values

    if len(stepData) > 1 and np.var(stepData) > 1e-5:
        kde = gaussian_kde(stepData)
        yVals = kde(xEval)
    else:
        yVals = np.zeros_like(xEval)
        if len(stepData) > 0:
            idx = (np.abs(xEval - np.mean(stepData))).argmin()
            yVals[idx] = 1.0

    line.set_ydata(yVals)
    title.set_text(titleTemplate.format(frame=frame))
    return line, title

def generateKdeGif(df, columnName, fileName, titleTemplate="Distribution Dynamics [{frame}]", 
                   xlabel="Value", color="teal", xmax=1000.0, ymax=5.0, stepInterval=2):
    fig, ax = plt.subplots(figsize=(7, 4))
    
    xMin = df[columnName].min()
    xMax = df[columnName].max()
    xEval = np.linspace(xMin, xMax, 200)
    
    ax.set_xlim(xMin, xmax)
    ax.set_ylim(0, ymax)
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
    
    maxStep = df['step'].max()
    ani = animation.FuncAnimation(
        fig, boundUpdate, frames=range(0, maxStep + 1, stepInterval), blit=True
    )
    
    ani.save(fileName, writer='pillow', fps=15)
    plt.close()
    print("KDEアニメーションを保存しました: {filename}")