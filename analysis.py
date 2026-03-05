import sys
import symnmf
import numpy as np
from sklearn.metrics import silhouette_score

#things for kmeans algorithem ----------------------------------------------------------

def calculate_distance(p,q): #p and q are arrays representing points
    sum_of_deltas = 0
    for i in range(len(p)):
        sum_of_deltas += (p[i] - q[i]) ** 2
    return sum_of_deltas**0.5

def update_centroid(cluster):
    dim = len(cluster[0])
    num_of_points = len(cluster)
    updated_centroid = [0 for i in range(dim)]

    for cord in range(dim):
        sum_of_cord = 0
        for point in range(num_of_points):
            sum_of_cord += cluster[point][cord]
        updated_centroid[cord] = sum_of_cord / num_of_points

    return updated_centroid

def find_closest_cluster(centroids, point): #recieves centroids and a point and returns the index of the closest centroid
    close_cluster_index=0
    distance=calculate_distance(centroids[0],point)
    for i in range(1,len(centroids)):
        temp_dis=calculate_distance(centroids[i],point)
        if temp_dis<distance:
            distance=temp_dis
            close_cluster_index=i
    return close_cluster_index

def cluster_handle(k , iter , points_arr):

    centroids = [0 for i in range(k)]
    clusters = [[] for i in range(k)]
    for i in range(k):
        centroids[i] = points_arr[i]
    for iteration in range(iter):

        temp_clusters = [[] for i in range(k)]
        for point in points_arr:
            index = find_closest_cluster(centroids,point)
            temp_clusters[index].append(point)

        clusters = temp_clusters
        convergence = True

        for i in range(k):
            updated_centroid = update_centroid(clusters[i])

            for cord in range(len(updated_centroid)):
                if abs(centroids[i][cord]-updated_centroid[cord]) >= 0.001:
                    convergence = False

            centroids[i] = updated_centroid

        if convergence:
            break
    return clusters

def get_cluster_list_kmeans(clusters, points_list):
    clusters_list = [0 for i in range(len(points_list))]
    for i in range(len(points_list)):
        for j in range(len(clusters)):
            if points_list[i] in clusters[j]:
                clusters_list[i] = j
                break
    return clusters_list

#things for the symnmf alogrithem---------------------------------------

def calculate_matrix_avg(M):
    sum = 0
    for i in range(len(M)):
        for j in range(len(M[0])):
            sum += M[i][j]
    return sum / (len(M)**2)
    
def init_H(W, k):
    n = len(W)
    m = calculate_matrix_avg(W)
    interval = 2 * ((m / k) ** 0.5)
    H = [[np.random.uniform(0.0, interval) for col in range(k)] for row in range(n)]
    return H

def get_cluster_list_symnmf(H, points_list):
    clusters_list = [0 for i in range(len(points_list))]
    for i in range (len(H)):
        max_score = 0
        max_idx = 0
        for j in range(len(H[i])):
            if (H[i][j] > max_score):
                max_score = H[i][j]
                max_idx = j
        clusters_list[i] = max_idx
    return clusters_list

# general code

def parse_points(file_name):
    try:
        points_arr = []
        # Using 'with handles closing the file, even if an error occurs
        with open(file_name, 'r') as file:
            for line in file:
                line = line.strip()
                
                # If the line is empty, skip to the next iteration
                if not line:
                    continue
                
                # Split the line and convert directly to floats using a list comprehension
                temp = line.split(",")
                points_arr.append([float(val) for val in temp])
                
        return points_arr

    # Catching an Exception if one were to occur
    except Exception:
        print("An Error Has Occurred")
        sys.exit(1)


def main():
    np.random.seed(1234)
    if len(sys.argv) != 3:
        print("An Error Has Occurred")
        sys.exit(1)
    
    k = int(sys.argv[1])
    file_name = sys.argv[2]

    points_list = parse_points(file_name)
    n = len(points_list)
    d = len(points_list[0])
    max_iter = 300 #not sure if needed
    epsilon = 1e-4 #not sure if needed

    kmeans_clusters = cluster_handle(k, max_iter, points_list)

    W = symnmf.norm(points_list, n ,d)
    H = init_H(W, k)
    H = symnmf.symnmf(H, W, n, k, epsilon, max_iter)

    symnmf_clusters = get_cluster_list_symnmf(H, points_list)
    kmeans_clusters = get_cluster_list_kmeans(kmeans_clusters, points_list)

    symnmf_score = silhouette_score(points_list, symnmf_clusters)
    kmeans_score = silhouette_score(points_list, kmeans_clusters)

    s = f"nmf: {symnmf_score:.4f}\nkmeans: {kmeans_score:.4f}"
    print(s)
    exit(0)



if __name__ == '__main__':
    main()