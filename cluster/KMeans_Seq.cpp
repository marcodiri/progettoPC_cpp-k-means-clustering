#include "KMeans_Seq.h"

KMeans_Seq::KMeans_Seq(int nClusters, int maxIter, uint seed) : KMeans(nClusters, maxIter,seed) {}

const KMeans *KMeans_Seq::fit(const matrix &points) {
    const auto &points_n = points[0].size();
    const auto &dimension_n = points.size();
    const auto &cluster_n = getNClusters();

    // 1. select random observation as starting centroids
    matrix centroids{dimension_n};

    std::mt19937 mt(getSeed());
    std::uniform_real_distribution<double> dist(0, points_n);

    for (int i=0; i<cluster_n; ++i) {
        auto rnd_i = static_cast<uint>(dist(mt));
        for (int ii=0; ii<dimension_n; ++ii) {
            centroids[ii].push_back(points[ii][rnd_i]);
        }
    }

    decltype(labels) labels(points_n);
    int nIter = 0;
    bool centroidsChanged = true;
    while (centroidsChanged && nIter < getMaxIter()) {
        ++nIter;
        centroidsChanged = false;
        std::vector<unsigned int> pointsPerCluster(cluster_n);
        for (auto &p : pointsPerCluster) p = 0;
        matrix centroidsNew{dimension_n};
        for (auto &el : centroidsNew)
            el.resize(cluster_n, 0); // init new centroids to 0 vectors
        // 2. compute the distance between centroids and observation
        for (size_t i=0; i<points_n; i++) {
            double min = -1; // minimum distance between p and a centroid
            unsigned int centroid_i = 0; // index of the centroid closer to p
            for (size_t j=0; j < cluster_n; j++) {
                // euclidean distance (norm2) between centroid and point
                // sqrt is unnecessary since it's a monotone function
                el_type d = 0;
                for (size_t d_i=0; d_i < dimension_n; ++d_i) {
                    el_type sub = points[d_i][i] - centroids[d_i][j];
                    d += sub * sub;
                }
                if (d < min || min == -1) {
                    min = d;
                    centroid_i = j;
                }
            }
            // 3. assign each observation to a centroid based on their distance
            labels[i] = centroid_i;
            pointsPerCluster[centroid_i]++;

            // update new centroids partial sum
            for (size_t d_i=0; d_i<dimension_n; ++d_i)
                centroidsNew[d_i][centroid_i] += points[d_i][i];
        }

        // 4. compute new centroids (mean of observations in a cluster)
        for (size_t d_i=0; d_i<dimension_n; d_i++) {
            for (size_t i=0; i<cluster_n; i++) {
                centroidsNew[d_i][i] /= pointsPerCluster[i];
                // check whether the new centroids are the same as the old ones
                if (!centroidsChanged && !compare(centroidsNew[d_i][i], centroids[d_i][i], 3))
                    centroidsChanged = true;
            }
        }
        centroids = centroidsNew;
    }

    clusterCenters = centroids;
    this->labels = labels;
    this->nIter = nIter;

    return this;
}
