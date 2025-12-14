#include "graph.h"
#include <stdio.h>
#include <stdlib.h>

static int my_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}


void createGraph(Graph *g) {
    g->numVertices = 0;
    g->vertexCount = 0;
    for (int i = 0; i < MAX_GRAPH_VERTICES; i++) {
        g->vertices[i] = NULL;
        g->vertexIndexMap[i] = -1;
        for (int j = 0; j < MAX_GRAPH_VERTICES; j++) {
            g->adjMatrix[i][j] = false;
        }
    }
}

void addVertex(Graph *g, User *user) {
    if (user == NULL || g->vertexCount >= MAX_GRAPH_VERTICES) {
        return;
    }
    
    if (isVertexInGraph(g, getUserID(user))) {
        return;
    }
    
    
    g->vertices[g->vertexCount] = user;
    g->vertexIndexMap[g->vertexCount] = g->vertexCount;
    g->vertexCount++;
    g->numVertices = g->vertexCount;
}

void addEdge(Graph *g, char *follower_id, char *following_id) {
    int follower_idx = getVertexIndex(g, follower_id);
    int following_idx = getVertexIndex(g, following_id);
    
    if (follower_idx == -1 || following_idx == -1) {
        return;
    }
    
    g->adjMatrix[follower_idx][following_idx] = true;
}

void removeEdge(Graph *g, char *follower_id, char *following_id) {
    int follower_idx = getVertexIndex(g, follower_id);
    int following_idx = getVertexIndex(g, following_id);
    
    if (follower_idx == -1 || following_idx == -1) {
        return;
    }
    
    g->adjMatrix[follower_idx][following_idx] = false;
}


boolean hasEdge(Graph *g, char *follower_id, char *following_id) {
    int follower_idx = getVertexIndex(g, follower_id);
    int following_idx = getVertexIndex(g, following_id);
    
    if (follower_idx == -1 || following_idx == -1) {
        return false;
    }
    
    return g->adjMatrix[follower_idx][following_idx];
}

boolean isVertexInGraph(Graph *g, char *user_id) {
    return getVertexIndex(g, user_id) != -1;
}


int getVertexIndex(Graph *g, char *user_id) {
    for (int i = 0; i < g->vertexCount; i++) {
        if (g->vertices[i] != NULL && my_strcmp(getUserID(g->vertices[i]), user_id) == 0) {
            return i;
        }
    }
    return -1;
}

int getInDegree(Graph *g, char *user_id) {
    int user_idx = getVertexIndex(g, user_id);
    if (user_idx == -1) {
        return 0;
    }
    
    int count = 0;
    for (int i = 0; i < g->vertexCount; i++) {
        if (g->adjMatrix[i][user_idx]) {
            count++;
        }
    }
    return count;
}

int getOutDegree(Graph *g, char *user_id) {
    int user_idx = getVertexIndex(g, user_id);
    if (user_idx == -1) {
        return 0;
    }
    
    int count = 0;
    for (int i = 0; i < g->vertexCount; i++) {
        if (g->adjMatrix[user_idx][i]) {
            count++;
        }
    }
    return count;
}


void findMutualFriends(Graph *g, char *user1_id, char *user2_id, char **mutual, int *count) {
    *count = 0;
    int user1_idx = getVertexIndex(g, user1_id);
    int user2_idx = getVertexIndex(g, user2_id);
    
    if (user1_idx == -1 || user2_idx == -1) {
        return;
    }
    
    
    for (int i = 0; i < g->vertexCount; i++) {
        if (g->adjMatrix[user1_idx][i] && g->adjMatrix[user2_idx][i] && mutual != NULL) {
            if (*count < MAX_GRAPH_VERTICES) {
                mutual[*count] = getUserID(g->vertices[i]);
                (*count)++;
            }
        }
    }
}

void findSuggestedFriends(Graph *g, char *user_id, char **suggested, int *count, int maxSuggestions) {
    *count = 0;
    int user_idx = getVertexIndex(g, user_id);
    
    if (user_idx == -1) {
        return;
    }
    
    boolean alreadyFollowed[MAX_GRAPH_VERTICES];
    for (int i = 0; i < g->vertexCount; i++) {
        alreadyFollowed[i] = g->adjMatrix[user_idx][i] || (i == user_idx);
    }
    
    for (int i = 0; i < g->vertexCount && *count < maxSuggestions; i++) {
        if (alreadyFollowed[i]) {
            continue;
        }
        
        int mutualCount = 0;
        for (int j = 0; j < g->vertexCount; j++) {
            if (g->adjMatrix[user_idx][j] && g->adjMatrix[j][i]) {
                mutualCount++;
            }
        }
        
        if (mutualCount > 0 && suggested != NULL) {
            suggested[*count] = getUserID(g->vertices[i]);
            (*count)++;
        }
    }
}

