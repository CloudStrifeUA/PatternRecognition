from matplotlib import scale
import numpy as np
import matplotlib.pyplot as plt
from numpy.core.fromnumeric import mean
from scipy.spatial import Voronoi, voronoi_plot_2d

max_iters = 5
eps = 1e-2
cluster_colors = ['#ff0000', '#00ff00', '#0000ff', '#ff00ff', '#ffff00']
init_centers = [[2, 2], [4, 1], [4 ,4], [1, 1], [2.5, 2.5]]

def euclid_distance(x,y):
    return np.sqrt(((x-y)**2).sum(axis = -1))    

def plot_cluster_points(points, cluster_center, cluster_colors):
    v = Voronoi(cluster_center)
    fig = voronoi_plot_2d(v, show_points=False)
    fig.set_size_inches(10,10)
    plt.figure(fig.number)
    plt.axis([0,5, 0, 5])
    for point in points:
        cluster = np.argmin(euclid_distance(cluster_center, point))
        plt.scatter(point[0], point[1], color=cluster_colors[cluster])
    for cluster, center in enumerate(cluster_center):
        plt.scatter(center[0], center[1], color = cluster_colors[cluster], marker='x', label = f'Cluster {cluster + 1} center')
        plt.scatter([], [], color=cluster_colors[cluster], label = f'Cluster {cluster + 1} point')
    plt.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc='lower left', ncol = 5, mode="expand", borderaxespad=0.)

np.random.seed(0)
points = np.random.uniform(0, 5, (100, 2))
plot_cluster_points(points, init_centers, cluster_colors)
plt.title('Initial clusterization', y = 1.1)
plt.show()
old_centers = None
it = 0
while it < max_iters and (old_centers is None or  np.max(euclid_distance(old_centers, init_centers)) > eps):
    labels = []
    for dot in points:
        labels.append(np.argmin(euclid_distance(init_centers, dot)))
    labels = np.array(labels)
    means = []
    for i in range(len(init_centers)):
        means.append(points[np.where(labels == i)].sum(axis = 0)/ (labels == i).sum())
    old_centers = init_centers
    init_centers = np.array(means)
    plot_cluster_points(points, init_centers, cluster_colors)
    plt.title(f'Iteration: {it + 1}', y = 1.1)
    plt.show()
    it+=1