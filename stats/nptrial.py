
#!/usr/bin/python
# coding=utf-8
#
import sys
import pandas as pd
import numpy as np
from sklearn.decomposition import PCA
import m3summary
import matplotlib.pyplot as plt

df = pd.read_csv(sys.argv[1])

df['qps'] = 0.0
df['nxd'] = 0.0
df['hom'] = 0.0
df['cor'] = 0.0
df['mai'] = 0.0
df['cc3'] = "???"

countries = df.index.tolist()

for i in countries:   
    df.loc[i, 'cc3'] = m3summary.cc_to_iso3(df['cc'][i])
    if df['duration'][i] > 0:
        df.loc[i,'qps'] = float(df['queries'][i]) / float(df['duration'][i])
    if df['queries'][i] > 0:
        hom_i = float(df['home'][i])/float(df['queries'][i])
        cor_i = float(df['corp'][i])/float(df['queries'][i])
        mai_i = float(df['mail'][i])/float(df['queries'][i])
        df.loc[i,'nxd'] = (float(df['nx_domain'][i])/float(df['queries'][i])) - hom_i - cor_i - mai_i
        df.loc[i,'hom'] = hom_i
        df.loc[i,'cor'] = cor_i
        df.loc[i,'mai'] = mai_i

fdf = pd.DataFrame()
fdf['nxd'] = df['nxd']
fdf['hom'] = df['hom']
fdf['cor'] = df['cor']
fdf['mai'] = df['mai']


print(fdf.head())

#nxd = pd.Series(df['nxd'].head())
#print(type(nxd))

#fdfnp = fdf.to_numpy(dtype ='float32')

#corr = np.corrcoef(fdfnp, rowvar=False)
#print("Corr:")
#print(corr)
#
#w, v = np.linalg.eigh(corr)
#print("Eigh")
#print(w)
#print(v)

print("PCA")
pca=PCA(n_components=4)
pca.fit(fdf)
print(pca.components_)
print(pca.explained_variance_)
print(pca.explained_variance_ratio_)

x_pca = pca.transform(fdf)
print("original shape:   ", fdf.shape)
print("transformed shape:", x_pca.shape)

print(type(x_pca))
