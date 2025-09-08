import pandas as pd

def load_results(path: str):
    df = pd.read_csv(path)

    df["RX_TIME"] = df["RX_End"] - df["RX_Start"]
    df["TX_TIME"] = df["TX_End"] - df["TX_Start"]
    df["LATENCY"] = df["RX_End"] - df["TX_End"]

    return df