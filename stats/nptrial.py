
#!/usr/bin/python
# coding=utf-8
#
import sys
import pandas as pd
import numpy as np
from sklearn.decomposition import PCA
import m3summary

columns = ["useful", "useless",
#           "local", "localhost", "rfc6761",
#           "home", "lan", "internal", "ip", "localdomain" ,"corp", "mail",
           "dga", "others"]

df = pd.read_csv(sys.argv[1])

df['useful'] = 0.0
df['others'] = 0.0

print(df.head())

indices = df.index.tolist()

for i in indices:  
    n = df['queries'][i]
    nx = df['nx_domain'][i]
    df.loc[i, 'useful'] = n - nx - df['useless'][i]
    j = 2
    while j < len(columns) - 1:
        nx -= df.loc[i, columns[j]]
        j += 1
    df.loc[i, 'others'] = nx
#    for x in columns:
#        n -= df[x][i]
#   df.loc[i, 'others'] = n

print(df.head())

fdf = pd.DataFrame()

for column in columns:
    fdf[column] = df[column]

print(fdf.head())

sums = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
fraction = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]

j = 0
t = 0.0
while j < len(columns):
    sums[j] = sum(fdf[columns[j]])
    t += sums[j]
    j += 1
print("Sums")
print(sums)
j = 0
while j < len(columns):
    fraction[j] = sums[j]/t
    j += 1
print("fraction")
print(fraction)

nbcomponents = 8
if (len(columns) < nbcomponents):
    nbcomponents = len(columns)

coefs = np.corrcoef(fdf, rowvar=False)
print("Correlation coefficients")
print(coefs)

print("PCA")
pca=PCA(nbcomponents)
pca.fit(fdf)
print(pca.components_)
print(pca.explained_variance_)
print(pca.explained_variance_ratio_)

x_pca = pca.transform(fdf)
print("original shape:   ", fdf.shape)
print("transformed shape:", x_pca.shape)

print(type(x_pca))


