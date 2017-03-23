
# coding: utf-8

# In[2]:

import mysql.connector as sql
import pandas as pd
import numpy as np


# In[142]:

# from off campus it MAY/MAY NOT be faster to use a database mirror at 192.168.1.37
#mysqlCnx= sql.connect(host='192.168.1.37', 
mysqlCnx= sql.connect(host='falcon.cs.wfu.edu', 
                port=3306,user='CSstudent', passwd='CSdeacon', 
                db='mxm')


# In[3]:

# for the full data set use 237662
numLyrics = 237662
# for testing only
#numLyrics  = 5
numHashes = 101
#print('loaded tid names from database:', numLyrics)


# In[4]:

# create and initialize Signature matrix for minhashing
M = np.empty([numHashes, numLyrics], dtype=int)
M.fill( 2147483647, )   # fill with "infinity"
M.shape


# In[5]:

def hash( a, b, row ):
    return (a * row + b) % numLyrics

# generate coefficients for numHashes hash functions
hashCoefficients = np.random.randint(1001, high=2147483647, size=(numHashes,2))


# In[6]:

def updateSigMatrix( wordID, colNum):
    global M
    for hashKey in range(numHashes):
        hashval = hash( hashCoefficients[hashKey][0], hashCoefficients[hashKey][1], wordID )
        if hashval < M[hashKey][colNum]:
            M[hashKey][colNum] = hashval
    return      


# In[9]:

# This cell contains the final step for filling in the signature matrix
# for all the lyrics -- 
# For testing - skip this cell and run the next cell
for lyricID in range(numLyrics):

    if lyricID % 1000 == 0:
        print( "At lyricID: ", lyricID )
        
    query = "select wordID from lyricBags where lyricID = '%s'"%(lyricID,)
    wordInfoDF = pd.read_sql(query, con=mysqlCnx)
    
    for (_,wordID) in wordInfoDF.itertuples():
        updateSigMatrix( wordID, lyricID )


# In[7]:

# NOTE: Only execute this cell for testing purposes.
#       This only fills in the signature matrix for 7 lyrics.

# This is a test of the process on these 7 selected lyrics by lyricID
testLyrics = [ 58584, 86470, 112317, 146341, 166049, 199097, 233196 ]

# fill in the signature matrix for these selected lyricIDs
# This will result in just a few columns of the minhash matrix being computed
for lyricID in testLyrics:
    query = "select wordID from lyricBags where lyricID = '%s'"%(lyricID,)
    wordInfoDF = pd.read_sql(query, con=mysqlCnx)
    
    for (_,wordID) in wordInfoDF.itertuples():
        updateSigMatrix( wordID, lyricID )

# show the titles of the specific lyrics selected
query = "select artist, title from lyrics natural join details" +          " where lyricID in ( '%s', '%s', '%s', '%s', '%s', '%s', '%s' )" % tuple(testLyrics)
results = pd.read_sql(query, con=mysqlCnx)

for row in results.itertuples():
    print( row[0], row[1].decode('utf-8'), "\t", row[2].decode('utf-8') )
print()

# print first 20 rows of Signature matrix for these 7 test lyrics
for i in range(20):
    for lyricNum in range(len(testLyrics)):
       print( M[i,testLyrics[lyricNum]], end="\t" )
    print()


# In[13]:

# can run this for testing whether the entire table is filled or not
testLyrics = [ 58584, 86470, 112317, 146341, 166049, 199097, 233196 ]

# show the titles of the specific lyrics selected
query = "select artist, title from lyrics natural join details" +          " where lyricID in ( '%s', '%s', '%s', '%s', '%s', '%s', '%s' )" % tuple(testLyrics)
results = pd.read_sql(query, con=mysqlCnx)

for row in results.itertuples():
    print( row[0], row[1].decode('utf-8'), "\t", row[2].decode('utf-8') )
print()

# print first 20 rows of Signature matrix for these 7 test lyrics
for i in range(20):
    for lyricNum in range(len(testLyrics)):
       print( M[i,testLyrics[lyricNum]], end="\t" )
    print()


# In[121]:

# save M to a file
#np.save( "M.npy", M, allow_pickle=True )

M = np.load( "M.npy" )
matrix1 = M[:25]
matrix2 = M[25:50]
matrix3 = M[50:75]
matrix4 = M[75:]
pair_matrix = []

def create_pairs(input_matrix):
    newhashCoefficients = np.random.randint(1001, high=2147483647, size=(input_matrix.shape[0]))
    random_constant = np.random.randint(1001, high=2147483647)
    dicthash = {}
    for i in range(input_matrix.shape[1]):
        w = (np.dot(newhashCoefficients,input_matrix[:,i])+random_constant)%1000000000
        if w in dicthash:
            dicthash[w].append(i)
        else:
            dicthash[w]=[i]

    hash_matches = [v for (k,v) in dicthash.items() if len(v) > 1]
    for i in hash_matches:
        for j in range(len(i)-1):
            for k in range(j+1,len(i)):
                if [i[j],i[k]] not in pair_matrix:
                    pair_matrix.append([i[j],i[k]]) 



# In[122]:

create_pairs(matrix1)
create_pairs(matrix2)


# In[123]:

create_pairs(matrix3)
create_pairs(matrix4)
pair_matrix = sorted(pair_matrix)


# In[143]:

outfile = open("mydata.out", "w")
for lyricID in pair_matrix:
        
    query1 = "select count(*) from lyricBags as S join lyricBags as T using (wordID) where S.lyricID = '%s' and T.lyricID = '%s'"%(lyricID[0],lyricID[1])
    query2 = "select count(*) from lyricBags where lyricID = '%s'"%(lyricID[0])
    query3 = "select count(*) from lyricBags where lyricID = '%s'"%(lyricID[1])
    query4 = "select distinct details.title from lyricBags natural join lyrics natural join details where lyricID = '%s'"%(lyricID[0])
    query5 = "select distinct details.title from lyricBags natural join lyrics natural join details where lyricID = '%s'"%(lyricID[1])
    intersection = pd.read_sql(query1, con=mysqlCnx)
    total1 = pd.read_sql(query2, con=mysqlCnx)
    total2 = pd.read_sql(query3, con=mysqlCnx)
    title1 = pd.read_sql(query4, con=mysqlCnx)
    title2 = pd.read_sql(query5, con=mysqlCnx)

    similarity = intersection/(total1+total2-intersection)
    similarity = float(similarity.values)
    if similarity >= 0.95:
        print(title1['title'][0].decode('utf-8'),'\t',title2['title'][0].decode('utf-8'),'\t',similarity, file=outfile)
outfile.close()

