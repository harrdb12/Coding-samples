import numpy as np
import itertools

mat = np.matrix([[0,1,1,0],[3,0,1,0],[3,2,0,0],[2,0,0,3],[1,1,1,1],[1,1,1,1],[0,3,2,3]])
mat = mat.transpose()

rhs = np.matrix([2,2,2,0])

c = 0
for i in range(mat.shape[1]):
    a = list(set(itertools.permutations((np.concatenate((np.ones(i+1),
                                  np.zeros(mat.shape[1]-i-1))).astype(int)))))
    for j in a:
        b = np.matmul(mat,j)
        if (b >= rhs).sum() == rhs.size:
            c = 1
            print(j,b.sum())
    if c == 1:
        break
