# Coding-samples
kmeans.c is a k-means code written in C++ that has been parallelized with Pthreads.
When I compiled it, I found it to be very efficient, even more so than the instructor's compiled code.


march_madness.ipynb and march_madness2.ipynb are both part of an ongoing group project where we are trying to build a model to predict outcomes of basketball games. The data was provided by Kaggle as part of the recent March Madness 2017 competition, although detailed data exists on the past 15 years of basketball. 
Currently both notebooks were coded completely by me, although as more work is done on the project I expect it will contain contributions from my other group members.

The first notebook, march_madness.ipynb, is rather simple; for the entire regular season, it looks at all games played between teams which have a seed in the March tournament, and builds a predictive model based the difference in seeds between two teams and the game result. 
It then uses this model to predict results for the March tournament of that year. So the season serves as the training data and the tournament serves as the test data. It's rather simplistic but gives an idea of what I would ultimately want to do.

The second notebook tries to incorporate more variables, and creates a dataframe called 'averages' which contains in the rows a list of all the teams (with the index of the rows being the team label) along with all of the mean for all of that teams statistics
(field goals made, field goals attempted, etc.). Then, for each game in the regular season, it creates a list of statistics for each team and the result of the game (in the dataframe called 'compiled'). A model is then trained on the first 3000 rows and tested on the rest. At the end of the notebook, I've also created a dataframe called 'differences', which instead of having the statistics for each individual team looks at the difference between the average statistics of both teams.

I've noticed that the models tend to overfit, getting around 80% accuracy on the training data but only 65% accuracy on the testing data. I've also noticed that random forests and SVM perform similarly. Finally, the current descriptors I'm using are only marginally better than simply looking at the seed difference between teams. Current goals are to determine a better set of descriptors and determine
the best modeling method.


Project2.ipynb is a project I did for my Data Management and Analytics class (i.e. data science class) 
wherein a large number of songs had their lyrics uploaded and stemmed. These were provided
to us for the project and our goal was to determine which songs had a Jaccard similarity of greater
than 0.95; ideally this would mean determining how many songs were duplicates or near duplicates
based on similarity of lyrics. Because of the large amount of data (>200,000), a direct performance
of the Jaccard similiarty was unfeasible, because it would require far too many comparisons, so
min-hashing was employed to determine candidate pairs. 


vasp_molecules is a Python script I wrote to show how I use Python in my day-to-day work.
This script takes an input structure file (i.e. a list of atoms and their coordinates) and
determines all molecules within that file based on the distance of atoms to their nearest neighbor
(some threshold is chosen). At the moment, the code is a little messy, but that is due to the
quick-and-dirty solution for the problem I wanted. If I continue using it regularly, I will likely
clean up the code and perhaps include it as part of a larger code as a module.
