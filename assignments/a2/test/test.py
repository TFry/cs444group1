import matplotlib.pyplot as graph

sectors = []

with open("io.txt") as io:
   lines = io.read().split('\n')

   for line in lines:
   	if not line or line[0] == 'A':
		continue
 	
	cur = int(line.split()[-1][:-1])
	sectors.append(cur)

graph.xlabel('request num')
graph.ylabel('sector')

graph.plot(sectors)
graph.show()
