import numpy as np
import matplotlib.pyplot as plt

__, ax = plt.subplots()

def UniformGenerator(bords, n):
    _bords = np.array(bords)
    return np.random.random((n, len(_bords))) * (_bords[:,1] - _bords[:,0]) + _bords[:,0]

def NormalGenerator(params, n):
    return np.hstack([np.random.normal(loc= param[0], scale= param[1], size=(n, 1)) for param in params])

def CalcMean(types, point, skip):
    res = 0
    n = 0
    for type in types:
        if type.type == skip:
            continue
        res += np.abs(type.mean - point)
        n += 1
    return res/n

class Classifier:
    def __init__(self, types):
        self.types = types
    def classifyM1(self, point):
        c = []
        res = []
        for type in self.types:
            c.append(np.abs(type.mean - point)/CalcMean(self.types, point, type.type))
        for ax in np.swapaxes(np.array(c), 0, 1):
            res.append(self.types[ax.argmin(axis=-1)].type)
        counts = np.bincount(np.array(res))
        max = counts.max()
        res = []
        for type, count in enumerate(counts):
            if count == max:
                res.append(type)
        return res
    def classifyM2(self, point):
        res = []
        for type in self.types:
            res.append(np.sum(np.abs(type.mean - point)))
        res = np.array(res)
        min = res.min()
        _res = []
        for i, r in enumerate(res):
            if r == min:
                _res.append(self.types[i].type)
        return _res

class ClassPoints:
    def __init__(self, points, type, color):
        self.points = points
        self.n = len(points)
        self.dim = 0 if self.n == 0 else len(points[0])
        self.mean = [np.sum(points[:,i])/self.n for i in range(self.dim)]
        self.type = type
        self.color = color
    def plot(self):
        plt.scatter(self.points.T[0], self.points.T[1], color = self.color + '2f')
        plt.scatter(self.mean[0], self.mean[1], color = self.color, marker='x')
        label = plt.scatter([], [], color = self.color + '2f', label = f'Type {self.type}')
        label_mean = plt.scatter([], [], color = self.color, marker='x', label = f'Type {self.type} mean')
        handles, labels = ax.get_legend_handles_labels()
        handles.append(label)
        return plt.legend(handles, labels, bbox_to_anchor=(0., 1.02, 1., .102), loc='lower left', ncol = 3, mode="expand", borderaxespad=0.)


type1_n = 20
type2_n = 20
type3_n = 20
main_n = 100
dim = 2
colors = ['#ff0000', '#0000ff', '#00ff00']
type1 = ClassPoints(UniformGenerator([[3,4], [4,5]], type1_n), 1, colors[0])
type2 = ClassPoints(NormalGenerator([[1,0.1], [1,0.7]], type2_n), 2, colors[1])
type3 = ClassPoints(UniformGenerator([[5,6], [0,1]], type3_n), 3, colors[2])
main_points = UniformGenerator([[0, 7], [0, 5]], main_n)
classifier = Classifier([type1, type2, type3])
res = [[], [], []]
for point in main_points:
    types = classifier.classifyM2(point)
    for t in types:
        print(f'Point {point} has type {t}')
    res[types[0] - 1].append(point)
if dim == 2:
    type1.plot()
    type2.plot()
    plt.gca().add_artist(type3.plot())
    for i in range(3):
        res[i] = np.array(res[i])
    handles = []
    for i, res_point in enumerate(res):
        handles.append(plt.scatter(res_point.T[0], res_point.T[1], color = colors[i], label = f'Test type {i + 1}'))
    plt.legend(handles=handles, loc = 'upper right')
    plt.show()