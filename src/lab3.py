import numpy as np
import matplotlib.pyplot as plt

__, ax = plt.subplots()

def D_M(point, mu, Cov):
    """
    Mahalanobis distance
    """
    return np.sqrt( ((point - mu)[:np.newaxis]).T @ np.linalg.inv(Cov) @ (point- mu) )

def UniformGenerator(bords, n):
    points = []
    for bord in bords:
        points.append([np.random.rand(n)*(bord["x"][1] - bord["x"][0]) + bord["x"][0], np.random.rand(n)*(bord["y"][1] - bord["y"][0]) + bord["y"][0]])
    return points

def NormalGenerator(bords, n):
    points = []
    for bord in bords:
        points.append([np.random.normal(loc= bord["x"][0], scale= bord["x"][1], size=n), np.random.normal(loc= bord["y"][0], scale=bord["y"][1], size= n)])
    return points

class Classifier:
    def __init__(self, types):
        self.types = types
    def classify(self, point):
        t = []
        d = []
        for type in self.types:
            for i, mean in enumerate(type.means):
                d.append([D_M(point, mean, type.covs[i]), type.type])
        d.sort(key=lambda x:x[0])
        i = 0
        while d[i][0] == d[0][0] and i < d.__len__():
            t.append(d[i][1])
            i += 1
        return list( dict.fromkeys(t) )

class ClassPoints:
    def __init__(self, points, n, type, color):
        self.points = points
        self.means = []
        self.n = n
        self.type = type
        self.color = color
        self.covs = []
        for point in points:
            self.means.append([np.sum(point[0])/n, np.sum(point[1])/n])
            self.covs.append(np.cov(np.stack((point[0], point[1]))))
    def plot(self):
        for point in self.points:
            plt.scatter(point[0], point[1], color = self.color + '2f')
        for mean in self.means:
            plt.scatter(mean[0], mean[1], color = self.color, marker='x')
        label = plt.scatter([], [], color = self.color + '2f', label = f'Type {self.type}')
        label_mean = plt.scatter([], [], color = self.color, marker='x', label = f'Type {self.type} mean')
        handles, labels = ax.get_legend_handles_labels()
        handles.append(label)
        return plt.legend(handles, labels, bbox_to_anchor=(0., 1.02, 1., .102), loc='lower left', ncol = 3, mode="expand", borderaxespad=0.)
        
        


type1_n = 20
type2_n = 20
type3_n = 20
main_n = 100
colors = ['#ff0000', '#0000ff', '#00ff00']
main_bords = [ {
                    "x": [0, 7],
                    "y": [0, 5]
               }]
type1_bords = [ {
                    "x": [1, 6],
                    "y": [0.5, 1]
                },
                {
                    "x": [1, 6],
                    "y": [3.5, 4]
                },
                {
                    "x": [1, 1.5],
                    "y": [0.5, 4]
                },
                {
                    "x": [5.5, 6],
                    "y": [0.5, 4]
                }]
type2_bords = [ {
                    "x": [2, 0.1],
                    "y": [2.25, 0.3]
                },
                {
                    "x": [3.5, 0.6],
                    "y": [1.5, 0.1]
                },
                {
                    "x": [5, 0.1],
                    "y": [2.25, 0.3]
                },
                {
                    "x": [3.5, 0.6],
                    "y": [3, 0.1]
                }]
type3_bords = [ {
                    "x": [3, 4],
                    "y": [2, 2.5]
                }]
points = UniformGenerator(type1_bords, type1_n)
type1 = ClassPoints(points, type1_n, 1, colors[0])
type1.plot()
points2 = NormalGenerator(type2_bords, type2_n)
type2 = ClassPoints(points2, type2_n, 2, colors[1])
type2.plot()
points3 = UniformGenerator(type3_bords, type3_n)
type3 = ClassPoints(points3, type3_n, 3, colors[2])
plt.gca().add_artist(type3.plot())
main_points = UniformGenerator(main_bords, main_n)
classifier = Classifier([type1, type2, type3])
res = [[], [], []]
for point in main_points:
    for p in np.array(list(zip(point[0],point[1]))):
        t = classifier.classify(p)
        for pt in t:
            print(f'Point ({p[0]}; {p[1]}) has type {pt}')
        res[t[0] - 1].append(p)
for i in range(3):
    res[i] = np.array(res[i])
handles = []
for i, res_point in enumerate(res):
    handles.append(plt.scatter(res_point.T[0], res_point.T[1], color = colors[i], label = f'Test type {i + 1}'))
plt.legend(handles=handles, loc = 'upper right')
plt.show()