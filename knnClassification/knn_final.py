# Sk-learn knn-classifiation.
# All the magic is done is the preprocessing stage
#
# Original data not provided here
# 
# Erik Bertse


import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# For preprocessing and kNN
import sklearn.preprocessing as skl_pre
import sklearn.neighbors as skl_nb

# For cross validation
from sklearn.model_selection import cross_val_score

# Selector and scoring functions
from sklearn.feature_selection import SelectKBest
from sklearn.feature_selection import chi2
from sklearn.feature_selection import f_classif

# PCA
from sklearn.decomposition import PCA

# Metric Learn
import metric_learn as ml

# If True, shows visualizations.
# Also makes NCA verbose.
SHOWVIS = True

np.random.seed(0)
pca = PCA(n_components = 2)

data = pd.read_csv('data/training_data.csv')
realtestdata = pd.read_csv('data/songs_to_classify.csv')

X = data.drop(columns = 'label')
y = data['label']

# Colors for visualizations
colors = []
for i in y:
  if i==1:
    colors.append('green')
  else:
    colors.append('black')

if (SHOWVIS):
  XVis = pd.DataFrame(pca.fit_transform(X))
  XVis.plot(x=0, y=1, kind='scatter', c=colors)
  plt.suptitle('Raw data')
  plt.show(block = True)

# --------------------------------------
# XX ------- One Hot Encoding ------- XX
# --------------------------------------
X = pd.get_dummies(X, columns=['key','mode','time_signature'])
realtestdata = pd.get_dummies(realtestdata, columns=['key','mode', 'time_signature'])

if (SHOWVIS):
  XVis = pd.DataFrame(pca.fit_transform(X))
  XVis.plot(x=0, y=1, kind='scatter', c=colors)
  plt.suptitle('Data after One Hot encoding')
  plt.show(block = True)


# --------------------------------------
# XX ---- Feature Normalization ----- XX
# --------------------------------------

# min_max_scaler = skl_pre.MinMaxScaler()
normal_scaler = skl_pre.StandardScaler() #tiny bit better

X = pd.DataFrame(normal_scaler.fit_transform(X))
realtestdata = pd.DataFrame(normal_scaler.fit_transform(realtestdata))

if (SHOWVIS):
  XVis = pd.DataFrame(pca.fit_transform(X))
  XVis.plot(x=0, y=1, kind='scatter', c=colors)
  plt.suptitle('Data after normalization')
  plt.show(block = True)


# --------------------------------------
# XX ------- Feature Selection ------ XX
# --------------------------------------

selectedlabels = X.columns

selector = SelectKBest(score_func=f_classif, k=5) #k=5 seems optimal
fit = selector.fit(X, y)

cols = selector.get_support(indices=True)

X = X.iloc[:,cols]
realtestdata = realtestdata.iloc[:,cols]
# print('Chosen columns: ' + str(splitlabels[cols]))

if (SHOWVIS):
  XVis = pd.DataFrame(pca.fit_transform(X))
  XVis.plot(x=0, y=1, kind='scatter', c=colors)
  plt.suptitle('Data after feature selection')
  plt.show(block = True)


# --------------------------------------
# XX -- Mahalanobis Transformation -- XX
# --------------------------------------
metric_learner = ml.nca.NCA(max_iter=1000, verbose=SHOWVIS)
X = pd.DataFrame(metric_learner.fit_transform(X, y))
realtestdata = pd.DataFrame(metric_learner.transform(realtestdata))

if (SHOWVIS):
  XVis = pd.DataFrame(pca.fit_transform(X))
  XVis.plot(x=0, y=1, kind='scatter', c=colors)
  plt.suptitle('Data after Mahalanobis transformation')
  plt.show(block = True)


# --------------------------------------
# XX - kNN Training and validation -  XX
# --------------------------------------
score_accuracy = []
testto = 30

for k in range(testto):
  # Classifier
  classifier = skl_nb.KNeighborsClassifier(n_neighbors=k+1, weights='distance')

  # 10Fold Cross validation
  scores = cross_val_score(classifier, X, y, cv=10)
  
  print("For k = %d Accuracy: %0.5f (+/- %0.2f)" % (k+1, scores.mean(), scores.std() * 2))
  
  if (SHOWVIS):
    print("scores at " + str(k+1) + ": " + str(scores))
    score_accuracy.append(scores.mean())


# print("Accuracy: " + str(score_accuracy))

if (SHOWVIS):
  K = np.linspace(1, testto, testto)
  plt.xlabel('k')
  plt.ylabel('Accuracy')
  plt.grid(True)
  l1, = plt.plot(K, score_accuracy)
  plt.show(block=True)

classifier_real = skl_nb.KNeighborsClassifier(n_neighbors=8, weights='distance')
classifier_real.fit(X, y)
realprediction = classifier_real.predict(realtestdata)

if (SHOWVIS):
  print(realprediction)
  print('Prediction as string: ')
  for i in realprediction:
    print(str(i), end="")
  print("")