# Coding-samples
kmeans.c is a k-means code written in C++ that has been parallelized with Pthreads.
When I compiled it, I found it to be very efficient, even more so than the instructor's version.

Project2.py is a project I did for my Data Management and Analytics class (i.e. data science class) 
wherein a large number of songs had their lyrics uploaded and stemmed. These were provided
to us for the project and our goal was to determine which songs had a Jaccard similarity of greater
than 0.95; ideally this would mean determining how many songs were duplicates or near duplicates
based on similarity of lyrics. Because of the large amount of data (>200,000), a direct performance
of the Jaccard similiarty was unfeasible, because it would require far too many comparisons, so
min-hashing was employed to determine candidate pairs. The work itself was done in a Jupyter
notebook, which I have exported here as a .py file.

vasp_molecules is a Python script I wrote to show how I use Python in my day-to-day work.
This script takes an input structure file (i.e. a list of atoms and their coordinates) and
determines all molecules within that file based on the distance of atoms to their nearest neighbor
(some threshold is chosen). At the moment, the code is a little messy, but that is due to the
quick-and-dirty solution for the problem I wanted. If I continue using it regularly, I will likely
clean up the code and perhaps include it as part of a larger code as a module.
