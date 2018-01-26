import matplotlib.pyplot as plt
import csv

x = []
y = []
z = []

with open('data.csv','r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        x.append(int(row[0]))
        y.append(int(row[1]))
        z.append(int(row[2]))

plt.plot(x,y, label='C', color="blue")
plt.plot(x,z, label='R', color="red")
plt.xlabel('x')
plt.ylabel('y')

plt.title('Grafico Compressao/Respiracao')
plt.xlabel('Time')
plt.ylabel('Compressao/Respiracao')
plt.legend()
plt.show()