boolean findPath(Graph *g, char *from_id, char *to_id, char **path, int *pathLength) {
    *pathLength = 0;
    int from_idx = getVertexIndex(g, from_id);
    int to_idx = getVertexIndex(g, to_id);
    
    if (from_idx == -1 || to_idx == -1) {
        return false;
    }
    
    if (from_idx == to_idx) {
        if (path != NULL && *pathLength < MAX_GRAPH_VERTICES) {
            path[*pathLength] = getUserID(g->vertices[from_idx]);
            (*pathLength)++;
        }
        return true;
    }
    
    boolean visited[MAX_GRAPH_VERTICES];
    int parent[MAX_GRAPH_VERTICES];
    int queue[MAX_GRAPH_VERTICES];
    int front = 0, rear = 0;
    
    for (int i = 0; i < g->vertexCount; i++) {
        visited[i] = false;
        parent[i] = -1;
    }
    
    visited[from_idx] = true;
    queue[rear++] = from_idx;
    
    while (front < rear) {
        int current = queue[front++];
        
        if (current == to_idx) {
            int path_stack[MAX_GRAPH_VERTICES];
            int path_top = 0;
            int node = to_idx;
            
            while (node != -1) {
                path_stack[path_top++] = node;
                node = parent[node];
            }
            
            *pathLength = path_top;
            if (path != NULL) {
                for (int i = path_top - 1; i >= 0; i--) {
                    path[path_top - 1 - i] = getUserID(g->vertices[path_stack[i]]);
                }
            }
            return true;
        }
        
        for (int i = 0; i < g->vertexCount; i++) {
            if (g->adjMatrix[current][i] && !visited[i]) {
                visited[i] = true;
                parent[i] = current;
                queue[rear++] = i;
            }
        }
    }
    
    return false;
}

void findMostPopularUsers(Graph *g, char **popular, int *count, int maxResults) {
    *count = 0;
    
    struct {
        int index;
        int inDegree;
    } users[MAX_GRAPH_VERTICES];
    
    int validCount = 0;
    for (int i = 0; i < g->vertexCount; i++) {
        if (g->vertices[i] != NULL) {
            users[validCount].index = i;
            users[validCount].inDegree = getInDegree(g, getUserID(g->vertices[i]));
            validCount++;
        }
    }
    
    for (int i = 0; i < validCount - 1; i++) {
        for (int j = 0; j < validCount - i - 1; j++) {
            if (users[j].inDegree < users[j + 1].inDegree) {
                int temp_index = users[j].index;
                int temp_inDegree = users[j].inDegree;
                users[j].index = users[j + 1].index;
                users[j].inDegree = users[j + 1].inDegree;
                users[j + 1].index = temp_index;
                users[j + 1].inDegree = temp_inDegree;
            }
        }
    }
    
    int resultCount = (validCount < maxResults) ? validCount : maxResults;
    *count = resultCount;
    if (popular != NULL) {
        for (int i = 0; i < resultCount; i++) {
            popular[i] = getUserID(g->vertices[users[i].index]);
        }
    }
}

void findMostActiveUsers(Graph *g, char **active, int *count, int maxResults) {
    *count = 0;
    
    struct {
        int index;
        int outDegree;
    } users[MAX_GRAPH_VERTICES];
    
    int validCount = 0;
    for (int i = 0; i < g->vertexCount; i++) {
        if (g->vertices[i] != NULL) {
            users[validCount].index = i;
            users[validCount].outDegree = getOutDegree(g, getUserID(g->vertices[i]));
            validCount++;
        }
    }
    
    for (int i = 0; i < validCount - 1; i++) {
        for (int j = 0; j < validCount - i - 1; j++) {
            if (users[j].outDegree < users[j + 1].outDegree) {
                int temp_index = users[j].index;
                int temp_outDegree = users[j].outDegree;
                users[j].index = users[j + 1].index;
                users[j].outDegree = users[j + 1].outDegree;
                users[j + 1].index = temp_index;
                users[j + 1].outDegree = temp_outDegree;
            }
        }
    }
    
    int resultCount = (validCount < maxResults) ? validCount : maxResults;
    *count = resultCount;
    if (active != NULL) {
        for (int i = 0; i < resultCount; i++) {
            active[i] = getUserID(g->vertices[users[i].index]);
        }
    }
}

static void dfsHelper(Graph *g, int current_idx, boolean *visited) {
    visited[current_idx] = true;
    
    for (int i = 0; i < g->vertexCount; i++) {
        if (g->adjMatrix[current_idx][i] && !visited[i]) {
            dfsHelper(g, i, visited);
        }
    }
}


