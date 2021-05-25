import numpy as np
import matplotlib.pyplot as plt

fig, ax = plt.subplots()
fig.set_size_inches(10,10)
plt.axis([0,6, 0, 6])

def UniformGenerator(bords, n):
    _bords = np.array(bords)
    return np.random.random((n, len(_bords))) * (_bords[:,1] - _bords[:,0]) + _bords[:,0]

def euclid_distance(x,y):
    return np.sqrt(((x-y)**2).sum(axis = -1)) 

def Norm(v):
    return np.sqrt(np.dot(v, v))

def ProjectToLine(points, line):
    fwd = np.array(line[1]) - np.array(line[0]) 
    fwd /= Norm(fwd)
    proj = []
    for point in points:
        p = line[0] + fwd*np.dot(point - line[0], fwd)
        proj.append(p)

    return np.array(proj)

class Classifier:
    def __init__(self, types):
        self.types = types
        self.projs = []
        self.lines = []
        for i in range(len(self.types)):
            for j in range(i + 1, len(self.types)):
                self.lines.append([self.types[i].mean, self.types[j].mean])
        for type in self.types:
            proj = []
            for line in self.lines:
                    proj.append(ProjectToLine(type.points, line))
            self.projs.append(proj)
        self.projs = np.array(self.projs)
        res = []
        for i in range(len(self.lines)):
            proj = self.projs[:, i]
            means = []
            for j, p in enumerate(proj):
                mean_proj = ProjectToLine([np.array(self.types[j].mean)], self.lines[i])
                means.append(np.sum(abs(p - mean_proj))/len(p))
            res.append(np.sum(np.array(means))/len(means))
        self.line = np.array(res).argmax()
    def plot(self):
        for line in self.lines:
                plt.axline(line[0], line[1], color = 'yellow')
        handles = []
        for i, proj in enumerate(self.projs):
            for p in proj:
                plt.scatter(p[:,0], p[:,1], marker='.', color = self.types[i].color)
            label = plt.scatter([], [], color = self.types[i].color, marker='.', label = f'Type {self.types[i].type} projection')
            handles.append(label)
        plt.gca().add_artist(plt.legend(handles=handles, loc='upper left'))
    def classify(self, point):
        p_prj = ProjectToLine([np.array(point)], self.lines[self.line])
        proj = self.projs[:, self.line]
        res = []
        for i, _ in enumerate(proj):
            mean_proj = ProjectToLine([np.array(self.types[i].mean)], self.lines[self.line])
            res.append(euclid_distance(p_prj, mean_proj))
        res = np.array(res)
        min = res.min()
        t = []
        for i, r in enumerate(res):
            if r == min:
                t.append(i)
        if len(t) == 1:
            t[0] = self.types[t[0]].type
            return t
        p_count = []
        for type in t:
            dist = res[type]
            count = 0
            for p in self.projs[type][self.line]:
                if  euclid_distance(p, p_prj) < dist:
                    count += 1
            p_count.append(count)
        p_count = np.array(p_count)
        max = p_count.max()
        types = []
        for i, c in enumerate(p_count):
            if c == max:
                types.append(self.types[i].type)
        return types


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
        plt.scatter(self.mean[0], self.mean[1], color = self.color, marker='x', s = 200)
        label = plt.scatter([], [], color = self.color + '2f', label = f'Type {self.type}')
        plt.scatter([], [], color = self.color, marker='x', label = f'Type {self.type} mean')
        handles, labels = ax.get_legend_handles_labels()
        handles.append(label)
        return plt.legend(handles, labels, bbox_to_anchor=(0., 1.02, 1., .102), loc='lower left', ncol = 3, mode="expand", borderaxespad=0.)

type1_n = 20
type2_n = 20
type3_n = 20
main_n = 100
dim = 2
np.random.seed(69)
colors = ['#ff0000', '#0000ff', '#00ff00']
main_points = UniformGenerator([[0, 6], [0, 6]], main_n)
type1 = ClassPoints(UniformGenerator([[3, 4], [3, 6]], type1_n), 1, colors[0])
type2 = ClassPoints(UniformGenerator([[0, 2], [1, 3]], type2_n), 2, colors[1])
type3 = ClassPoints(UniformGenerator([[5,6], [0,1]], type3_n), 3, colors[2])
classifier = Classifier([type1, type2, type3])
res = [[], [], []]
for point in main_points:
    types = classifier.classify(point)
    for t in types:
        print(f'Point {point} has type {t}')
    res[types[0] - 1].append(point)
if dim == 2:
    type1.plot()
    type2.plot()
    plt.gca().add_artist(type3.plot())
    classifier.plot()
    for i in range(3):
        res[i] = np.array(res[i])
    handles = []
    for i, res_point in enumerate(res):
        handles.append(plt.scatter(res_point.T[0], res_point.T[1], color = colors[i], label = f'Test type {i + 1}'))
    plt.legend(handles=handles, loc = 'upper right')
    plt.show()