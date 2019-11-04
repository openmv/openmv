import vec.distance

def distance_to_kth_nearest_neighbor(points, i, k):
    fix = points[i]
    d = [vec.distance.euclidean(fix, p) for p in points]
    d.sort()
    return d[k]
    # Smallest distance is always to the point itself.
    # First element can be ignored.
    # Distance to k-th nearest neighbor is at index == k.

def calculate_threshold(r, d, beta):
    return r / (1 - beta)**(1 / d)

def find_threshold_position(threshold, ascending):
    lower = 0
    upper = len(ascending) - 1

    if ascending[upper][0] <= threshold:
        return upper

    # binary search
    while upper - lower > 1:
        i = (upper + lower) // 2
        r = ascending[i][0]

        if r == threshold:
            # push `i` to last element equal to threshold
            while i+1 < len(ascending) and ascending[i+1][0] == threshold:
                i += 1
            return i
        elif r < threshold:
            lower = i
        else:
            upper = i

    if ascending[upper][0] <= threshold:
        return upper
    else:
        return lower

def in_which(x, sets):
    for s in sets:
        if x in s:
            return s
    return None

def form_cluster_core(core_set,
                      existing_cores,
                      points,
                      sorted_radii,
                      seed_position,
                      threshold_position):

    def include(current_position):
        current_radius, current_index = sorted_radii[current_position]

        core_set.add(current_index)

        for other_position in range(0, threshold_position + 1):
            other_radius, other_index = sorted_radii[other_position]

            if other_index == current_index or other_index in core_set:
                continue

            distance = vec.distance.euclidean(
                        points[current_index], points[other_index])

            if distance <= min(current_radius, other_radius):
                # Connect current point to the other point

                if in_which(other_index, existing_cores) is not None:
                    # The other point is already in another core.
                    # Current set will not be disjoint from existing cores.
                    # Current set cannot form a new core.
                    return False
                else:
                    # Continue making connections, starting from new point.
                    clean = include(other_position)
                    if not clean:
                        return False
        return True

    return include(seed_position)

def assign_cluster(clusters,
                   points,
                   sorted_radii,
                   position):

    def find_cluster_to_belong(current_position):
        _, current_index = sorted_radii[current_position]

        cluster_set = in_which(current_index, clusters)
        if cluster_set is not None:
            # already belong to a cluster
            return cluster_set

        nearest_neighbor_distance = None
        nearest_neighbor_position = None

        # find nearest neighbor
        for i in range(0, current_position):
            _, other_index = sorted_radii[i]

            distance = vec.distance.euclidean(
                        points[current_index], points[other_index])

            if nearest_neighbor_distance is None \
                    or distance < nearest_neighbor_distance:
                nearest_neighbor_distance = distance
                nearest_neighbor_position = i

        # join nearest neighbor's cluster
        cluster_set = find_cluster_to_belong(nearest_neighbor_position)
        cluster_set.add(current_index)
        return cluster_set

    find_cluster_to_belong(position)

def cluster(points,
            k,
            beta,
            return_modes=False):
    dimension = len(points[0])

    sorted_radii = [(distance_to_kth_nearest_neighbor(points, i, k=k), i)
                        for i in range(0, len(points))]
    sorted_radii.sort()
    # smallest radius first, i.e. highest density first.

    modes = []
    clusters = []
    proposed_core = set()

    for position in range(0, len(sorted_radii)):
        radius, index = sorted_radii[position]

        threshold = calculate_threshold(radius, dimension, beta)
        threshold_position = find_threshold_position(threshold, sorted_radii)

        if form_cluster_core(proposed_core,
                             clusters,
                             points,
                             sorted_radii,
                             seed_position=position,
                             threshold_position=threshold_position):
            clusters.append(proposed_core)
            proposed_core = set()

            if return_modes:
                modes.append(index)
        else:
            proposed_core.clear()

    for position in range(0, len(sorted_radii)):
        assign_cluster(clusters,
                       points,
                       sorted_radii,
                       position=position)

    return (clusters, modes) if return_modes else clusters