void BFS(Graph *g, char *start_id, boolean *visited) {
    int start_idx = getVertexIndex(g, start_id);
    if (start_idx == -1) {
        return;
    }
    
    if (visited == NULL) {
        boolean local_visited[MAX_GRAPH_VERTICES];
        for (int i = 0; i < g->vertexCount; i++) {
            local_visited[i] = false;
        }
        visited = local_visited;
    }
    
    int queue[MAX_GRAPH_VERTICES];
    int front = 0, rear = 0;
    
    visited[start_idx] = true;
    queue[rear++] = start_idx;
    
    while (front < rear) {
        int current = queue[front++];
        
        for (int i = 0; i < g->vertexCount; i++) {
            if (g->adjMatrix[current][i] && !visited[i]) {
                visited[i] = true;
                queue[rear++] = i;
            }
        }
    }
}

void DFS(Graph *g, char *start_id, boolean *visited) {
    int start_idx = getVertexIndex(g, start_id);
    if (start_idx == -1) {
        return;
    }
    
    boolean local_visited[MAX_GRAPH_VERTICES];
    if (visited == NULL) {
        for (int i = 0; i < g->vertexCount; i++) {
            local_visited[i] = false;
        }
        visited = local_visited;
    }
    
    dfsHelper(g, start_idx, visited);
}

// fitur bonus: friend rec
// friend: A follow B, B follows A. (mutual)
boolean isFriend(Graph *g, char *user1_id, char *user2_id) {
    return hasEdge(g, user1_id, user2_id) && hasEdge(g, user2_id, user1_id);
}

int countMutual(Graph *g, char *user1_id, char *user2_id) {
    int user1_idx = getVertexIndex(g, user1_id);
    int user2_idx = getVertexIndex(g, user2_id);
    
    if (user1_idx == -1 || user2_idx == -1) {
        return 0;
    }
    
    int count = 0;
    for (int i = 0; i < g->vertexCount; i++) {
        if (i == user1_idx || i == user2_idx) continue;
        
        char *candidate_id = getUserID(g->vertices[i]);
        // Check if this user is friends with both user1 and user2
        if (isFriend(g, user1_id, candidate_id) && isFriend(g, user2_id, candidate_id)) {
            count++;
        }
    }
    return count;
}

void getRecommendations(Graph *g, char *user_id, FriendRecommendation *recommendations, int *count, int maxResults) {
    *count = 0;
    int user_idx = getVertexIndex(g, user_id);
    
    if (user_idx == -1 || recommendations == NULL) {
        return;
    }

    boolean visited[MAX_GRAPH_VERTICES];
    int depth[MAX_GRAPH_VERTICES];
    for (int i = 0; i < g->vertexCount; i++) {
        visited[i] = false;
        depth[i] = -1;
    }
    
    int queue[MAX_GRAPH_VERTICES];
    int front = 0, rear = 0;
    
    visited[user_idx] = true;
    depth[user_idx] = 0;
    queue[rear++] = user_idx;
    
    FriendRecommendation candidates[MAX_GRAPH_VERTICES];
    int candidateCount = 0;
    
    while (front < rear) {
        int current = queue[front++];
        int currentDepth = depth[current];
        
        if (currentDepth >= 3) {
            continue;
        }
        
        for (int i = 0; i < g->vertexCount; i++) {
            if (g->adjMatrix[current][i] && !visited[i]) {
                visited[i] = true;
                depth[i] = currentDepth + 1;
                queue[rear++] = i;
                
                if (depth[i] >= 2 && depth[i] <= 3) {
                    char *candidate_id = getUserID(g->vertices[i]);
                    
                    if (hasEdge(g, user_id, candidate_id)) {
                        continue;
                    }
                    
                    int mutualCount = countMutual(g, user_id, candidate_id);
                    
                    if (mutualCount > 0 && candidateCount < MAX_GRAPH_VERTICES) {
                        candidates[candidateCount].userIndex = i;
                        candidates[candidateCount].mutualCount = mutualCount;
                        candidateCount++;
                    }
                }
            }
        }
    }
    
    // aorting
    for (int i = 0; i < candidateCount - 1; i++) {
        for (int j = 0; j < candidateCount - i - 1; j++) {
            if (candidates[j].mutualCount < candidates[j + 1].mutualCount) {
                FriendRecommendation temp = candidates[j];
                candidates[j] = candidates[j + 1];
                candidates[j + 1] = temp;
            }
        }
    }
    
    int resultCount = (candidateCount < maxResults) ? candidateCount : maxResults;
    for (int i = 0; i < resultCount; i++) {
        recommendations[i] = candidates[i];
    }
    *count = resultCount;
}
